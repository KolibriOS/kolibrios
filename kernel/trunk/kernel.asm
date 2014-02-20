;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved.
;; PROGRAMMING:
;; Ivan Poddubny
;; Marat Zakiyanov (Mario79)
;; VaStaNi
;; Trans
;; Mihail Semenyako (mike.dld)
;; Sergey Kuzmin (Wildwest)
;; Andrey Halyavin (halyavin)
;; Mihail Lisovin (Mihasik)
;; Andrey Ignatiev (andrew_programmer)
;; NoName
;; Evgeny Grechnikov (Diamond)
;; Iliya Mihailov (Ghost)
;; Sergey Semyonov (Serge)
;; Johnny_B
;; SPraid (simba)
;; Hidnplayr
;; Alexey Teplov (<Lrz>)
;; Rus
;; Nable
;; shurf
;; Alver
;; Maxis
;; Galkov
;; CleverMouse
;; tsdima
;; turbanoff
;; Asper
;; art_zh
;;
;; Data in this file was originally part of MenuetOS project which is
;; distributed under the terms of GNU GPL. It is modified and redistributed as
;; part of KolibriOS project under the terms of GNU GPL.
;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa
;; PROGRAMMING:
;;
;; Ville Mikael Turjanmaa, villemt@itu.jyu.fi
;; - main os coding/design
;; Jan-Michael Brummer, BUZZ2@gmx.de
;; Felix Kaiser, info@felix-kaiser.de
;; Paolo Minazzi, paolo.minazzi@inwind.it
;; quickcode@mail.ru
;; Alexey, kgaz@crosswinds.net
;; Juan M. Caravaca, bitrider@wanadoo.es
;; kristol@nic.fi
;; Mike Hibbett, mikeh@oceanfree.net
;; Lasse Kuusijarvi, kuusijar@lut.fi
;; Jarek Pelczar, jarekp3@wp.pl
;;
;; KolibriOS is distributed in the hope that it will be useful, but WITHOUT ANY
;; WARRANTY. No author or distributor accepts responsibility to anyone for the
;; consequences of using it or for whether it serves any particular purpose or
;; works at all, unless he says so in writing. Refer to the GNU General Public
;; License (the "GPL") for full details.
;
;; Everyone is granted permission to copy, modify and redistribute KolibriOS,
;; but only under the conditions described in the GPL. A copy of this license
;; is supposed to have been given to you along with KolibriOS so you can know
;; your rights and responsibilities. It should be in a file named COPYING.
;; Among other things, the copyright notice and this notice must be preserved
;; on all copies.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary as "mnt"

include 'macros.inc'
include 'struct.inc'

$Revision$


USE_COM_IRQ     equ 1      ; make irq 3 and irq 4 available for PCI devices
VESA_1_2_VIDEO  equ 0      ; enable vesa 1.2 bank switch functions

; Enabling the next line will enable serial output console
;debug_com_base  equ 0x3f8  ; 0x3f8 is com1, 0x2f8 is com2, 0x3e8 is com3, 0x2e8 is com4, no irq's are used

include "proc32.inc"
include "kglobals.inc"
include "lang.inc"
include "encoding.inc"

include "const.inc"

iglobal
; The following variable, if equal to 1, duplicates debug output to the screen.
debug_direct_print db 0
; Start the first app (LAUNCHER) after kernel is loaded? (1=yes, 2 or 0=no)
launcher_start db 1
endg

max_processes    equ   255
tss_step         equ   (128+8192) ; tss & i/o - 65535 ports, * 256=557056*4

os_stack       equ  (os_data_l-gdts)    ; GDTs
os_code        equ  (os_code_l-gdts)
graph_data     equ  (3+graph_data_l-gdts)
tss0           equ  (tss0_l-gdts)
app_code       equ  (3+app_code_l-gdts)
app_data       equ  (3+app_data_l-gdts)
app_tls        equ  (3+tls_data_l-gdts)
pci_code_sel   equ  (pci_code_32-gdts)
pci_data_sel   equ  (pci_data_32-gdts)


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;   Included files:
;;
;;   Kernel16.inc
;;    - Booteng.inc   English text for bootup
;;    - Bootcode.inc  Hardware setup
;;    - Pci16.inc     PCI functions
;;
;;   Kernel32.inc
;;    - Sys32.inc     Process management
;;    - Shutdown.inc  Shutdown and restart
;;    - Fat32.inc     Read / write hd
;;    - Vesa12.inc    Vesa 1.2 driver
;;    - Vesa20.inc    Vesa 2.0 driver
;;    - Vga.inc       VGA driver
;;    - Stack.inc     Network interface
;;    - Mouse.inc     Mouse pointer
;;    - Scincode.inc  Window skinning
;;    - Pci32.inc     PCI functions
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                      ;;
;;                  16 BIT ENTRY FROM BOOTSECTOR                        ;;
;;                                                                      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

use16
                  org   0x0
        jmp     start_of_code

if lang eq sp
include "kernelsp.inc"  ; spanish kernel messages
else if lang eq et
version db    'Kolibri OS  versioon 0.7.7.0+    ',13,10,13,10,0
else
version db    'Kolibri OS  version 0.7.7.0+     ',13,10,13,10,0
end if

include "boot/bootstr.inc"     ; language-independent boot messages
include "boot/preboot.inc"

if lang eq ge
include "boot/bootge.inc"     ; german system boot messages
else if lang eq sp
include "boot/bootsp.inc"     ; spanish system boot messages
else if lang eq ru
include "boot/bootru.inc"      ; russian system boot messages
include "boot/ru.inc"          ; Russian font
else if lang eq et
include "boot/bootet.inc"      ; estonian system boot messages
include "boot/et.inc"          ; Estonian font
else
include "boot/booten.inc"      ; english system boot messages
end if

include "boot/bootcode.inc"    ; 16 bit system boot code
include "bus/pci/pci16.inc"
include "detect/biosdisk.inc"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                      ;;
;;                  SWITCH TO 32 BIT PROTECTED MODE                     ;;
;;                                                                      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; CR0 Flags - Protected mode and Paging

        mov     ecx, CR0_PE

; Enabling 32 bit protected mode

        sidt    [cs:old_ints_h]

        cli                             ; disable all irqs
        cld
        mov     al, 255                 ; mask all irqs
        out     0xa1, al
        out     0x21, al
   l.5:
        in      al, 0x64                ; Enable A20
        test    al, 2
        jnz     l.5
        mov     al, 0xD1
        out     0x64, al
   l.6:
        in      al, 0x64
        test    al, 2
        jnz     l.6
        mov     al, 0xDF
        out     0x60, al
   l.7:
        in      al, 0x64
        test    al, 2
        jnz     l.7
        mov     al, 0xFF
        out     0x64, al

        lgdt    [cs:tmp_gdt]            ; Load GDT
        mov     eax, cr0                ; protected mode
        or      eax, ecx
        and     eax, 10011111b *65536*256 + 0xffffff ; caching enabled
        mov     cr0, eax
        jmp     pword os_code:B32       ; jmp to enable 32 bit mode

align 8
tmp_gdt:

        dw     23
        dd     tmp_gdt+0x10000
        dw     0

        dw     0xffff
        dw     0x0000
        db     0x00
        dw     11011111b *256 +10011010b
        db     0x00

        dw     0xffff
        dw     0x0000
        db     0x00
        dw     11011111b *256 +10010010b
        db     0x00

include "data16.inc"

if ~ lang eq sp
diff16 "end of bootcode",0,$+0x10000
end if

use32
org $+0x10000

align 4
B32:
        mov     ax, os_stack       ; Selector for os
        mov     ds, ax
        mov     es, ax
        mov     fs, ax
        mov     gs, ax
        mov     ss, ax
        mov     esp, 0x006CC00       ; Set stack

; CLEAR 0x280000 - HEAP_BASE

        xor     eax, eax
        mov     edi, CLEAN_ZONE
        mov     ecx, (HEAP_BASE-OS_BASE-CLEAN_ZONE) / 4
        cld
        rep stosd

; CLEAR KERNEL UNDEFINED GLOBALS
        mov     edi, endofcode-OS_BASE
        mov     ecx, 0x90000
        sub     ecx, edi
        shr     ecx, 2
        rep stosd

; SAVE & CLEAR 0-0xffff

        mov     edi, 0x1000
        mov     ecx, 0x8000 / 4
        rep stosd
        mov     edi, 0xa000
        mov     ecx, 0x6000 / 4
        rep stosd

        call    test_cpu
        bts     [cpu_caps-OS_BASE], CAPS_TSC    ;force use rdtsc

        call    check_acpi
        call    init_BIOS32
; MEMORY MODEL
        call    mem_test
        call    init_mem
        call    init_page_map

; ENABLE PAGING

        mov     eax, sys_pgdir-OS_BASE
        mov     cr3, eax

        mov     eax, cr0
        or      eax, CR0_PG+CR0_WP
        mov     cr0, eax

        lgdt    [gdts]
        jmp     pword os_code:high_code

align 4
bios32_entry    dd ?
tmp_page_tabs   dd ?

use16
org $-0x10000
include "boot/shutdown.inc" ; shutdown or restart
org $+0x10000
use32

__DEBUG__ fix 1
__DEBUG_LEVEL__ fix 1
include 'init.inc'

org OS_BASE+$

include 'fdo.inc'

align 4
high_code:
        mov     ax, os_stack
        mov     bx, app_data
        mov     cx, app_tls
        mov     ss, ax
        add     esp, OS_BASE

        mov     ds, bx
        mov     es, bx
        mov     fs, cx
        mov     gs, bx

        bt      [cpu_caps], CAPS_PGE
        jnc     @F

        or      dword [sys_pgdir+(OS_BASE shr 20)], PG_GLOBAL

        mov     ebx, cr4
        or      ebx, CR4_PGE
        mov     cr4, ebx
@@:
        xor     eax, eax
        mov     dword [sys_pgdir], eax
        mov     dword [sys_pgdir+4], eax

        mov     eax, cr3
        mov     cr3, eax          ; flush TLB

        mov     ecx, pg_data.mutex
        call    mutex_init

        mov     ecx, disk_list_mutex
        call    mutex_init

        mov     ecx, keyboard_list_mutex
        call    mutex_init

        mov     ecx, unpack_mutex
        call    mutex_init

        mov     ecx, application_table_mutex
        call    mutex_init

        mov     ecx, ide_mutex
        call    mutex_init
        mov     ecx, ide_channel1_mutex
        call    mutex_init
        mov     ecx, ide_channel2_mutex
        call    mutex_init
;-----------------------------------------------------------------------------
; SAVE REAL MODE VARIABLES
;-----------------------------------------------------------------------------
save_variables_IDE_controller:
        xor     eax, eax
        mov     ax, [BOOT_VARS + BOOT_IDE_INTERR_16]
        mov     [IDE_Interrupt], ax
;--------------------------------------
        mov     ax, [BOOT_VARS + BOOT_IDE_PI_16]
        mov     [IDEContrProgrammingInterface], ax
;--------------------------------------
        mov     ax, [BOOT_VARS + BOOT_IDE_BASE_ADDR]
        mov     [IDEContrRegsBaseAddr], ax
;--------------------------------------
        mov     ax, [BOOT_VARS + BOOT_IDE_BAR0_16]
        cmp     ax, 0
        je      @f
        cmp     ax, 1
        jne     .no_PATA_BAR0
@@:
        mov     ax, 0x1F0
        jmp     @f
.no_PATA_BAR0:
        and     ax, 0xFFFC
@@:
        mov     [StandardATABases], ax
        mov     [hd_address_table], eax
        mov     [hd_address_table+8], eax
        mov     [IDE_BAR0_val], ax
;--------------------------------------
        mov     ax, [BOOT_VARS + BOOT_IDE_BAR1_16]
        cmp     ax, 0
        je      @f
        cmp     ax, 1
        jne     .no_PATA_BAR1
@@:
        mov     ax, 0x3F4
        jmp     @f
.no_PATA_BAR1:
        and     ax, 0xFFFC
@@:
        mov     [IDE_BAR1_val], ax
;--------------------------------------
        mov     ax, [BOOT_VARS + BOOT_IDE_BAR2_16]
        cmp     ax, 0
        je      @f
        cmp     ax, 1
        jne     .no_PATA_BAR2
@@:
        mov     ax, 0x170
        jmp     @f
.no_PATA_BAR2:
        and     ax, 0xFFFC
@@:
        mov     [StandardATABases+2], ax
        mov     [hd_address_table+16], eax
        mov     [hd_address_table+24], eax
        mov     [IDE_BAR2_val], ax
;--------------------------------------
        mov     ax, [BOOT_VARS + BOOT_IDE_BAR3_16]
        cmp     ax, 0
        je      @f
        cmp     ax, 1
        jne     .no_PATA_BAR3
@@:
        mov     ax, 0x374
        jmp     @f
.no_PATA_BAR3:
        and     ax, 0xFFFC
@@:
        mov     [IDE_BAR3_val], ax

; --------------- APM ---------------------

; init selectors
        mov     ebx, [BOOT_VARS+BOOT_APM_ENTRY]        ; offset of APM entry point
        movzx   eax, word [BOOT_VARS+BOOT_APM_CODE_32] ; real-mode segment base address of
                                                                                ; protected-mode 32-bit code segment
        movzx   ecx, word [BOOT_VARS+BOOT_APM_CODE_16]; real-mode segment base address of
                                                                                ; protected-mode 16-bit code segment
        movzx   edx, word [BOOT_VARS+BOOT_APM_DATA_16]; real-mode segment base address of
                                                                                ; protected-mode 16-bit data segment

        shl     eax, 4
        mov     [dword apm_code_32 + 2], ax
        shr     eax, 16
        mov     [dword apm_code_32 + 4], al

        shl     ecx, 4
        mov     [dword apm_code_16 + 2], cx
        shr     ecx, 16
        mov     [dword apm_code_16 + 4], cl

        shl     edx, 4
        mov     [dword apm_data_16 + 2], dx
        shr     edx, 16
        mov     [dword apm_data_16 + 4], dl

        mov     dword[apm_entry], ebx
        mov     word [apm_entry + 4], apm_code_32 - gdts

        mov     eax, [BOOT_VARS + BOOT_APM_VERSION] ; version & flags
        mov     [apm_vf], eax
; -----------------------------------------
        mov     al, [BOOT_VARS+BOOT_DMA]            ; DMA access
        mov     [allow_dma_access], al
        movzx   eax, byte [BOOT_VARS+BOOT_BPP]      ; bpp
        mov     [_display.bpp], eax
        mov     [_display.vrefresh], 60
        mov     al, [BOOT_VARS+BOOT_DEBUG_PRINT]    ; If nonzero, duplicates debug output to the screen
        mov     [debug_direct_print], al
        mov     al, [BOOT_VARS+BOOT_LAUNCHER_START] ; Start the first app (LAUNCHER) after kernel is loaded?
        mov     [launcher_start], al
        movzx   eax, word [BOOT_VARS+BOOT_X_RES]; X max
        mov     [_display.width], eax
        mov     [display_width_standard], eax
        dec     eax
        mov     [Screen_Max_X], eax
        mov     [screen_workarea.right], eax
        movzx   eax, word [BOOT_VARS+BOOT_Y_RES]; Y max
        mov     [_display.height], eax
        mov     [display_height_standard], eax
        dec     eax
        mov     [Screen_Max_Y], eax
        mov     [screen_workarea.bottom], eax
        movzx   eax, word [BOOT_VARS+BOOT_VESA_MODE] ; screen mode
        mov     dword [SCR_MODE], eax
;        mov     eax, [BOOT_VAR+0x9014]             ; Vesa 1.2 bnk sw add
;        mov     [BANK_SWITCH], eax
        mov     eax, 640 *4                         ; Bytes PerScanLine
        cmp     [SCR_MODE], word 0x13               ; 320x200
        je      @f
        cmp     [SCR_MODE], word 0x12               ; VGA 640x480
        je      @f
        movzx   eax, word[BOOT_VARS+BOOT_PITCH]      ; for other modes
@@:
        mov     [_display.pitch], eax
        mov     eax, [_display.width]
        mul     [_display.height]
        mov     [_WinMapSize], eax

        call    calculate_fast_getting_offset_for_WinMapAddress
; for Qemu or non standart video cards
; Unfortunately [BytesPerScanLine] does not always
;                             equal to [_display.width] * [ScreenBPP] / 8
        call    calculate_fast_getting_offset_for_LFB

        mov     esi, BOOT_VARS+0x9080
        movzx   ecx, byte [esi-1]
        mov     [NumBiosDisks], ecx
        mov     edi, BiosDisksData
        rep movsd

; GRAPHICS ADDRESSES

        mov     eax, [BOOT_VARS+BOOT_LFB]
        mov     [LFBAddress], eax

        cmp     [SCR_MODE], word 0100000000000000b
        jge     setvesa20
        cmp     [SCR_MODE], word 0x13  ; EGA 320*200 256 colors
        je      v20ga32
        jmp     v20ga24

setvesa20:
        mov     [PUTPIXEL], dword Vesa20_putpixel24 ; Vesa 2.0
        mov     [GETPIXEL], dword Vesa20_getpixel24
        cmp     byte [_display.bpp], 24
        jz      v20ga24
v20ga32:
        mov     [PUTPIXEL], dword Vesa20_putpixel32
        mov     [GETPIXEL], dword Vesa20_getpixel32
        jmp     no_mode_0x12
v20ga24:
        cmp     [SCR_MODE], word 0x12               ; 16 C VGA 640x480
        jne     no_mode_0x12
        mov     [PUTPIXEL], dword VGA_putpixel
        mov     [GETPIXEL], dword Vesa20_getpixel32
no_mode_0x12:

        mov     [MOUSE_PICTURE], dword mousepointer
        mov     [_display.check_mouse], check_mouse_area_for_putpixel
        mov     [_display.check_m_pixel], check_mouse_area_for_getpixel

; -------- Fast System Call init ----------
; Intel SYSENTER/SYSEXIT (AMD CPU support it too)
        bt      [cpu_caps], CAPS_SEP
        jnc     .SEnP  ; SysEnter not Present
        xor     edx, edx
        mov     ecx, MSR_SYSENTER_CS
        mov     eax, os_code
        wrmsr
        mov     ecx, MSR_SYSENTER_ESP
;           mov eax, sysenter_stack ; Check it
        xor     eax, eax
        wrmsr
        mov     ecx, MSR_SYSENTER_EIP
        mov     eax, sysenter_entry
        wrmsr
.SEnP:
; AMD SYSCALL/SYSRET
        cmp     byte[cpu_vendor], 'A'
        jne     .noSYSCALL
        mov     eax, 0x80000001
        cpuid
        test    edx, 0x800  ; bit_11 - SYSCALL/SYSRET support
        jz      .noSYSCALL
        mov     ecx, MSR_AMD_EFER
        rdmsr
        or      eax, 1 ; bit_0 - System Call Extension (SCE)
        wrmsr

        ; !!!! It`s dirty hack, fix it !!!
        ; Bits of EDX :
        ; Bit 31–16 During the SYSRET instruction, this field is copied into the CS register
        ;  and the contents of this field, plus 8, are copied into the SS register.
        ; Bit 15–0 During the SYSCALL instruction, this field is copied into the CS register
        ;  and the contents of this field, plus 8, are copied into the SS register.

        ; mov   edx, (os_code + 16) * 65536 + os_code
        mov     edx, 0x1B0008

        mov     eax, syscall_entry
        mov     ecx, MSR_AMD_STAR
        wrmsr
.noSYSCALL:
; -----------------------------------------
        stdcall alloc_page
        stdcall map_page, tss-0xF80, eax, PG_SW
        stdcall alloc_page
        stdcall map_page, tss+0x80, eax, PG_SW
        stdcall alloc_page
        stdcall map_page, tss+0x1080, eax, PG_SW

; LOAD IDT

        call    build_interrupt_table ;lidt is executed
          ;lidt [idtreg]

        call    init_kernel_heap
        stdcall kernel_alloc, (RING0_STACK_SIZE+512) * 2
        mov     [os_stack_seg], eax

        lea     esp, [eax+RING0_STACK_SIZE]

        mov     [tss._ss0], os_stack
        mov     [tss._esp0], esp
        mov     [tss._esp], esp
        mov     [tss._cs], os_code
        mov     [tss._ss], os_stack
        mov     [tss._ds], app_data
        mov     [tss._es], app_data
        mov     [tss._fs], app_data
        mov     [tss._gs], app_data
        mov     [tss._io], 128
;Add IO access table - bit array of permitted ports
        mov     edi, tss._io_map_0
        xor     eax, eax
        not     eax
        mov     ecx, 8192/4
        rep stosd                    ; access to 4096*8=65536 ports

        mov     ax, tss0
        ltr     ax

        mov     [LFBSize], 0xC00000
        call    init_LFB
        call    init_fpu
        call    init_malloc

        stdcall alloc_kernel_space, 0x51000
        mov     [default_io_map], eax

        add     eax, 0x2000
        mov     [ipc_tmp], eax
        mov     ebx, 0x1000

        add     eax, 0x40000
        mov     [proc_mem_map], eax

        add     eax, 0x8000
        mov     [proc_mem_pdir], eax

        add     eax, ebx
        mov     [proc_mem_tab], eax

        add     eax, ebx
        mov     [tmp_task_pdir], eax

        add     eax, ebx
        mov     [tmp_task_ptab], eax

        add     eax, ebx
        mov     [ipc_pdir], eax

        add     eax, ebx
        mov     [ipc_ptab], eax

        stdcall kernel_alloc, (unpack.LZMA_BASE_SIZE+(unpack.LZMA_LIT_SIZE shl \
                (unpack.lc+unpack.lp)))*4

        mov     [unpack.p], eax

        call    init_events
        mov     eax, srv.fd-SRV.fd
        mov     [srv.fd], eax
        mov     [srv.bk], eax

;Set base of graphic segment to linear address of LFB
        mov     eax, [LFBAddress]         ; set for gs
        mov     [graph_data_l+2], ax
        shr     eax, 16
        mov     [graph_data_l+4], al
        mov     [graph_data_l+7], ah

        stdcall kernel_alloc, [_WinMapSize]
        mov     [_WinMapAddress], eax

        xor     eax, eax
        inc     eax

; set background

        mov     [BgrDrawMode], eax
        mov     [BgrDataWidth], eax
        mov     [BgrDataHeight], eax
        mov     [mem_BACKGROUND], 4
        mov     [img_background], static_background_data

; set clipboard

        xor     eax, eax
        mov     [clipboard_slots], eax
        mov     [clipboard_write_lock], eax
        stdcall kernel_alloc, 4096
        test    eax, eax
        jnz     @f

        dec     eax
@@:
        mov     [clipboard_main_list], eax

; SET UP OS TASK

        mov     esi, boot_setostask
        call    boot_log

        mov     edx, SLOT_BASE+256
        mov     ebx, [os_stack_seg]
        add     ebx, 0x2000
        call    setup_os_slot
        mov     dword [edx], 'IDLE'
        sub     [edx+APPDATA.saved_esp], 4
        mov     eax, [edx+APPDATA.saved_esp]
        mov     dword [eax], idle_thread
        mov     ecx, IDLE_PRIORITY
        call    scheduler_add_thread

        mov     edx, SLOT_BASE+256*2
        mov     ebx, [os_stack_seg]
        call    setup_os_slot
        mov     dword [edx], 'OS'
        xor     ecx, ecx
        call    scheduler_add_thread

        mov     dword [CURRENT_TASK], 2
        mov     dword [TASK_COUNT], 2
        mov     dword [current_slot], SLOT_BASE + 256*2
        mov     dword [TASK_BASE], CURRENT_TASK + 32*2


; REDIRECT ALL IRQ'S TO INT'S 0x20-0x2f
        mov     esi, boot_initirq
        call    boot_log
        call    init_irqs

        mov     esi, boot_picinit
        call    boot_log
        call    PIC_init

        mov     esi, boot_v86machine
        call    boot_log
; Initialize system V86 machine
        call    init_sys_v86

        mov     esi, boot_inittimer
        call    boot_log
; Initialize system timer (IRQ0)
        call    PIT_init

; Register ramdisk file system
        mov     esi, boot_initramdisk
        call    boot_log
        call    ramdisk_init

        mov     esi, boot_initapic
        call    boot_log
; Try to Initialize APIC
        call    APIC_init

        mov     esi, boot_enableirq
        call    boot_log
; Enable timer IRQ (IRQ0) and co-processor IRQ (IRQ13)
; they are used: when partitions are scanned, hd_read relies on timer
        call    unmask_timer
        stdcall enable_irq, 2               ; @#$%! PIC
        stdcall enable_irq, 13              ; co-processor
;-----------------------------------------------------------------------------
; show SVN version of kernel on the message board
;-----------------------------------------------------------------------------
        mov     eax, [version_inf.rev]
        DEBUGF  1, "K : kernel SVN r%d\n", eax
;-----------------------------------------------------------------------------
; show CPU count on the message board
;-----------------------------------------------------------------------------
        mov     eax, [cpu_count]
        test    eax, eax
        jnz     @F
        mov     al, 1                             ; at least one CPU
@@:
        DEBUGF  1, "K : %d CPU detected\n", eax
;-----------------------------------------------------------------------------
; detect Floppy drives
;-----------------------------------------------------------------------------
        mov     esi, boot_detectfloppy
        call    boot_log
include 'detect/dev_fd.inc'
;-----------------------------------------------------------------------------
; START of initialisation IDE ATA code
;-----------------------------------------------------------------------------
        cmp     [IDEContrProgrammingInterface], 0
        je      @f

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
        DEBUGF  1, "K : BAR0 %x \n", [IDE_BAR0_val]:4
        DEBUGF  1, "K : BAR1 %x \n", [IDE_BAR1_val]:4
        DEBUGF  1, "K : BAR2 %x \n", [IDE_BAR2_val]:4
        DEBUGF  1, "K : BAR3 %x \n", [IDE_BAR3_val]:4
        DEBUGF  1, "K : BAR4 %x \n", [IDEContrRegsBaseAddr]:4
        DEBUGF  1, "K : IDEContrProgrammingInterface %x \n", [IDEContrProgrammingInterface]:4
        DEBUGF  1, "K : IDE_Interrupt %x \n", [IDE_Interrupt]:4
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

        cmp     [IDEContrProgrammingInterface], 0
        je      set_interrupts_for_IDE_controllers.continue
;-----------------------------------------------------------------------------
; set interrupts for IDE Controller
;-----------------------------------------------------------------------------
        mov     esi, boot_set_int_IDE
        call    boot_log
set_interrupts_for_IDE_controllers:
        mov     ax, [IDEContrProgrammingInterface]
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
        mov     esi, boot_init_sys
        call    boot_log
        call    Parser_params

if ~ defined extended_primary_loader
; ramdisk image should be loaded by extended primary loader if it exists
; READ RAMDISK IMAGE FROM HD

;!!!!!!!!!!!!!!!!!!!!!!!
include 'boot/rdload.inc'
;!!!!!!!!!!!!!!!!!!!!!!!
end if

if 0
        mov     ax, [OS_BASE+0x10000+bx_from_load]
        cmp     ax, 'r1'; if using not ram disk, then load librares and parameters {SPraid.simba}
        je      no_lib_load

        mov     esi, boot_loadlibs
        call    boot_log
; LOADING LIBRARES
        stdcall dll.Load, @IMPORT           ; loading librares for kernel (.obj files)
        call    load_file_parse_table       ; prepare file parse table
        call    set_kernel_conf             ; configure devices and gui
no_lib_load:
end if

; Display APIC status
        mov     esi, boot_APIC_found
        cmp     [irq_mode], IRQ_APIC
        je      @f
        mov     esi, boot_APIC_nfound
@@:
        call    boot_log

; PRINT AMOUNT OF MEMORY
        mov     esi, boot_memdetect
        call    boot_log

        movzx   ecx, word [boot_y]
        if lang eq ru
        or      ecx, (10+30*6) shl 16
        else if lang eq sp
        or      ecx, (10+33*6) shl 16
        else
        or      ecx, (10+29*6) shl 16
        end if
        sub     ecx, 10
        mov     edx, 0xFFFFFF
        mov     ebx, [MEM_AMOUNT]
        shr     ebx, 20
        xor     edi, edi
        mov     eax, 0x00040000
        inc     edi
        call    display_number_force

; BUILD SCHEDULER

;        call    build_scheduler; sys32.inc

;        mov     esi, boot_devices
;        call    boot_log

        mov     [pci_access_enabled], 1
        call    pci_enum
        call    clear_pci_ide_interrupts

include "detect/vortex86.inc"                     ; Vortex86 SoC detection code

        stdcall load_driver, szVidintel

        call    usb_init

; SET PRELIMINARY WINDOW STACK AND POSITIONS

        mov     esi, boot_windefs
        call    boot_log
        call    set_window_defaults

; SET BACKGROUND DEFAULTS

        mov     esi, boot_bgr
        call    boot_log
        call    init_background
        call    calculatebackground

; RESERVE SYSTEM IRQ'S JA PORT'S

        mov     esi, boot_resirqports
        call    boot_log
        call    reserve_irqs_ports

        call    init_display
        mov     eax, [def_cursor]
        mov     [SLOT_BASE+APPDATA.cursor+256], eax
        mov     [SLOT_BASE+APPDATA.cursor+256*2], eax

; PRINT CPU FREQUENCY

        mov     esi, boot_cpufreq
        call    boot_log

        cli                         ;FIXME check IF
        rdtsc
        mov     ecx, eax
        mov     esi, 250            ; wait 1/4 a second
        call    delay_ms
        rdtsc

        sub     eax, ecx
        xor     edx, edx
        shld    edx, eax, 2
        shl     eax, 2
        mov     dword [cpu_freq], eax
        mov     dword [cpu_freq+4], edx
        mov     ebx, 1000000
        div     ebx
        mov     ebx, eax

        movzx   ecx, word [boot_y]
        if lang eq ru
        add     ecx, (10+19*6) shl 16 - 10
        else if lang eq sp
        add     ecx, (10+25*6) shl 16 - 10
        else
        add     ecx, (10+17*6) shl 16 - 10
        end if

        mov     edx, 0xFFFFFF
        xor     edi, edi
        mov     eax, 0x00040000
        inc     edi
        call    display_number_force

; SET VARIABLES

        call    set_variables

; STACK AND FDC

        call    stack_init
        call    fdc_init

; PALETTE FOR 320x200 and 640x480 16 col

        cmp     [SCR_MODE], word 0x12
        jne     no_pal_vga
        mov     esi, boot_pal_vga
        call    boot_log
        call    paletteVGA
      no_pal_vga:

        cmp     [SCR_MODE], word 0x13
        jne     no_pal_ega
        mov     esi, boot_pal_ega
        call    boot_log
        call    palette320x200
      no_pal_ega:

; LOAD DEFAULT SKIN

        call    load_default_skin

;protect io permission map

        mov     esi, [default_io_map]
        stdcall map_page, esi, [SLOT_BASE+256+APPDATA.io_map], PG_MAP
        add     esi, 0x1000
        stdcall map_page, esi, [SLOT_BASE+256+APPDATA.io_map+4], PG_MAP

        stdcall map_page, tss._io_map_0, \
                [SLOT_BASE+256+APPDATA.io_map], PG_MAP
        stdcall map_page, tss._io_map_1, \
                [SLOT_BASE+256+APPDATA.io_map+4], PG_MAP

; LOAD FIRST APPLICATION
        cmp     byte [launcher_start], 1        ; Check if starting LAUNCHER is selected on blue screen (1 = yes)
        jnz     first_app_found

        cli
        mov     ebp, firstapp
        call    fs_execute_from_sysdir
        test    eax, eax
        jns     first_app_found

        mov     esi, boot_failed
        call    boot_log

        mov     eax, 0xDEADBEEF      ; otherwise halt
        hlt

first_app_found:

        cli

; SET KEYBOARD PARAMETERS
        mov     al, 0xf6       ; reset keyboard, scan enabled
        call    kb_write
        test    ah, ah
        jnz     .no_keyboard

        ; wait until 8042 is ready
        xor     ecx, ecx
      @@:
        in      al, 64h
        and     al, 00000010b
        loopnz  @b

iglobal
align 4
ps2_keyboard_functions:
        dd      .end - $
        dd      0       ; no close
        dd      ps2_set_lights
.end:
endg
        stdcall register_keyboard, ps2_keyboard_functions, 0
       ; mov   al, 0xED       ; Keyboard LEDs - only for testing!
       ; call  kb_write
       ; call  kb_read
       ; mov   al, 111b
       ; call  kb_write
       ; call  kb_read

        mov     al, 0xF3     ; set repeat rate & delay
        call    kb_write
;        call  kb_read
        mov     al, 0; 30 250 ;00100010b ; 24 500  ;00100100b  ; 20 500
        call    kb_write
;        call  kb_read
     ;// mike.dld [
        call    set_lights
     ;// mike.dld ]
        stdcall attach_int_handler, 1, irq1, 0
        DEBUGF  1, "K : IRQ1 error code %x\n", eax
.no_keyboard:

; Load PS/2 mouse driver

        stdcall load_driver, szPS2MDriver

        mov     esi, boot_setmouse
        call    boot_log
        call    setmouse

; Setup serial output console (if enabled)
if defined debug_com_base

        ; reserve port so nobody else will use it
        xor     ebx, ebx
        mov     ecx, debug_com_base
        mov     edx, debug_com_base+7
        call    r_f_port_area

        ; enable Divisor latch
        mov     dx, debug_com_base+3
        mov     al, 1 shl 7
        out     dx, al

        ; Set speed to 115200 baud (max speed)
        mov     dx, debug_com_base
        mov     al, 0x01
        out     dx, al

        mov     dx, debug_com_base+1
        mov     al, 0x00
        out     dx, al

        ; No parity, 8bits words, one stop bit, dlab bit back to 0
        mov     dx, debug_com_base+3
        mov     al, 3
        out     dx, al

        ; disable interrupts
        mov     dx, debug_com_base+1
        mov     al, 0
        out     dx, al

        ; clear +  enable fifo (64 bits)
        mov     dx, debug_com_base+2
        mov     al, 0x7 + 1 shl 5
        out     dx, al

end if
; START MULTITASKING

; A 'All set - press ESC to start' messages if need
if preboot_blogesc
        mov     esi, boot_tasking
        call    boot_log
.bll1:
        in      al, 0x60        ; wait for ESC key press
        cmp     al, 129
        jne     .bll1
end if

        mov     [timer_ticks_enable], 1         ; for cd driver

        sti
;        call    change_task

        jmp     osloop


        ; Fly :)

include 'unpacker.inc'

align 4
boot_log:
        pushad

        mov     ebx, 10*65536
        mov     bx, word [boot_y]
        add     [boot_y], dword 10
        mov     ecx, 0x80ffffff; ASCIIZ string with white color
        xor     edi, edi
        mov     edx, esi
        inc     edi
        call    dtext

        mov     [novesachecksum], 1000
        call    checkVga_N13

        popad

        ret

; in: edx -> APPDATA for OS/IDLE slot
; in: ebx = stack base
proc setup_os_slot
        xor     eax, eax
        mov     ecx, 256/4
        mov     edi, edx
        rep stosd

        mov     eax, tss+0x80
        call    get_pg_addr
        inc     eax
        mov     [edx+APPDATA.io_map], eax
        mov     eax, tss+0x1080
        call    get_pg_addr
        inc     eax
        mov     [edx+APPDATA.io_map+4], eax

        mov     dword [edx+APPDATA.pl0_stack], ebx
        lea     edi, [ebx+0x2000-512]
        mov     dword [edx+APPDATA.fpu_state], edi
        mov     dword [edx+APPDATA.saved_esp0], edi
        mov     dword [edx+APPDATA.saved_esp], edi
        mov     dword [edx+APPDATA.terminate_protection], 1 ; make unkillable

        mov     esi, fpu_data
        mov     ecx, 512/4
        cld
        rep movsd

        lea     eax, [edx+APP_OBJ_OFFSET]
        mov     dword [edx+APPDATA.fd_obj], eax
        mov     dword [edx+APPDATA.bk_obj], eax

        mov     dword [edx+APPDATA.cur_dir], sysdir_path

        mov     [edx + APPDATA.dir_table], sys_pgdir - OS_BASE

        mov     eax, edx
        shr     eax, 3
        add     eax, CURRENT_TASK - (SLOT_BASE shr 3)
        mov     [eax+TASKDATA.wnd_number], dh
        mov     byte [eax+TASKDATA.pid], dh

        ret
endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                                    ;
;                    MAIN OS LOOP START                              ;
;                                                                    ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 32
osloop:
        mov     edx, osloop_has_work?
        xor     ecx, ecx
        call    Wait_events
        xor     eax, eax
        xchg    eax, [osloop_nonperiodic_work]
        test    eax, eax
        jz      .no_periodic
;        call    [draw_pointer]
        call    __sys_draw_pointer
        call    window_check_events
        call    mouse_check_events
        call    checkmisc
        call    checkVga_N13
.no_periodic:
        call    stack_handler
        call    check_fdd_motor_status
        call    check_ATAPI_device_event
        call    check_lights_state
        call    check_timers
        jmp     osloop
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                                    ;
;                      MAIN OS LOOP END                              ;
;                                                                    ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
proc osloop_has_work?
        cmp     [osloop_nonperiodic_work], 0
        jnz     .yes
        call    stack_handler_has_work?
        jnz     .yes
        call    check_fdd_motor_status_has_work?
        jnz     .yes
        call    check_ATAPI_device_event_has_work?
        jnz     .yes
        call    check_lights_state_has_work?
        jnz     .yes
        call    check_timers_has_work?
        jnz     .yes
.no:
        xor     eax, eax
        ret
.yes:
        xor     eax, eax
        inc     eax
        ret
endp

proc wakeup_osloop
        mov     [osloop_nonperiodic_work], 1
        ret
endp

uglobal
align 4
osloop_nonperiodic_work dd      ?
endg

align 4
idle_thread:
        sti
idle_loop:
        hlt
        jmp     idle_loop



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                                      ;
;                   INCLUDED SYSTEM FILES                              ;
;                                                                      ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


include "kernel32.inc"


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                                      ;
;                       KERNEL FUNCTIONS                               ;
;                                                                      ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

reserve_irqs_ports:


; RESERVE PORTS
        mov     eax, RESERVED_PORTS
        mov     ecx, 1

        mov     [eax], dword 4

        mov     [eax+16], ecx
        mov     [eax+16+4], dword 0
        mov     [eax+16+8], dword 0x2D

        mov     [eax+32], ecx
        mov     [eax+32+4], dword 0x30
        mov     [eax+32+8], dword 0x4D

        mov     [eax+48], ecx
        mov     [eax+48+4], dword 0x50
        mov     [eax+48+8], dword 0xDF

        mov     [eax+64], ecx
        mov     [eax+64+4], dword 0xE5
        mov     [eax+64+8], dword 0xFF

        ret


iglobal
  process_number dd 0x2
endg

set_variables:

        mov     ecx, 0x16                    ; flush port 0x60
.fl60:
        in      al, 0x60
        loop    .fl60
        push    eax

        mov     ax, [BOOT_VARS+BOOT_Y_RES]
        shr     ax, 1
        shl     eax, 16
        mov     ax, [BOOT_VARS+BOOT_X_RES]
        shr     ax, 1
        mov     [MOUSE_X], eax
        call    wakeup_osloop

        xor     eax, eax
        mov     [BTN_ADDR], dword BUTTON_INFO ; address of button list

        mov     byte [KEY_COUNT], al              ; keyboard buffer
        mov     byte [BTN_COUNT], al              ; button buffer
;        mov   [MOUSE_X],dword 100*65536+100    ; mouse x/y

        pop     eax
        ret

align 4
;input  eax=43,bl-byte of output, ecx - number of port
sys_outport:

        mov     edi, ecx   ; separate flag for read / write
        and     ecx, 65535

        mov     eax, [RESERVED_PORTS]
        test    eax, eax
        jnz     .sopl8
        inc     eax
        mov     [esp+32], eax
        ret

  .sopl8:
        mov     edx, [TASK_BASE]
        mov     edx, [edx+0x4]
    ;and   ecx,65535
    ;cld - set on interrupt 0x40
  .sopl1:

        mov     esi, eax
        shl     esi, 4
        add     esi, RESERVED_PORTS
        cmp     edx, [esi+0]
        jne     .sopl2
        cmp     ecx, [esi+4]
        jb      .sopl2
        cmp     ecx, [esi+8]
        jg      .sopl2
.sopl3:

        test    edi, 0x80000000; read ?
        jnz     .sopl4

        mov     eax, ebx
        mov     dx, cx   ; write
        out     dx, al
        and     [esp+32], dword 0
        ret

        .sopl2:

        dec     eax
        jnz     .sopl1
        inc     eax
        mov     [esp+32], eax
        ret


  .sopl4:

        mov     dx, cx   ; read
        in      al, dx
        and     eax, 0xff
        and     [esp+32], dword 0
        mov     [esp+20], eax
        ret

display_number:
;It is not optimization
        mov     eax, ebx
        mov     ebx, ecx
        mov     ecx, edx
        mov     edx, esi
        mov     esi, edi
; eax = print type, al=0 -> ebx is number
;                   al=1 -> ebx is pointer
;                   ah=0 -> display decimal
;                   ah=1 -> display hexadecimal
;                   ah=2 -> display binary
;                   eax bits 16-21 = number of digits to display (0-32)
;                   eax bits 22-31 = reserved
;
; ebx = number or pointer
; ecx = x shl 16 + y
; edx = color
        xor     edi, edi
display_number_force:
        push    eax
        and     eax, 0x3fffffff
        cmp     eax, 0xffff     ; length > 0 ?
        pop     eax
        jge     cont_displ
        ret
   cont_displ:
        push    eax
        and     eax, 0x3fffffff
        cmp     eax, 61*0x10000  ; length <= 60 ?
        pop     eax
        jb      cont_displ2
        ret
   cont_displ2:

        pushad

        cmp     al, 1            ; ecx is a pointer ?
        jne     displnl1
        mov     ebp, ebx
        add     ebp, 4
        mov     ebp, [ebp+std_application_base_address]
        mov     ebx, [ebx+std_application_base_address]
 displnl1:
        sub     esp, 64

        test    ah, ah            ; DECIMAL
        jnz     no_display_desnum
        shr     eax, 16
        and     eax, 0xC03f
;     and   eax,0x3f
        push    eax
        and     eax, 0x3f
        mov     edi, esp
        add     edi, 4+64-1
        mov     ecx, eax
        mov     eax, ebx
        mov     ebx, 10
 d_desnum:
        xor     edx, edx
        call    division_64_bits
        div     ebx
        add     dl, 48
        mov     [edi], dl
        dec     edi
        loop    d_desnum
        pop     eax
        call    normalize_number
        call    draw_num_text
        add     esp, 64
        popad
        ret
   no_display_desnum:

        cmp     ah, 0x01         ; HEXADECIMAL
        jne     no_display_hexnum
        shr     eax, 16
        and     eax, 0xC03f
;     and   eax,0x3f
        push    eax
        and     eax, 0x3f
        mov     edi, esp
        add     edi, 4+64-1
        mov     ecx, eax
        mov     eax, ebx
        mov     ebx, 16
   d_hexnum:
        xor     edx, edx
        call    division_64_bits
        div     ebx
   hexletters = __fdo_hexdigits
        add     edx, hexletters
        mov     dl, [edx]
        mov     [edi], dl
        dec     edi
        loop    d_hexnum
        pop     eax
        call    normalize_number
        call    draw_num_text
        add     esp, 64
        popad
        ret
   no_display_hexnum:

        cmp     ah, 0x02         ; BINARY
        jne     no_display_binnum
        shr     eax, 16
        and     eax, 0xC03f
;     and   eax,0x3f
        push    eax
        and     eax, 0x3f
        mov     edi, esp
        add     edi, 4+64-1
        mov     ecx, eax
        mov     eax, ebx
        mov     ebx, 2
   d_binnum:
        xor     edx, edx
        call    division_64_bits
        div     ebx
        add     dl, 48
        mov     [edi], dl
        dec     edi
        loop    d_binnum
        pop     eax
        call    normalize_number
        call    draw_num_text
        add     esp, 64
        popad
        ret
   no_display_binnum:

        add     esp, 64
        popad
        ret

normalize_number:
        test    ah, 0x80
        jz      .continue
        mov     ecx, 48
        and     eax, 0x3f
@@:
        inc     edi
        cmp     [edi], cl
        jne     .continue
        dec     eax
        cmp     eax, 1
        ja      @r
        mov     al, 1
.continue:
        and     eax, 0x3f
        ret

division_64_bits:
        test    [esp+1+4], byte 0x40
        jz      .continue
        push    eax
        mov     eax, ebp
        div     ebx
        mov     ebp, eax
        pop     eax
.continue:
        ret

draw_num_text:
        mov     esi, eax
        mov     edx, 64+4
        sub     edx, eax
        add     edx, esp
        mov     ebx, [esp+64+32-8+4]
; add window start x & y
        mov     ecx, [TASK_BASE]

        mov     edi, [CURRENT_TASK]
        shl     edi, 8

        mov     eax, [ecx-twdw+WDATA.box.left]
        add     eax, [edi+SLOT_BASE+APPDATA.wnd_clientbox.left]
        shl     eax, 16
        add     eax, [ecx-twdw+WDATA.box.top]
        add     eax, [edi+SLOT_BASE+APPDATA.wnd_clientbox.top]
        add     ebx, eax
        mov     ecx, [esp+64+32-12+4]
        and     ecx, not 0x80000000     ; force counted string
        mov     eax, [esp+64+8]         ; background color (if given)
        mov     edi, [esp+64+4]
        jmp     dtext

align 4

sys_setup:

; 1=roland mpu midi base , base io address
; 2=keyboard   1, base kaybap 2, shift keymap, 9 country 1eng 2fi 3ger 4rus
; 3=cd base    1, pri.master 2, pri slave 3 sec master, 4 sec slave
; 5=system language, 1eng 2fi 3ger 4rus
; 7=hd base    1, pri.master 2, pri slave 3 sec master, 4 sec slave
; 8=fat32 partition in hd
; 9
; 10 = sound dma channel
; 11 = enable lba read
; 12 = enable pci access


        and     [esp+32], dword 0
        dec     ebx                             ; MIDI
        jnz     nsyse1
        cmp     ecx, 0x100

        jb      nsyse1
        mov     esi, 65535
        cmp     esi, ecx

        jb      nsyse1
        mov     [midi_base], cx ;bx
        mov     word [mididp], cx;bx
        inc     cx              ;bx
        mov     word [midisp], cx;bx
        ret

iglobal
midi_base dw 0
endg

   nsyse1:
        dec     ebx                              ; KEYBOARD
        jnz     nsyse2
        mov     edi, [TASK_BASE]
        mov     eax, [edi+TASKDATA.mem_start]
        add     eax, edx

        dec     ecx
        jnz     kbnobase
        mov     ebx, keymap
        mov     ecx, 128
        call    memmove
        ret
   kbnobase:
        dec     ecx
        jnz     kbnoshift

        mov     ebx, keymap_shift
        mov     ecx, 128
        call    memmove
        ret
   kbnoshift:
        dec     ecx
        jnz     kbnoalt
        mov     ebx, keymap_alt
        mov     ecx, 128
        call    memmove
        ret
   kbnoalt:
        sub     ecx, 6
        jnz     kbnocountry
        mov     word [keyboard], dx
        ret
   kbnocountry:
        mov     [esp+32], dword 1
        ret
   nsyse2:
        dec     ebx                         ; CD
        jnz     nsyse4

        test    ecx, ecx
        jz      nosesl

        cmp     ecx, 4
        ja      nosesl
        mov     [cd_base], cl

        dec     ecx
        jnz     noprma
        mov     eax, [hd_address_table]
        mov     [cdbase], eax   ;0x1f0
        mov     [cdid], 0xa0
   noprma:

        dec     ecx
        jnz     noprsl
        mov     eax, [hd_address_table]
        mov     [cdbase], eax   ;0x1f0
        mov     [cdid], 0xb0
   noprsl:
        dec     ecx
        jnz     nosema
        mov     eax, [hd_address_table+16]
        mov     [cdbase], eax   ;0x170
        mov     [cdid], 0xa0
   nosema:
        dec     ecx
        jnz     nosesl
        mov     eax, [hd_address_table+16]
        mov     [cdbase], eax   ;0x170
        mov     [cdid], 0xb0
   nosesl:
        ret

iglobal
cd_base db 0

endg
   nsyse4:

        sub     ebx, 2           ; SYSTEM LANGUAGE
        jnz     nsyse5
        mov     [syslang], ecx
        ret
   nsyse5:

        sub     ebx, 2          ; HD BASE - obsolete
        jnz     nsyse7

   nosethd:
        ret

nsyse7:

;     cmp  eax,8                      ; HD PARTITION - obsolete
        dec     ebx
        jnz     nsyse8
        ret

nsyse8:
;     cmp  eax,11                     ; ENABLE LBA READ
        and     ecx, 1
        sub     ebx, 3
        jnz     no_set_lba_read
        mov     [lba_read_enabled], ecx
        ret

no_set_lba_read:
;     cmp  eax,12                     ; ENABLE PCI ACCESS
        dec     ebx
        jnz     sys_setup_err
        mov     [pci_access_enabled], ecx
        ret

sys_setup_err:
        or      [esp+32], dword -1
        ret

align 4

sys_getsetup:

; 1=roland mpu midi base , base io address
; 2=keyboard   1, base kaybap 2, shift keymap, 9 country 1eng 2fi 3ger 4rus
; 3=cd base    1, pri.master 2, pri slave 3 sec master, 4 sec slave
; 5=system language, 1eng 2fi 3ger 4rus
; 7=hd base    1, pri.master 2, pri slave 3 sec master, 4 sec slave
; 8=fat32 partition in hd
; 9=get hs timer tic

;     cmp  eax,1
        dec     ebx
        jnz     ngsyse1
        movzx   eax, [midi_base]
        mov     [esp+32], eax
        ret
ngsyse1:
;     cmp  eax,2
        dec     ebx
        jnz     ngsyse2

        mov     edi, [TASK_BASE]
        mov     ebx, [edi+TASKDATA.mem_start]
        add     ebx, edx

;     cmp  ebx,1
        dec     ecx
        jnz     kbnobaseret
        mov     eax, keymap
        mov     ecx, 128
        call    memmove
        ret
kbnobaseret:
;     cmp  ebx,2
        dec     ecx
        jnz     kbnoshiftret

        mov     eax, keymap_shift
        mov     ecx, 128
        call    memmove
        ret
kbnoshiftret:
;     cmp  ebx,3
        dec     ecx
        jne     kbnoaltret

        mov     eax, keymap_alt
        mov     ecx, 128
        call    memmove
        ret
kbnoaltret:
;     cmp  ebx,9
        sub     ecx, 6
        jnz     ngsyse2
        movzx   eax, word [keyboard]
        mov     [esp+32], eax
        ret


ngsyse2:
;         cmp  eax,3
        dec     ebx
        jnz     ngsyse3
        movzx   eax, [cd_base]
        mov     [esp+32], eax
        ret
ngsyse3:
;         cmp  eax,5
        sub     ebx, 2
        jnz     ngsyse5
        mov     eax, [syslang]
        mov     [esp+32], eax
        ret
ngsyse5:
;     cmp  eax,9
        sub     ebx, 4
        jnz     ngsyse9
        mov     eax, [timer_ticks];[0xfdf0]
        mov     [esp+32], eax
        ret
ngsyse9:
;     cmp  eax,11
        sub     ebx, 2
        jnz     ngsyse11
        mov     eax, [lba_read_enabled]
        mov     [esp+32], eax
        ret
ngsyse11:
;     cmp  eax,12
        dec     ebx
        jnz     ngsyse12
        mov     eax, [pci_access_enabled]
        mov     [esp+32], eax
        ret
ngsyse12:
        mov     [esp+32], dword 1
        ret


get_timer_ticks:
        mov     eax, [timer_ticks]
        ret

iglobal
align 4
mousefn dd msscreen, mswin, msbutton, msset
        dd app_load_cursor
        dd app_set_cursor
        dd app_delete_cursor
        dd msz
endg

readmousepos:

; eax=0 screen relative
; eax=1 window relative
; eax=2 buttons pressed
; eax=3 set mouse pos   ; reserved
; eax=4 load cursor
; eax=5 set cursor
; eax=6 delete cursor   ; reserved
; eax=7 get mouse_z

        cmp     ebx, 7
        ja      msset
        jmp     [mousefn+ebx*4]
msscreen:
        mov     eax, [MOUSE_X]
        shl     eax, 16
        mov     ax, [MOUSE_Y]
        mov     [esp+36-4], eax
        ret
mswin:
        mov     eax, [MOUSE_X]
        shl     eax, 16
        mov     ax, [MOUSE_Y]
        mov     esi, [TASK_BASE]
        mov     bx, word [esi-twdw+WDATA.box.left]
        shl     ebx, 16
        mov     bx, word [esi-twdw+WDATA.box.top]
        sub     eax, ebx

        mov     edi, [CURRENT_TASK]
        shl     edi, 8
        sub     ax, word[edi+SLOT_BASE+APPDATA.wnd_clientbox.top]
        rol     eax, 16
        sub     ax, word[edi+SLOT_BASE+APPDATA.wnd_clientbox.left]
        rol     eax, 16
        mov     [esp+36-4], eax
        ret
msbutton:
        movzx   eax, byte [BTN_DOWN]
        mov     [esp+36-4], eax
        ret
msz:
        mov     edi, [TASK_COUNT]
        movzx   edi, word [WIN_POS + edi*2]
        cmp     edi, [CURRENT_TASK]
        jne     @f
        mov     ax, [MOUSE_SCROLL_H]
        shl     eax, 16
        mov     ax, [MOUSE_SCROLL_V]
        mov     [esp+36-4], eax
        and     [MOUSE_SCROLL_H], word 0
        and     [MOUSE_SCROLL_V], word 0
        ret
       @@:
        and     [esp+36-4], dword 0
;           ret
msset:
        ret

app_load_cursor:
        cmp     ecx, OS_BASE
        jae     msset
        stdcall load_cursor, ecx, edx
        mov     [esp+36-4], eax
        ret

app_set_cursor:
        stdcall set_cursor, ecx
        mov     [esp+36-4], eax
        ret

app_delete_cursor:
        stdcall delete_cursor, ecx
        mov     [esp+36-4], eax
        ret

is_input:

        push    edx
        mov     dx, word [midisp]
        in      al, dx
        and     al, 0x80
        pop     edx
        ret

is_output:

        push    edx
        mov     dx, word [midisp]
        in      al, dx
        and     al, 0x40
        pop     edx
        ret


get_mpu_in:

        push    edx
        mov     dx, word [mididp]
        in      al, dx
        pop     edx
        ret


put_mpu_out:

        push    edx
        mov     dx, word [mididp]
        out     dx, al
        pop     edx
        ret



align 4

sys_midi:
        cmp     [mididp], 0
        jnz     sm0
        mov     [esp+36], dword 1
        ret
sm0:
        and     [esp+36], dword 0
        dec     ebx
        jnz     smn1
 ;    call setuart
su1:
        call    is_output
        test    al, al
        jnz     su1
        mov     dx, word [midisp]
        mov     al, 0xff
        out     dx, al
su2:
        mov     dx, word [midisp]
        mov     al, 0xff
        out     dx, al
        call    is_input
        test    al, al
        jnz     su2
        call    get_mpu_in
        cmp     al, 0xfe
        jnz     su2
su3:
        call    is_output
        test    al, al
        jnz     su3
        mov     dx, word [midisp]
        mov     al, 0x3f
        out     dx, al
        ret
smn1:
        dec     ebx
        jnz     smn2
sm10:
        call    get_mpu_in
        call    is_output
        test    al, al
        jnz     sm10
        mov     al, bl
        call    put_mpu_out
        smn2:
        ret

detect_devices:
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;include 'detect/commouse.inc'
;include 'detect/ps2mouse.inc'
;include 'detect/dev_fd.inc'
;include 'detect/dev_hdcd.inc'
;include 'detect/sear_par.inc'
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        ret

sys_end:
;--------------------------------------
        cmp     [_display.select_cursor], 0
        je      @f
; restore default cursor before killing
        pusha
        mov     ecx, [current_slot]
        call    restore_default_cursor_before_killing
        popa
@@:
;--------------------------------------
; kill all sockets this process owns
        pusha
        mov     edx, [TASK_BASE]
        mov     edx, [edx+TASKDATA.pid]
        call    SOCKET_process_end
        popa
;--------------------------------------
        mov     ecx, [current_slot]
        mov     eax, [ecx+APPDATA.tls_base]
        test    eax, eax
        jz      @F

        stdcall user_free, eax
@@:

        mov     eax, [TASK_BASE]
        mov     [eax+TASKDATA.state], 3; terminate this program
        call    wakeup_osloop

.waitterm:            ; wait here for termination
        call    change_task
        jmp     .waitterm
;------------------------------------------------------------------------------
align 4
restore_default_cursor_before_killing:
        pushfd
        cli
        mov     eax, [def_cursor]
        mov     [ecx+APPDATA.cursor], eax

        movzx   eax, word [MOUSE_Y]
        movzx   ebx, word [MOUSE_X]
;        mov     ecx, [Screen_Max_X]
;        inc     ecx
;        mul     ecx
        mov     eax, [d_width_calc_area + eax*4]

        add     eax, [_WinMapAddress]
        movzx   edx, byte [ebx+eax]
        shl     edx, 8
        mov     esi, [edx+SLOT_BASE+APPDATA.cursor]

        cmp     esi, [current_cursor]
        je      @f

        push    esi
        call    [_display.select_cursor]
        mov     [current_cursor], esi
@@:
        mov     [redrawmouse_unconditional], 1
        call    wakeup_osloop
        popfd
        ret
;------------------------------------------------------------------------------
iglobal
align 4
sys_system_table:
        dd      sysfn_deactivate        ; 1 = deactivate window
        dd      sysfn_terminate         ; 2 = terminate thread
        dd      sysfn_activate          ; 3 = activate window
        dd      sysfn_getidletime       ; 4 = get idle time
        dd      sysfn_getcpuclock       ; 5 = get cpu clock
        dd      sysfn_saveramdisk       ; 6 = save ramdisk
        dd      sysfn_getactive         ; 7 = get active window
        dd      sysfn_sound_flag        ; 8 = get/set sound_flag
        dd      sysfn_shutdown          ; 9 = shutdown with parameter
        dd      sysfn_minimize          ; 10 = minimize window
        dd      sysfn_getdiskinfo       ; 11 = get disk subsystem info
        dd      sysfn_lastkey           ; 12 = get last pressed key
        dd      sysfn_getversion        ; 13 = get kernel version
        dd      sysfn_waitretrace       ; 14 = wait retrace
        dd      sysfn_centermouse       ; 15 = center mouse cursor
        dd      sysfn_getfreemem        ; 16 = get free memory size
        dd      sysfn_getallmem         ; 17 = get total memory size
        dd      sysfn_terminate2        ; 18 = terminate thread using PID
                                        ;                 instead of slot
        dd      sysfn_mouse_acceleration; 19 = set/get mouse acceleration
        dd      sysfn_meminfo           ; 20 = get extended memory info
        dd      sysfn_pid_to_slot       ; 21 = get slot number for pid
        dd      sysfn_min_rest_window   ; 22 = minimize and restore any window
        dd      sysfn_min_windows       ; 23 = minimize all windows
        dd      sysfn_set_screen_sizes  ; 24 = set screen sizes for Vesa
sysfn_num = ($ - sys_system_table)/4
endg
;------------------------------------------------------------------------------
sys_system:
        dec     ebx
        cmp     ebx, sysfn_num
        jae     @f
        jmp     dword [sys_system_table + ebx*4]
@@:
        ret
;------------------------------------------------------------------------------
sysfn_shutdown:          ; 18.9 = system shutdown
        cmp     ecx, 1
        jl      exit_for_anyone
        cmp     ecx, 4
        jg      exit_for_anyone
        mov     [BOOT_VARS+0x9030], cl

        mov     eax, [TASK_COUNT]
        mov     [SYS_SHUTDOWN], al
        mov     [shutdown_processes], eax
        call    wakeup_osloop
        and     dword [esp+32], 0
 exit_for_anyone:
        ret
  uglobal
   shutdown_processes:
                       dd 0x0
  endg
;------------------------------------------------------------------------------
sysfn_terminate:        ; 18.2 = TERMINATE
        push    ecx
        cmp     ecx, 2
        jb      noprocessterminate
        mov     edx, [TASK_COUNT]
        cmp     ecx, edx
        ja      noprocessterminate
        mov     eax, [TASK_COUNT]
        shl     ecx, 5
        mov     edx, [ecx+CURRENT_TASK+TASKDATA.pid]
        add     ecx, CURRENT_TASK+TASKDATA.state
        cmp     byte [ecx], 9
        jz      noprocessterminate
        push    ecx edx
        lea     edx, [(ecx-(CURRENT_TASK and 1FFFFFFFh)-TASKDATA.state)*8+SLOT_BASE]
        call    request_terminate
        pop     edx ecx
        test    eax, eax
        jz      noprocessterminate
;--------------------------------------
; terminate all network sockets it used
        pusha
        mov     eax, edx
        call    SOCKET_process_end
        popa
;--------------------------------------
        cmp     [_display.select_cursor], 0
        je      .restore_end
; restore default cursor before killing
        pusha
        mov     ecx, [esp+32]
        shl     ecx, 8
        add     ecx, SLOT_BASE
        mov     eax, [def_cursor]
        cmp     [ecx+APPDATA.cursor], eax
        je      @f
        call    restore_default_cursor_before_killing
@@:
        popa
.restore_end:
;--------------------------------------
     ;call MEM_Heap_Lock      ;guarantee that process isn't working with heap
        mov     [ecx], byte 3; clear possible i40's
        call    wakeup_osloop
     ;call MEM_Heap_UnLock

        cmp     edx, [application_table_owner]; clear app table stat
        jne     noatsc
        call    unlock_application_table
noatsc:
noprocessterminate:
        add     esp, 4
        ret
;------------------------------------------------------------------------------
sysfn_terminate2:
;lock application_table_status mutex
.table_status:
        call    lock_application_table
        mov     eax, ecx
        call    pid_to_slot
        test    eax, eax
        jz      .not_found
        mov     ecx, eax
        cli
        call    sysfn_terminate
        call    unlock_application_table
        sti
        and     dword [esp+32], 0
        ret
.not_found:
        call    unlock_application_table
        or      dword [esp+32], -1
        ret
;------------------------------------------------------------------------------
sysfn_deactivate:         ; 18.1 = DEACTIVATE WINDOW
        cmp     ecx, 2
        jb      .nowindowdeactivate
        cmp     ecx, [TASK_COUNT]
        ja      .nowindowdeactivate

        movzx   esi, word [WIN_STACK + ecx*2]
        cmp     esi, 1
        je      .nowindowdeactivate ; already deactive

        mov     edi, ecx
        shl     edi, 5
        add     edi, window_data
        movzx   esi, word [WIN_STACK + ecx * 2]
        lea     esi, [WIN_POS + esi * 2]
        call    window._.window_deactivate

        call    syscall_display_settings._.calculate_whole_screen
        call    syscall_display_settings._.redraw_whole_screen
.nowindowdeactivate:
        ret
;------------------------------------------------------------------------------
sysfn_activate:         ; 18.3 = ACTIVATE WINDOW
        cmp     ecx, 2
        jb      .nowindowactivate
        cmp     ecx, [TASK_COUNT]
        ja      .nowindowactivate
;-------------------------------------
@@:
; If the window is captured and moved by the user,
; then you can't change the position in window stack!!!
        mov     al, [mouse.active_sys_window.action]
        and     al, WINDOW_MOVE_AND_RESIZE_FLAGS
        test    al, al
        jz      @f
        call    change_task
        jmp     @b
@@:
;-------------------------------------
        mov     [window_minimize], 2; restore window if minimized
        call    wakeup_osloop

        movzx   esi, word [WIN_STACK + ecx*2]
        cmp     esi, [TASK_COUNT]
        je      .nowindowactivate; already active

        mov     edi, ecx
        shl     edi, 5
        add     edi, window_data
        movzx   esi, word [WIN_STACK + ecx * 2]
        lea     esi, [WIN_POS + esi * 2]
        call    waredraw
.nowindowactivate:
        ret
;------------------------------------------------------------------------------
sysfn_getidletime:              ; 18.4 = GET IDLETIME
        mov     eax, [CURRENT_TASK+32+TASKDATA.cpu_usage]
        mov     [esp+32], eax
        ret
;------------------------------------------------------------------------------
sysfn_getcpuclock:              ; 18.5 = GET TSC/SEC
        mov     eax, dword [cpu_freq]
        mov     [esp+32], eax
        ret
;------------------------------------------------------------------------------
get_cpu_freq:
        mov     eax, dword [cpu_freq]
        mov     edx, dword [cpu_freq+4]
        ret
;  SAVE ramdisk to /hd/1/menuet.img
;!!!!!!!!!!!!!!!!!!!!!!!!
   include 'blkdev/rdsave.inc'
;!!!!!!!!!!!!!!!!!!!!!!!!
;------------------------------------------------------------------------------
align 4
sysfn_getactive:        ; 18.7 = get active window
        mov     eax, [TASK_COUNT]
        movzx   eax, word [WIN_POS + eax*2]
        mov     [esp+32], eax
        ret
;------------------------------------------------------------------------------
sysfn_sound_flag:       ; 18.8 = get/set sound_flag
;     cmp  ecx,1
        dec     ecx
        jnz     nogetsoundflag
        movzx   eax, byte [sound_flag]; get sound_flag
        mov     [esp+32], eax
        ret
 nogetsoundflag:
;     cmp  ecx,2
        dec     ecx
        jnz     nosoundflag
        xor     byte [sound_flag], 1
 nosoundflag:
        ret
;------------------------------------------------------------------------------
sysfn_minimize:         ; 18.10 = minimize window
        mov     [window_minimize], 1
        call    wakeup_osloop
        ret
;------------------------------------------------------------------------------
align 4
sysfn_getdiskinfo:      ; 18.11 = get disk info table
;     cmp  ecx,1
        dec     ecx
        jnz     full_table
  small_table:
        call    for_all_tables
        mov     ecx, 10
        cld
        rep movsb
        ret
   for_all_tables:
        mov     edi, edx
        mov     esi, DRIVE_DATA
        ret
  full_table:
;     cmp  ecx,2
        dec     ecx
        jnz     exit_for_anyone
        call    for_all_tables
        mov     ecx, DRIVE_DATA_SIZE/4
        cld
        rep movsd
        ret
;------------------------------------------------------------------------------
sysfn_lastkey:          ; 18.12 = return 0 (backward compatibility)
        and     dword [esp+32], 0
        ret
;------------------------------------------------------------------------------
sysfn_getversion:       ; 18.13 = get kernel ID and version
        mov     edi, ecx
        mov     esi, version_inf
        mov     ecx, version_end-version_inf
        rep movsb
        ret
;------------------------------------------------------------------------------
sysfn_waitretrace:     ; 18.14 = sys wait retrace
     ;wait retrace functions
 sys_wait_retrace:
        mov     edx, 0x3da
 WaitRetrace_loop:
        in      al, dx
        test    al, 1000b
        jz      WaitRetrace_loop
        and     [esp+32], dword 0
        ret
;------------------------------------------------------------------------------
align 4
sysfn_centermouse:      ; 18.15 = mouse centered
; removed here by <Lrz>
;     call  mouse_centered
;* mouse centered - start code- Mario79
;mouse_centered:
;        push  eax
        mov     eax, [Screen_Max_X]
        shr     eax, 1
        mov     [MOUSE_X], ax
        mov     eax, [Screen_Max_Y]
        shr     eax, 1
        mov     [MOUSE_Y], ax
        call    wakeup_osloop
;        ret
;* mouse centered - end code- Mario79
        xor     eax, eax
        and     [esp+32], eax
;        pop   eax
        ret
;------------------------------------------------------------------------------
align 4
sysfn_mouse_acceleration: ; 18.19 = set/get mouse features
        test    ecx, ecx; get mouse speed factor
        jnz     .set_mouse_acceleration
        xor     eax, eax
        mov     ax, [mouse_speed_factor]
        mov     [esp+32], eax
        ret
 .set_mouse_acceleration:
;     cmp  ecx,1  ; set mouse speed factor
        dec     ecx
        jnz     .get_mouse_delay
        mov     [mouse_speed_factor], dx
        ret
 .get_mouse_delay:
;     cmp  ecx,2  ; get mouse delay
        dec     ecx
        jnz     .set_mouse_delay
        mov     eax, [mouse_delay]
        mov     [esp+32], eax
        ret
 .set_mouse_delay:
;     cmp  ecx,3  ; set mouse delay
        dec     ecx
        jnz     .set_pointer_position
        mov     [mouse_delay], edx
        ret
 .set_pointer_position:
;     cmp  ecx,4  ; set mouse pointer position
        dec     ecx
        jnz     .set_mouse_button
        cmp     dx, word[Screen_Max_Y]
        ja      .end
        rol     edx, 16
        cmp     dx, word[Screen_Max_X]
        ja      .end
        mov     [MOUSE_X], edx
        mov     [mouse_active], 1
        call    wakeup_osloop
        ret
 .set_mouse_button:
;     cmp   ecx,5  ; set mouse button features
        dec     ecx
        jnz     .end
        mov     [BTN_DOWN], dl
        mov     [mouse_active], 1
        call    wakeup_osloop
 .end:
        ret
;------------------------------------------------------------------------------
sysfn_getfreemem:
        mov     eax, [pg_data.pages_free]
        shl     eax, 2
        mov     [esp+32], eax
        ret
;------------------------------------------------------------------------------
sysfn_getallmem:
        mov     eax, [MEM_AMOUNT]
        shr     eax, 10
        mov     [esp+32], eax
        ret
;------------------------------------------------------------------------------
sysfn_pid_to_slot:
        mov     eax, ecx
        call    pid_to_slot
        mov     [esp+32], eax
        ret
;------------------------------------------------------------------------------
sysfn_min_rest_window:
        pushad
        mov     eax, edx ; ebx - operating
        shr     ecx, 1
        jnc     @f
        call    pid_to_slot
@@:
        or      eax, eax ; eax - number of slot
        jz      .error
        cmp     eax, 255    ; varify maximal slot number
        ja      .error
        movzx   eax, word [WIN_STACK + eax*2]
        shr     ecx, 1
        jc      .restore
 ; .minimize:
        call    minimize_window
        jmp     .exit
.restore:
        call    restore_minimized_window
.exit:
        popad
        xor     eax, eax
        mov     [esp+32], eax
        ret
.error:
        popad
        xor     eax, eax
        dec     eax
        mov     [esp+32], eax
        ret
;------------------------------------------------------------------------------
sysfn_min_windows:
        call    minimize_all_window
        mov     [esp+32], eax
        call    change_task
        ret
;------------------------------------------------------------------------------
sysfn_set_screen_sizes:
        cmp     [SCR_MODE], word 0x13
        jbe     .exit

        cmp     [_display.select_cursor], select_cursor
        jne     .exit

        cmp     ecx, [display_width_standard]
        ja      .exit

        cmp     edx, [display_height_standard]
        ja      .exit

        pushfd
        cli
        mov     eax, ecx
        mov     ecx, [_display.pitch]
        mov     [_display.width], eax
        dec     eax
        mov     [_display.height], edx
        dec     edx
; eax - new Screen_Max_X
; edx - new Screen_Max_Y
        mov     [do_not_touch_winmap], 1
        call    set_screen
        mov     [do_not_touch_winmap], 0
        popfd
        call    change_task
.exit:
        ret
;------------------------------------------------------------------------------
uglobal
screen_workarea RECT
display_width_standard dd 0
display_height_standard dd 0
do_not_touch_winmap db 0
window_minimize db 0
sound_flag      db 0

endg

UID_NONE=0
UID_MENUETOS=1   ;official
UID_KOLIBRI=2    ;russian

iglobal
version_inf:
        db 0,7,7,0  ; version 0.7.7.0
        db 0
.rev    dd __REV__
version_end:
endg
;------------------------------------------------------------------------------
align 4
sys_cachetodiskette:
        cmp     ebx, 1
        jb      .no_floppy_save
        cmp     ebx, 2
        ja      .no_floppy_save
        call    save_image
        mov     [esp + 32], eax
        ret
.no_floppy_save:
        mov     [esp + 32], dword 1
        ret
;------------------------------------------------------------------------------
uglobal
;  bgrchanged  dd  0x0
align 4
bgrlockpid dd 0
bgrlock db 0
endg
;------------------------------------------------------------------------------
align 4
sys_background:
        cmp     ebx, 1                     ; BACKGROUND SIZE
        jnz     nosb1
        test    ecx, ecx
        jz      sbgrr

        test    edx, edx
        jz      sbgrr
;--------------------------------------
align 4
@@:
;;Maxis use atomic bts for mutexes  4.4.2009
        bts     dword [bgrlock], 0
        jnc     @f
        call    change_task
        jmp     @b
;--------------------------------------
align 4
@@:
        mov     [BgrDataWidth], ecx
        mov     [BgrDataHeight], edx
;    mov   [bgrchanged],1

        pushad
; return memory for old background
        mov     eax, [img_background]
        cmp     eax, static_background_data
        jz      @f
        stdcall kernel_free, eax
;--------------------------------------
align 4
@@:
; calculate RAW size
        xor     eax, eax
        inc     eax
        cmp     [BgrDataWidth], eax
        jae     @f
        mov     [BgrDataWidth], eax
;--------------------------------------
align 4
@@:
        cmp     [BgrDataHeight], eax
        jae     @f
        mov     [BgrDataHeight], eax
;--------------------------------------
align 4
@@:
        mov     eax, [BgrDataWidth]
        imul    eax, [BgrDataHeight]
        lea     eax, [eax*3]
; it is reserved with aligned to the boundary of 4 KB pages,
; otherwise there may be exceptions a page fault for vesa20_drawbackground_tiled
; because the 32 bit read is used for  high performance: "mov eax,[esi]"
        shr     eax, 12
        inc     eax
        shl     eax, 12
        mov     [mem_BACKGROUND], eax
; get memory for new background
        stdcall kernel_alloc, eax
        test    eax, eax
        jz      .memfailed
        mov     [img_background], eax
        jmp     .exit
;--------------------------------------
align 4
.memfailed:
; revert to static monotone data
        mov     [img_background], static_background_data
        xor     eax, eax
        inc     eax
        mov     [BgrDataWidth], eax
        mov     [BgrDataHeight], eax
        mov     [mem_BACKGROUND], 4
;--------------------------------------
align 4
.exit:
        popad
        mov     [bgrlock], 0
;--------------------------------------
align 4
sbgrr:
        ret
;------------------------------------------------------------------------------
align 4
nosb1:
        cmp     ebx, 2                     ; SET PIXEL
        jnz     nosb2

        mov     eax, [img_background]
        test    ecx, ecx
        jz      @f
        cmp     eax, static_background_data
        jz      .ret
;--------------------------------------
align 4
@@:
        mov     ebx, [mem_BACKGROUND]
        add     ebx, 4095
        and     ebx, -4096
        sub     ebx, 4
        cmp     ecx, ebx
        ja      .ret

        mov     ebx, [eax+ecx]
        and     ebx, 0xFF000000;255*256*256*256
        and     edx, 0x00FFFFFF;255*256*256+255*256+255
        add     edx, ebx
        mov     [eax+ecx], edx
;--------------------------------------
align 4
.ret:
        ret
;------------------------------------------------------------------------------
align 4
nosb2:
        cmp     ebx, 3                     ; DRAW BACKGROUND
        jnz     nosb3
;--------------------------------------
align 4
draw_background_temp:
        mov     [background_defined], 1
        call    force_redraw_background
;--------------------------------------
align 4
nosb31:
        ret
;------------------------------------------------------------------------------
align 4
nosb3:
        cmp     ebx, 4                     ; TILED / STRETCHED
        jnz     nosb4
        cmp     ecx, [BgrDrawMode]
        je      nosb41
        mov     [BgrDrawMode], ecx
;--------------------------------------
align 4
nosb41:
        ret
;------------------------------------------------------------------------------
align 4
nosb4:
        cmp     ebx, 5                     ; BLOCK MOVE TO BGR
        jnz     nosb5
        cmp     [img_background], static_background_data
        jnz     @f
        test    edx, edx
        jnz     .fin
        cmp     esi, 4
        ja      .fin
;--------------------------------------
align 4
@@:
  ; bughere
        mov     eax, ecx
        mov     ebx, edx
        add     ebx, [img_background];IMG_BACKGROUND
        mov     ecx, esi
        call    memmove
;--------------------------------------
align 4
.fin:
        ret
;------------------------------------------------------------------------------
align 4
nosb5:
        cmp     ebx, 6
        jnz     nosb6
;--------------------------------------
align 4
;;Maxis use atomic bts for mutex 4.4.2009
@@:
        bts     dword [bgrlock], 0
        jnc     @f
        call    change_task
        jmp     @b
;--------------------------------------
align 4
@@:
        mov     eax, [CURRENT_TASK]
        mov     [bgrlockpid], eax
        cmp     [img_background], static_background_data
        jz      .nomem
        stdcall user_alloc, [mem_BACKGROUND]
        mov     [esp+32], eax
        test    eax, eax
        jz      .nomem
        mov     ebx, eax
        shr     ebx, 12
        or      dword [page_tabs+(ebx-1)*4], DONT_FREE_BLOCK
        mov     esi, [img_background]
        shr     esi, 12
        mov     ecx, [mem_BACKGROUND]
        add     ecx, 0xFFF
        shr     ecx, 12
;--------------------------------------
align 4
.z:
        mov     eax, [page_tabs+ebx*4]
        test    al, 1
        jz      @f
        call    free_page
;--------------------------------------
align 4
@@:
        mov     eax, [page_tabs+esi*4]
        or      al, PG_UW
        mov     [page_tabs+ebx*4], eax
        mov     eax, ebx
        shl     eax, 12
        invlpg  [eax]
        inc     ebx
        inc     esi
        loop    .z
        ret
;--------------------------------------
align 4
.nomem:
        and     [bgrlockpid], 0
        mov     [bgrlock], 0
;------------------------------------------------------------------------------
align 4
nosb6:
        cmp     ebx, 7
        jnz     nosb7
        cmp     [bgrlock], 0
        jz      .err
        mov     eax, [CURRENT_TASK]
        cmp     [bgrlockpid], eax
        jnz     .err
        mov     eax, ecx
        mov     ebx, ecx
        shr     eax, 12
        mov     ecx, [page_tabs+(eax-1)*4]
        test    cl, USED_BLOCK+DONT_FREE_BLOCK
        jz      .err
        jnp     .err
        push    eax
        shr     ecx, 12
        dec     ecx
;--------------------------------------
align 4
@@:
        and     dword [page_tabs+eax*4], 0
        mov     edx, eax
        shl     edx, 12
        push    eax
        invlpg  [edx]
        pop     eax
        inc     eax
        loop    @b
        pop     eax
        and     dword [page_tabs+(eax-1)*4], not DONT_FREE_BLOCK
        stdcall user_free, ebx
        mov     [esp+32], eax
        and     [bgrlockpid], 0
        mov     [bgrlock], 0
        ret
;--------------------------------------
align 4
.err:
        and     dword [esp+32], 0
        ret
;------------------------------------------------------------------------------
align 4
nosb7:
        cmp     ebx, 8
        jnz     nosb8

        mov     ecx, [current_slot]
        xor     eax, eax
        xchg    eax, [ecx+APPDATA.draw_bgr_x]
        mov     [esp + 32], eax ; eax = [left]*65536 + [right]
        xor     eax, eax
        xchg    eax, [ecx+APPDATA.draw_bgr_y]
        mov     [esp + 20], eax ; ebx = [top]*65536 + [bottom]
        ret
;------------------------------------------------------------------------------
align 4
nosb8:
        cmp     ebx, 9
        jnz     nosb9
; ecx = [left]*65536 + [right]
; edx = [top]*65536 + [bottom]
        mov     eax, [Screen_Max_X]
        mov     ebx, [Screen_Max_Y]
; check [right]
        cmp     cx, ax
        ja      .exit
; check [left]
        ror     ecx, 16
        cmp     cx, ax
        ja      .exit
; check [bottom]
        cmp     dx, bx
        ja      .exit
; check [top]
        ror     edx, 16
        cmp     dx, bx
        ja      .exit

        movzx   eax, cx  ; [left]
        movzx   ebx, dx  ; [top]

        shr     ecx, 16 ; [right]
        shr     edx, 16 ; [bottom]

        mov     [background_defined], 1

        mov     [draw_data+32 + RECT.left], eax
        mov     [draw_data+32 + RECT.top], ebx

        mov     [draw_data+32 + RECT.right], ecx
        mov     [draw_data+32 + RECT.bottom], edx

        inc     byte[REDRAW_BACKGROUND]
        call    wakeup_osloop
;--------------------------------------
align 4
.exit:
        ret
;------------------------------------------------------------------------------
align 4
nosb9:
        ret
;------------------------------------------------------------------------------
align 4
uglobal
  BG_Rect_X_left_right  dd   0x0
  BG_Rect_Y_top_bottom  dd   0x0
endg
;------------------------------------------------------------------------------
align 4
force_redraw_background:
        and     [draw_data+32 + RECT.left], 0
        and     [draw_data+32 + RECT.top], 0
        push    eax ebx
        mov     eax, [Screen_Max_X]
        mov     ebx, [Screen_Max_Y]
        mov     [draw_data+32 + RECT.right], eax
        mov     [draw_data+32 + RECT.bottom], ebx
        pop     ebx eax
        inc     byte[REDRAW_BACKGROUND]
        call    wakeup_osloop
        ret
;------------------------------------------------------------------------------
align 4
sys_getbackground:
;    cmp   eax,1                                  ; SIZE
        dec     ebx
        jnz     nogb1
        mov     eax, [BgrDataWidth]
        shl     eax, 16
        mov     ax, word [BgrDataHeight]
        mov     [esp+32], eax
        ret
;------------------------------------------------------------------------------
align 4
nogb1:
;    cmp   eax,2                                  ; PIXEL
        dec     ebx
        jnz     nogb2

        mov     eax, [img_background]
        test    ecx, ecx
        jz      @f
        cmp     eax, static_background_data
        jz      .ret
;--------------------------------------
align 4
@@:
        mov     ebx, [mem_BACKGROUND]
        add     ebx, 4095
        and     ebx, -4096
        sub     ebx, 4
        cmp     ecx, ebx
        ja      .ret

        mov     eax, [ecx+eax]

        and     eax, 0xFFFFFF
        mov     [esp+32], eax
;--------------------------------------
align 4
.ret:
        ret
;------------------------------------------------------------------------------
align 4
nogb2:

;    cmp   eax,4                                  ; TILED / STRETCHED
        dec     ebx
        dec     ebx
        jnz     nogb4
        mov     eax, [BgrDrawMode]
;--------------------------------------
align 4
nogb4:
        mov     [esp+32], eax
        ret
;------------------------------------------------------------------------------
align 4
sys_getkey:
        mov     [esp + 32], dword 1
        ; test main buffer
        mov     ebx, [CURRENT_TASK]                          ; TOP OF WINDOW STACK
        movzx   ecx, word [WIN_STACK + ebx * 2]
        mov     edx, [TASK_COUNT]
        cmp     ecx, edx
        jne     .finish
        cmp     [KEY_COUNT], byte 0
        je      .finish
        movzx   eax, byte [KEY_BUFF]
        shl     eax, 8
        push    eax
        dec     byte [KEY_COUNT]
        and     byte [KEY_COUNT], 127
        movzx   ecx, byte [KEY_COUNT]
        add     ecx, 2
        mov     eax, KEY_BUFF + 1
        mov     ebx, KEY_BUFF
        call    memmove
        pop     eax
;--------------------------------------
align 4
.ret_eax:
        mov     [esp + 32], eax
        ret
;--------------------------------------
align 4
.finish:
; test hotkeys buffer
        mov     ecx, hotkey_buffer
;--------------------------------------
align 4
@@:
        cmp     [ecx], ebx
        jz      .found
        add     ecx, 8
        cmp     ecx, hotkey_buffer + 120 * 8
        jb      @b
        ret
;--------------------------------------
align 4
.found:
        mov     ax, [ecx + 6]
        shl     eax, 16
        mov     ah, [ecx + 4]
        mov     al, 2
        and     dword [ecx + 4], 0
        and     dword [ecx], 0
        jmp     .ret_eax
;------------------------------------------------------------------------------
align 4
sys_getbutton:
        mov     ebx, [CURRENT_TASK]                         ; TOP OF WINDOW STACK
        mov     [esp + 32], dword 1
        movzx   ecx, word [WIN_STACK + ebx * 2]
        mov     edx, [TASK_COUNT] ; less than 256 processes
        cmp     ecx, edx
        jne     .exit
        movzx   eax, byte [BTN_COUNT]
        test    eax, eax
        jz      .exit
        mov     eax, [BTN_BUFF]
        and     al, 0xFE                                    ; delete left button bit
        mov     [BTN_COUNT], byte 0
        mov     [esp + 32], eax
;--------------------------------------
align 4
.exit:
        ret
;------------------------------------------------------------------------------
align 4
sys_cpuusage:

;  RETURN:
;
;  +00 dword     process cpu usage
;  +04  word     position in windowing stack
;  +06  word     windowing stack value at current position (cpu nro)
;  +10 12 bytes  name
;  +22 dword     start in mem
;  +26 dword     used mem
;  +30 dword     PID , process idenfification number
;

        cmp     ecx, -1 ; who am I ?
        jne     .no_who_am_i
        mov     ecx, [CURRENT_TASK]
  .no_who_am_i:
        cmp     ecx, max_processes
        ja      .nofillbuf

; +4: word: position of the window of thread in the window stack
        mov     ax, [WIN_STACK + ecx * 2]
        mov     [ebx+4], ax
; +6: word: number of the thread slot, which window has in the window stack
;           position ecx (has no relation to the specific thread)
        mov     ax, [WIN_POS + ecx * 2]
        mov     [ebx+6], ax

        shl     ecx, 5

; +0: dword: memory usage
        mov     eax, [ecx+CURRENT_TASK+TASKDATA.cpu_usage]
        mov     [ebx], eax
; +10: 11 bytes: name of the process
        push    ecx
        lea     eax, [ecx*8+SLOT_BASE+APPDATA.app_name]
        add     ebx, 10
        mov     ecx, 11
        call    memmove
        pop     ecx

; +22: address of the process in memory
; +26: size of used memory - 1
        push    edi
        lea     edi, [ebx+12]
        xor     eax, eax
        mov     edx, 0x100000*16
        cmp     ecx, 1 shl 5
        je      .os_mem
        mov     edx, [SLOT_BASE+ecx*8+APPDATA.mem_size]
        mov     eax, std_application_base_address
.os_mem:
        stosd
        lea     eax, [edx-1]
        stosd

; +30: PID/TID
        mov     eax, [ecx+CURRENT_TASK+TASKDATA.pid]
        stosd

    ; window position and size
        push    esi
        lea     esi, [ecx + window_data + WDATA.box]
        movsd
        movsd
        movsd
        movsd

    ; Process state (+50)
        mov     eax, dword [ecx+CURRENT_TASK+TASKDATA.state]
        stosd

    ; Window client area box
        lea     esi, [ecx*8 + SLOT_BASE + APPDATA.wnd_clientbox]
        movsd
        movsd
        movsd
        movsd

    ; Window state
        mov     al, [ecx+window_data+WDATA.fl_wstate]
        stosb

    ; Event mask (+71)
        mov     EAX, dword [ECX+CURRENT_TASK+TASKDATA.event_mask]
        stosd

    ; Keyboard mode (+75)
        mov     al, byte [ecx*8 + SLOT_BASE + APPDATA.keyboard_mode]
        stosb

        pop     esi
        pop     edi

.nofillbuf:
    ; return number of processes

        mov     eax, [TASK_COUNT]
        mov     [esp+32], eax
        ret

align 4
sys_clock:
        cli
  ; Mikhail Lisovin  xx Jan 2005
  @@:
        mov     al, 10
        out     0x70, al
        in      al, 0x71
        test    al, al
        jns     @f
        mov     esi, 1
        call    delay_ms
        jmp     @b
  @@:
  ; end Lisovin's fix

        xor     al, al        ; seconds
        out     0x70, al
        in      al, 0x71
        movzx   ecx, al
        mov     al, 02        ; minutes
        shl     ecx, 16
        out     0x70, al
        in      al, 0x71
        movzx   edx, al
        mov     al, 04        ; hours
        shl     edx, 8
        out     0x70, al
        in      al, 0x71
        add     ecx, edx
        movzx   edx, al
        add     ecx, edx
        sti
        mov     [esp + 32], ecx
        ret


align 4

sys_date:

        cli
  @@:
        mov     al, 10
        out     0x70, al
        in      al, 0x71
        test    al, al
        jns     @f
        mov     esi, 1
        call    delay_ms
        jmp     @b
  @@:

        mov     ch, 0
        mov     al, 7           ; date
        out     0x70, al
        in      al, 0x71
        mov     cl, al
        mov     al, 8           ; month
        shl     ecx, 16
        out     0x70, al
        in      al, 0x71
        mov     ch, al
        mov     al, 9           ; year
        out     0x70, al
        in      al, 0x71
        mov     cl, al
        sti
        mov     [esp+32], ecx
        ret


; redraw status

sys_redrawstat:
        cmp     ebx, 1
        jne     no_widgets_away
        ; buttons away
        mov     ecx, [CURRENT_TASK]
  sys_newba2:
        mov     edi, [BTN_ADDR]
        cmp     [edi], dword 0  ; empty button list ?
        je      end_of_buttons_away
        movzx   ebx, word [edi]
        inc     ebx
        mov     eax, edi
  sys_newba:
        dec     ebx
        jz      end_of_buttons_away

        add     eax, 0x10
        cmp     cx, [eax]
        jnz     sys_newba

        push    eax ebx ecx
        mov     ecx, ebx
        inc     ecx
        shl     ecx, 4
        mov     ebx, eax
        add     eax, 0x10
        call    memmove
        dec     dword [edi]
        pop     ecx ebx eax

        jmp     sys_newba2

  end_of_buttons_away:

        ret

  no_widgets_away:

        cmp     ebx, 2
        jnz     srl1

        mov     edx, [TASK_BASE]      ; return whole screen draw area for this app
        add     edx, draw_data - CURRENT_TASK
        mov     [edx + RECT.left], 0
        mov     [edx + RECT.top], 0
        mov     eax, [Screen_Max_X]
        mov     [edx + RECT.right], eax
        mov     eax, [Screen_Max_Y]
        mov     [edx + RECT.bottom], eax

  srl1:
        ret

;ok - 100% work
;nt - not tested
;---------------------------------------------------------------------------------------------
;eax
;0 - task switch counter. Ret switch counter in eax. Block. ok.
;1 - change task. Ret nothing. Block. ok.
;2 - performance control
; ebx
; 0 - enable or disable (inversion) PCE flag on CR4 for rdmpc in user mode.
; returned new cr4 in eax. Ret cr4 in eax. Block. ok.
; 1 - is cache enabled. Ret cr0 in eax if enabled else zero in eax. Block. ok.
; 2 - enable cache. Ret 1 in eax. Ret nothing. Block. ok.
; 3 - disable cache. Ret 0 in eax. Ret nothing. Block. ok.
;eax
;3 - rdmsr. Counter in edx. (edx:eax) [esi:edi, edx] => [edx:esi, ecx]. Ret in ebx:eax. Block. ok.
;4 - wrmsr. Counter in edx. (edx:eax) [esi:edi, edx] => [edx:esi, ecx]. Ret in ebx:eax. Block. ok.
;---------------------------------------------------------------------------------------------
iglobal
align 4
sheduler:
        dd      sys_sheduler.00
        dd      change_task
        dd      sys_sheduler.02
        dd      sys_sheduler.03
        dd      sys_sheduler.04
endg
sys_sheduler:
;rewritten by <Lrz>  29.12.2009
        jmp     dword [sheduler+ebx*4]
;.shed_counter:
.00:
        mov     eax, [context_counter]
        mov     [esp+32], eax
        ret

.02:
;.perf_control:
        inc     ebx                     ;before ebx=2, ebx=3
        cmp     ebx, ecx                ;if ecx=3, ebx=3
        jz      cache_disable

        dec     ebx                     ;ebx=2
        cmp     ebx, ecx                ;
        jz      cache_enable            ;if ecx=2 and ebx=2

        dec     ebx                     ;ebx=1
        cmp     ebx, ecx
        jz      is_cache_enabled        ;if ecx=1 and ebx=1

        dec     ebx
        test    ebx, ecx                ;ebx=0 and ecx=0
        jz      modify_pce              ;if ecx=0

        ret

.03:
;.rdmsr_instr:
;now counter in ecx
;(edx:eax) esi:edi => edx:esi
        mov     eax, esi
        mov     ecx, edx
        rdmsr
        mov     [esp+32], eax
        mov     [esp+20], edx           ;ret in ebx?
        ret

.04:
;.wrmsr_instr:
;now counter in ecx
;(edx:eax) esi:edi => edx:esi
        ; Fast Call MSR can't be destroy
        ; Но MSR_AMD_EFER можно изменять, т.к. в этом регистре лиш
        ; включаются/выключаются расширенные возможности
        cmp     edx, MSR_SYSENTER_CS
        je      @f
        cmp     edx, MSR_SYSENTER_ESP
        je      @f
        cmp     edx, MSR_SYSENTER_EIP
        je      @f
        cmp     edx, MSR_AMD_STAR
        je      @f

        mov     eax, esi
        mov     ecx, edx
        wrmsr
        ; mov   [esp + 32], eax
        ; mov   [esp + 20], edx ;ret in ebx?
@@:
        ret

cache_disable:
        mov     eax, cr0
        or      eax, 01100000000000000000000000000000b
        mov     cr0, eax
        wbinvd  ;set MESI
        ret

cache_enable:
        mov     eax, cr0
        and     eax, 10011111111111111111111111111111b
        mov     cr0, eax
        ret

is_cache_enabled:
        mov     eax, cr0
        mov     ebx, eax
        and     eax, 01100000000000000000000000000000b
        jz      cache_disabled
        mov     [esp+32], ebx
cache_disabled:
        mov     dword [esp+32], eax;0
        ret

modify_pce:
        mov     eax, cr4
;       mov ebx,0
;       or  bx,100000000b ;pce
;       xor eax,ebx ;invert pce
        bts     eax, 8;pce=cr4[8]
        mov     cr4, eax
        mov     [esp+32], eax
        ret
;---------------------------------------------------------------------------------------------


; check if pixel is allowed to be drawn

;checkpixel:
;        push    eax edx

;;        mov     edx, [Screen_Max_X] ; screen x size
;;        inc     edx
;;        imul    edx, ebx
;        mov     edx, [d_width_calc_area + ebx*4]
;        add     eax, [_WinMapAddress]
;        mov     dl, [eax+edx]; lea eax, [...]

;        xor     ecx, ecx
;        mov     eax, [CURRENT_TASK]
;        cmp     al, dl
;        setne   cl

;        pop     edx eax
;        ret

iglobal
  cpustring db 'CPU',0
endg

uglobal
background_defined    db    0    ; diamond, 11.04.2006
endg
;-----------------------------------------------------------------------------
align 4
checkmisc:
        cmp     [ctrl_alt_del], 1
        jne     nocpustart

        mov     ebp, cpustring
        call    fs_execute_from_sysdir

        mov     [ctrl_alt_del], 0
;--------------------------------------
align 4
nocpustart:
        cmp     [mouse_active], 1
        jne     mouse_not_active
        mov     [mouse_active], 0

        xor     edi, edi
        mov     ebx, CURRENT_TASK

        mov     ecx, [TASK_COUNT]
        movzx   eax, word [WIN_POS + ecx*2]     ; active window
        shl     eax, 8
        push    eax

        movzx   eax, word [MOUSE_X]
        movzx   edx, word [MOUSE_Y]
;--------------------------------------
align 4
.set_mouse_event:
        add     edi, 256
        add     ebx, 32
        test    [ebx+TASKDATA.event_mask], 0x80000000
        jz      .pos_filter

        cmp     edi, [esp]                      ; skip if filtration active
        jne     .skip
;--------------------------------------
align 4
.pos_filter:
        test    [ebx+TASKDATA.event_mask], 0x40000000
        jz      .set

        mov     esi, [ebx-twdw+WDATA.box.left]
        cmp     eax, esi
        jb      .skip
        add     esi, [ebx-twdw+WDATA.box.width]
        cmp     eax, esi
        ja      .skip

        mov     esi, [ebx-twdw+WDATA.box.top]
        cmp     edx, esi
        jb      .skip
        add     esi, [ebx-twdw+WDATA.box.height]
        cmp     edx, esi
        ja      .skip
;--------------------------------------
align 4
.set:
        or      [edi+SLOT_BASE+APPDATA.event_mask], 100000b  ; set event 6
;--------------------------------------
align 4
.skip:
        loop    .set_mouse_event

        pop     eax
;--------------------------------------
align 4
mouse_not_active:
        cmp     byte[REDRAW_BACKGROUND], 0         ; background update ?
        jz      nobackgr

        cmp     [background_defined], 0
        jz      nobackgr
;--------------------------------------
align 4
backgr:
        mov     eax, [draw_data+32 + RECT.left]
        shl     eax, 16
        add     eax, [draw_data+32 + RECT.right]
        mov     [BG_Rect_X_left_right], eax ; [left]*65536 + [right]

        mov     eax, [draw_data+32 + RECT.top]
        shl     eax, 16
        add     eax, [draw_data+32 + RECT.bottom]
        mov     [BG_Rect_Y_top_bottom], eax ; [top]*65536 + [bottom]

        call    drawbackground
;        DEBUGF  1, "K : drawbackground\n"
;        DEBUGF  1, "K : backg x %x\n",[BG_Rect_X_left_right]
;        DEBUGF  1, "K : backg y %x\n",[BG_Rect_Y_top_bottom]
;--------- set event 5 start ----------
        push    ecx edi
        xor     edi, edi
        mov     ecx, [TASK_COUNT]
;--------------------------------------
align 4
set_bgr_event:
        add     edi, 256
        mov     eax, [BG_Rect_X_left_right]
        mov     edx, [BG_Rect_Y_top_bottom]
        cmp     [edi+SLOT_BASE+APPDATA.draw_bgr_x], 0
        jz      .set
.join:
        cmp     word [edi+SLOT_BASE+APPDATA.draw_bgr_x], ax
        jae     @f
        mov     word [edi+SLOT_BASE+APPDATA.draw_bgr_x], ax
@@:
        shr     eax, 16
        cmp     word [edi+SLOT_BASE+APPDATA.draw_bgr_x+2], ax
        jbe     @f
        mov     word [edi+SLOT_BASE+APPDATA.draw_bgr_x+2], ax
@@:
        cmp     word [edi+SLOT_BASE+APPDATA.draw_bgr_y], dx
        jae     @f
        mov     word [edi+SLOT_BASE+APPDATA.draw_bgr_y], dx
@@:
        shr     edx, 16
        cmp     word [edi+SLOT_BASE+APPDATA.draw_bgr_y+2], dx
        jbe     @f
        mov     word [edi+SLOT_BASE+APPDATA.draw_bgr_y+2], dx
@@:
        jmp     .common
.set:
        mov     [edi+SLOT_BASE+APPDATA.draw_bgr_x], eax
        mov     [edi+SLOT_BASE+APPDATA.draw_bgr_y], edx
.common:
        or      [edi+SLOT_BASE+APPDATA.event_mask], 10000b  ; set event 5
        loop    set_bgr_event
        pop     edi ecx
;--------- set event 5 stop -----------
        dec     byte[REDRAW_BACKGROUND]    ; got new update request?
        jnz     backgr

        xor     eax, eax
        mov     [draw_data+32 + RECT.left], eax
        mov     [draw_data+32 + RECT.top], eax
        mov     [draw_data+32 + RECT.right], eax
        mov     [draw_data+32 + RECT.bottom], eax
;--------------------------------------
align 4
nobackgr:
; system shutdown request
        cmp     [SYS_SHUTDOWN], byte 0
        je      noshutdown

        mov     edx, [shutdown_processes]

        cmp     [SYS_SHUTDOWN], dl
        jne     noshutdown

        lea     ecx, [edx-1]
        mov     edx, OS_BASE+0x3040
        jecxz   no_mark_system_shutdown
;--------------------------------------
align 4
markz:
        push    ecx edx
        cmp     [edx+TASKDATA.state], 9
        jz      .nokill
        lea     edx, [(edx-(CURRENT_TASK and 1FFFFFFFh))*8+SLOT_BASE]
        cmp     [edx+APPDATA.dir_table], sys_pgdir - OS_BASE
        jz      .nokill
        call    request_terminate
        jmp     .common
.nokill:
        dec     byte [SYS_SHUTDOWN]
        xor     eax, eax
.common:
        pop     edx ecx
        test    eax, eax
        jz      @f
        mov     [edx+TASKDATA.state], byte 3
@@:
        add     edx, 0x20
        loop    markz
        call    wakeup_osloop
;--------------------------------------
align 4
@@:
no_mark_system_shutdown:
        dec     byte [SYS_SHUTDOWN]
        je      system_shutdown
;--------------------------------------
align 4
noshutdown:
        mov     eax, [TASK_COUNT]           ; termination
        mov     ebx, TASK_DATA+TASKDATA.state
        mov     esi, 1
;--------------------------------------
align 4
newct:
        mov     cl, [ebx]
        cmp     cl, byte 3
        jz      .terminate

        cmp     cl, byte 4
        jnz     .noterminate
.terminate:
        pushad
        mov     ecx, eax
        shl     ecx, 8
        add     ecx, SLOT_BASE
        call    restore_default_cursor_before_killing
        popad

        pushad
        call    terminate
        popad
        cmp     byte [SYS_SHUTDOWN], 0
        jz      .noterminate
        dec     byte [SYS_SHUTDOWN]
        je      system_shutdown

.noterminate:
        add     ebx, 0x20
        inc     esi
        dec     eax
        jnz     newct
        ret
;-----------------------------------------------------------------------------
align 4
redrawscreen:
; eax , if process window_data base is eax, do not set flag/limits

        pushad
        push    eax

;;;         mov   ebx,2
;;;         call  delay_hs

         ;mov   ecx,0               ; redraw flags for apps
        xor     ecx, ecx
;--------------------------------------
align 4
newdw2:
        inc     ecx
        push    ecx

        mov     eax, ecx
        shl     eax, 5
        add     eax, window_data

        cmp     eax, [esp+4]
        je      not_this_task
                                   ; check if window in redraw area
        mov     edi, eax

        cmp     ecx, 1             ; limit for background
        jz      bgli

        mov     eax, [edi + WDATA.box.left]
        mov     ebx, [edi + WDATA.box.top]

        mov     ecx, [draw_limits.bottom] ; ecx = area y end     ebx = window y start
        cmp     ecx, ebx
        jb      ricino

        mov     ecx, [draw_limits.right] ; ecx = area x end     eax = window x start
        cmp     ecx, eax
        jb      ricino

        mov     eax, [edi + WDATA.box.left]
        mov     ebx, [edi + WDATA.box.top]
        mov     ecx, [edi + WDATA.box.width]
        mov     edx, [edi + WDATA.box.height]
        add     ecx, eax
        add     edx, ebx

        mov     eax, [draw_limits.top]  ; eax = area y start     edx = window y end
        cmp     edx, eax
        jb      ricino

        mov     eax, [draw_limits.left]  ; eax = area x start     ecx = window x end
        cmp     ecx, eax
        jb      ricino
;--------------------------------------
align 4
bgli:
        cmp     dword[esp], 1
        jnz     .az

        cmp     byte[REDRAW_BACKGROUND], 0
        jz      .az

        mov     dl, 0
        lea     eax, [edi+draw_data-window_data]
        mov     ebx, [draw_limits.left]
        cmp     ebx, [eax+RECT.left]
        jae     @f

        mov     [eax+RECT.left], ebx
        mov     dl, 1
;--------------------------------------
align 4
@@:
        mov     ebx, [draw_limits.top]
        cmp     ebx, [eax+RECT.top]
        jae     @f

        mov     [eax+RECT.top], ebx
        mov     dl, 1
;--------------------------------------
align 4
@@:
        mov     ebx, [draw_limits.right]
        cmp     ebx, [eax+RECT.right]
        jbe     @f

        mov     [eax+RECT.right], ebx
        mov     dl, 1
;--------------------------------------
align 4
@@:
        mov     ebx, [draw_limits.bottom]
        cmp     ebx, [eax+RECT.bottom]
        jbe     @f

        mov     [eax+RECT.bottom], ebx
        mov     dl, 1
;--------------------------------------
align 4
@@:
        add     byte[REDRAW_BACKGROUND], dl
        call    wakeup_osloop
        jmp     newdw8
;--------------------------------------
align 4
.az:
        mov     eax, edi
        add     eax, draw_data-window_data

        mov     ebx, [draw_limits.left]        ; set limits
        mov     [eax + RECT.left], ebx
        mov     ebx, [draw_limits.top]
        mov     [eax + RECT.top], ebx
        mov     ebx, [draw_limits.right]
        mov     [eax + RECT.right], ebx
        mov     ebx, [draw_limits.bottom]
        mov     [eax + RECT.bottom], ebx

        sub     eax, draw_data-window_data

        cmp     dword [esp], 1
        jne     nobgrd
        inc     byte[REDRAW_BACKGROUND]
        call    wakeup_osloop
;--------------------------------------
align 4
newdw8:
nobgrd:
;--------------------------------------
        push    eax  edi ebp
        mov     edi, [esp+12]
        cmp     edi, 1
        je      .found

        mov     eax, [draw_limits.left]
        mov     ebx, [draw_limits.top]
        mov     ecx, [draw_limits.right]
        sub     ecx, eax
        test    ecx, ecx
        jz      .not_found

        mov     edx, [draw_limits.bottom]
        sub     edx, ebx
        test    edx, edx
        jz      .not_found

; eax - x, ebx - y
; ecx - size x, edx - size y
        add     ebx, edx
;--------------------------------------
align 4
.start_y:
        push    ecx
;--------------------------------------
align 4
.start_x:
        add     eax, ecx
        mov     ebp, [d_width_calc_area + ebx*4]
        add     ebp, [_WinMapAddress]
        movzx   ebp, byte[eax+ebp] ; get value for current point
        cmp     ebp, edi
        jne     @f

        pop     ecx
        jmp     .found
;--------------------------------------
align 4
@@:
        sub     eax, ecx

        dec     ecx
        jnz     .start_x

        pop     ecx
        dec     ebx
        dec     edx
        jnz     .start_y
;--------------------------------------
align 4
.not_found:
        pop     ebp edi eax
        jmp     ricino
;--------------------------------------
align 4
.found:
        pop     ebp edi eax

        mov     [eax + WDATA.fl_redraw], byte 1  ; mark as redraw
;--------------------------------------
align 4
ricino:
not_this_task:
        pop     ecx

        cmp     ecx, [TASK_COUNT]
        jle     newdw2

        pop     eax
        popad
        ret
;-----------------------------------------------------------------------------
align 4
calculatebackground:   ; background
        mov     edi, [_WinMapAddress]              ; set os to use all pixels
        mov     eax, 0x01010101
        mov     ecx, [_WinMapSize]
        shr     ecx, 2
        rep stosd

        mov     byte[REDRAW_BACKGROUND], 0            ; do not draw background!
        ret
;-----------------------------------------------------------------------------
uglobal
  imax    dd 0x0
endg
;-----------------------------------------------------------------------------
align 4
delay_ms:     ; delay in 1/1000 sec
        push    eax
        push    ecx

        mov     ecx, esi
        ; <CPU clock fix by Sergey Kuzmin aka Wildwest>
        imul    ecx, 33941
        shr     ecx, 9
        ; </CPU clock fix>

        in      al, 0x61
        and     al, 0x10
        mov     ah, al
        cld
;--------------------------------------
align 4
cnt1:
        in      al, 0x61
        and     al, 0x10
        cmp     al, ah
        jz      cnt1

        mov     ah, al
        loop    cnt1

        pop     ecx
        pop     eax
        ret
;-----------------------------------------------------------------------------
align 4
set_app_param:
        mov     edi, [TASK_BASE]
        mov     eax, ebx
        btr     eax, 3                           ; move MOUSE_FILTRATION
        mov     ebx, [current_slot]              ; bit into event_filter
        setc    byte [ebx+APPDATA.event_filter]
        xchg    eax, [edi + TASKDATA.event_mask] ; set new event mask
        mov     [esp+32], eax                    ; return old mask value
        ret
;-----------------------------------------------------------------------------

; this is for syscall
proc delay_hs_unprotected
        call    unprotect_from_terminate
        call    delay_hs
        call    protect_from_terminate
        ret
endp

if 1
align 4
delay_hs:     ; delay in 1/100 secs
; ebx = delay time

        pushad
        push    ebx
        xor     esi, esi
        mov     ecx, MANUAL_DESTROY
        call    create_event
        test    eax, eax
        jz      .done

        mov     ebx, edx
        mov     ecx, [esp]
        push    edx
        push    eax
        call    wait_event_timeout
        pop     eax
        pop     ebx
        call    destroy_event
.done:
        add     esp, 4
        popad
        ret

else

align 4
delay_hs:     ; delay in 1/100 secs
; ebx = delay time
        push    ecx
        push    edx

        mov     edx, [timer_ticks]
;--------------------------------------
align 4
newtic:
        mov     ecx, [timer_ticks]
        sub     ecx, edx
        cmp     ecx, ebx
        jae     zerodelay

        call    change_task

        jmp     newtic
;--------------------------------------
align 4
zerodelay:
        pop     edx
        pop     ecx
        ret
end if

;-----------------------------------------------------------------------------
align 16        ;very often call this subrutine
memmove:       ; memory move in bytes
; eax = from
; ebx = to
; ecx = no of bytes
        test    ecx, ecx
        jle     .ret

        push    esi edi ecx

        mov     edi, ebx
        mov     esi, eax

        test    ecx, not 11b
        jz      @f

        push    ecx
        shr     ecx, 2
        rep movsd
        pop     ecx
        and     ecx, 11b
        jz      .finish
;--------------------------------------
align 4
@@:
        rep movsb
;--------------------------------------
align 4
.finish:
        pop     ecx edi esi
;--------------------------------------
align 4
.ret:
        ret
;-----------------------------------------------------------------------------
; <diamond> Sysfunction 34, read_floppy_file, is obsolete. Use 58 or 70 function instead.
;align 4
;
;read_floppy_file:
;
;; as input
;;
;; eax pointer to file
;; ebx file lenght
;; ecx start 512 byte block number
;; edx number of blocks to read
;; esi pointer to return/work area (atleast 20 000 bytes)
;;
;;
;; on return
;;
;; eax = 0 command succesful
;;       1 no fd base and/or partition defined
;;       2 yet unsupported FS
;;       3 unknown FS
;;       4 partition not defined at hd
;;       5 file not found
;; ebx = size of file
;
;     mov   edi,[TASK_BASE]
;     add   edi,0x10
;     add   esi,[edi]
;     add   eax,[edi]
;
;     pushad
;     mov  edi,esi
;     add  edi,1024
;     mov  esi,0x100000+19*512
;     sub  ecx,1
;     shl  ecx,9
;     add  esi,ecx
;     shl  edx,9
;     mov  ecx,edx
;     cld
;     rep  movsb
;     popad
;
;     mov   [esp+36],eax
;     mov   [esp+24],ebx
;     ret



align 4
set_io_access_rights:
        push    edi eax
        mov     edi, tss._io_map_0
;     mov   ecx,eax
;     and   ecx,7    ; offset in byte
;     shr   eax,3    ; number of byte
;     add   edi,eax
;     mov   ebx,1
;     shl   ebx,cl
        test    ebp, ebp
;     cmp   ebp,0                ; enable access - ebp = 0
        jnz     .siar1
;     not   ebx
;     and   [edi],byte bl
        btr     [edi], eax
        pop     eax edi
        ret
.siar1:
        bts     [edi], eax
  ;  or    [edi],byte bl        ; disable access - ebp = 1
        pop     eax edi
        ret
;reserve/free group of ports
;  * eax = 46 - number function
;  * ebx = 0 - reserve, 1 - free
;  * ecx = number start arrea of ports
;  * edx = number end arrea of ports (include last number of port)
;Return value:
;  * eax = 0 - succesful
;  * eax = 1 - error
;  * The system has reserve this ports:
;    0..0x2d, 0x30..0x4d, 0x50..0xdf, 0xe5..0xff (include last number of port).
;destroys eax,ebx, ebp
r_f_port_area:

        test    ebx, ebx
        jnz     free_port_area
;     je    r_port_area
;     jmp   free_port_area

;   r_port_area:

;     pushad

        cmp     ecx, edx      ; beginning > end ?
        ja      rpal1
        cmp     edx, 65536
        jae     rpal1
        mov     eax, [RESERVED_PORTS]
        test    eax, eax      ; no reserved areas ?
        je      rpal2
        cmp     eax, 255      ; max reserved
        jae     rpal1
 rpal3:
        mov     ebx, eax
        shl     ebx, 4
        add     ebx, RESERVED_PORTS
        cmp     ecx, [ebx+8]
        ja      rpal4
        cmp     edx, [ebx+4]
        jae     rpal1
;     jb    rpal4
;     jmp   rpal1
 rpal4:
        dec     eax
        jnz     rpal3
        jmp     rpal2
   rpal1:
;     popad
;     mov   eax,1
        xor     eax, eax
        inc     eax
        ret
   rpal2:
;     popad
     ; enable port access at port IO map
        cli
        pushad                        ; start enable io map

        cmp     edx, 65536;16384
        jae     no_unmask_io; jge
        mov     eax, ecx
;       push    ebp
        xor     ebp, ebp               ; enable - eax = port
new_port_access:
;     pushad
        call    set_io_access_rights
;     popad
        inc     eax
        cmp     eax, edx
        jbe     new_port_access
;       pop     ebp
no_unmask_io:
        popad                         ; end enable io map
        sti

        mov     eax, [RESERVED_PORTS]
        add     eax, 1
        mov     [RESERVED_PORTS], eax
        shl     eax, 4
        add     eax, RESERVED_PORTS
        mov     ebx, [TASK_BASE]
        mov     ebx, [ebx+TASKDATA.pid]
        mov     [eax], ebx
        mov     [eax+4], ecx
        mov     [eax+8], edx

        xor     eax, eax
        ret

free_port_area:

;     pushad
        mov     eax, [RESERVED_PORTS]; no reserved areas ?
        test    eax, eax
        jz      frpal2
        mov     ebx, [TASK_BASE]
        mov     ebx, [ebx+TASKDATA.pid]
   frpal3:
        mov     edi, eax
        shl     edi, 4
        add     edi, RESERVED_PORTS
        cmp     ebx, [edi]
        jne     frpal4
        cmp     ecx, [edi+4]
        jne     frpal4
        cmp     edx, [edi+8]
        jne     frpal4
        jmp     frpal1
   frpal4:
        dec     eax
        jnz     frpal3
   frpal2:
;     popad
        inc     eax
        ret
   frpal1:
        push    ecx
        mov     ecx, 256
        sub     ecx, eax
        shl     ecx, 4
        mov     esi, edi
        add     esi, 16
        cld
        rep movsb

        dec     dword [RESERVED_PORTS]
;popad
;disable port access at port IO map

;     pushad                        ; start disable io map
        pop     eax     ;start port
        cmp     edx, 65536;16384
        jge     no_mask_io

;     mov   eax,ecx
        xor     ebp, ebp
        inc     ebp
new_port_access_disable:
;     pushad
;     mov   ebp,1                  ; disable - eax = port
        call    set_io_access_rights
;     popad
        inc     eax
        cmp     eax, edx
        jbe     new_port_access_disable
no_mask_io:
;     popad                         ; end disable io map
        xor     eax, eax
        ret
;-----------------------------------------------------------------------------
align 4
drawbackground:
dbrv20:
        cmp     [BgrDrawMode], dword 1
        jne     bgrstr
        call    vesa20_drawbackground_tiled
;        call    [draw_pointer]
        call    __sys_draw_pointer
        ret
;--------------------------------------
align 4
bgrstr:
        call    vesa20_drawbackground_stretch
;        call    [draw_pointer]
        call    __sys_draw_pointer
        ret
;-----------------------------------------------------------------------------
align 4
syscall_putimage:                       ; PutImage
sys_putimage:
        test    ecx, 0x80008000
        jnz     .exit
        test    ecx, 0x0000FFFF
        jz      .exit
        test    ecx, 0xFFFF0000
        jnz     @f
;--------------------------------------
align 4
.exit:
        ret
;--------------------------------------
align 4
@@:
        mov     edi, [current_slot]
        add     dx, word[edi+APPDATA.wnd_clientbox.top]
        rol     edx, 16
        add     dx, word[edi+APPDATA.wnd_clientbox.left]
        rol     edx, 16
;--------------------------------------
align 4
.forced:
        push    ebp esi 0
        mov     ebp, putimage_get24bpp
        mov     esi, putimage_init24bpp
;--------------------------------------
align 4
sys_putimage_bpp:
        call    vesa20_putimage
        pop     ebp esi ebp
        ret
;        jmp     [draw_pointer]
;-----------------------------------------------------------------------------
align 4
sys_putimage_palette:
; ebx = pointer to image
; ecx = [xsize]*65536 + [ysize]
; edx = [xstart]*65536 + [ystart]
; esi = number of bits per pixel, must be 8, 24 or 32
; edi = pointer to palette
; ebp = row delta
        mov     eax, [CURRENT_TASK]
        shl     eax, 8
        add     dx, word [eax+SLOT_BASE+APPDATA.wnd_clientbox.top]
        rol     edx, 16
        add     dx, word [eax+SLOT_BASE+APPDATA.wnd_clientbox.left]
        rol     edx, 16
;--------------------------------------
align 4
.forced:
        cmp     esi, 1
        jnz     @f
        push    edi
        mov     eax, [edi+4]
        sub     eax, [edi]
        push    eax
        push    dword [edi]
        push    0ffffff80h
        mov     edi, esp
        call    put_mono_image
        add     esp, 12
        pop     edi
        ret
;--------------------------------------
align 4
@@:
        cmp     esi, 2
        jnz     @f
        push    edi
        push    0ffffff80h
        mov     edi, esp
        call    put_2bit_image
        pop     eax
        pop     edi
        ret
;--------------------------------------
align 4
@@:
        cmp     esi, 4
        jnz     @f
        push    edi
        push    0ffffff80h
        mov     edi, esp
        call    put_4bit_image
        pop     eax
        pop     edi
        ret
;--------------------------------------
align 4
@@:
        push    ebp esi ebp
        cmp     esi, 8
        jnz     @f
        mov     ebp, putimage_get8bpp
        mov     esi, putimage_init8bpp
        jmp     sys_putimage_bpp
;--------------------------------------
align 4
@@:
        cmp     esi, 9
        jnz     @f
        mov     ebp, putimage_get9bpp
        mov     esi, putimage_init9bpp
        jmp     sys_putimage_bpp
;--------------------------------------
align 4
@@:
        cmp     esi, 15
        jnz     @f
        mov     ebp, putimage_get15bpp
        mov     esi, putimage_init15bpp
        jmp     sys_putimage_bpp
;--------------------------------------
align 4
@@:
        cmp     esi, 16
        jnz     @f
        mov     ebp, putimage_get16bpp
        mov     esi, putimage_init16bpp
        jmp     sys_putimage_bpp
;--------------------------------------
align 4
@@:
        cmp     esi, 24
        jnz     @f
        mov     ebp, putimage_get24bpp
        mov     esi, putimage_init24bpp
        jmp     sys_putimage_bpp
;--------------------------------------
align 4
@@:
        cmp     esi, 32
        jnz     @f
        mov     ebp, putimage_get32bpp
        mov     esi, putimage_init32bpp
        jmp     sys_putimage_bpp
;--------------------------------------
align 4
@@:
        pop     ebp esi ebp
        ret
;-----------------------------------------------------------------------------
align 4
put_mono_image:
        push    ebp esi ebp
        mov     ebp, putimage_get1bpp
        mov     esi, putimage_init1bpp
        jmp     sys_putimage_bpp
;-----------------------------------------------------------------------------
align 4
put_2bit_image:
        push    ebp esi ebp
        mov     ebp, putimage_get2bpp
        mov     esi, putimage_init2bpp
        jmp     sys_putimage_bpp
;-----------------------------------------------------------------------------
align 4
put_4bit_image:
        push    ebp esi ebp
        mov     ebp, putimage_get4bpp
        mov     esi, putimage_init4bpp
        jmp     sys_putimage_bpp
;-----------------------------------------------------------------------------
align 4
putimage_init24bpp:
        lea     eax, [eax*3]
putimage_init8bpp:
putimage_init9bpp:
        ret
;-----------------------------------------------------------------------------
align 16
putimage_get24bpp:
        movzx   eax, byte [esi+2]
        shl     eax, 16
        mov     ax, [esi]
        add     esi, 3
        ret     4
;-----------------------------------------------------------------------------
align 16
putimage_get8bpp:
        movzx   eax, byte [esi]
        push    edx
        mov     edx, [esp+8]
        mov     eax, [edx+eax*4]
        pop     edx
        inc     esi
        ret     4
;-----------------------------------------------------------------------------
align 16
putimage_get9bpp:
        lodsb
        mov     ah, al
        shl     eax, 8
        mov     al, ah
        ret     4
;-----------------------------------------------------------------------------
align 4
putimage_init1bpp:
        add     eax, ecx
        push    ecx
        add     eax, 7
        add     ecx, 7
        shr     eax, 3
        shr     ecx, 3
        sub     eax, ecx
        pop     ecx
        ret
;-----------------------------------------------------------------------------
align 16
putimage_get1bpp:
        push    edx
        mov     edx, [esp+8]
        mov     al, [edx]
        add     al, al
        jnz     @f
        lodsb
        adc     al, al
@@:
        mov     [edx], al
        sbb     eax, eax
        and     eax, [edx+8]
        add     eax, [edx+4]
        pop     edx
        ret     4
;-----------------------------------------------------------------------------
align 4
putimage_init2bpp:
        add     eax, ecx
        push    ecx
        add     ecx, 3
        add     eax, 3
        shr     ecx, 2
        shr     eax, 2
        sub     eax, ecx
        pop     ecx
        ret
;-----------------------------------------------------------------------------
align 16
putimage_get2bpp:
        push    edx
        mov     edx, [esp+8]
        mov     al, [edx]
        mov     ah, al
        shr     al, 6
        shl     ah, 2
        jnz     .nonewbyte
        lodsb
        mov     ah, al
        shr     al, 6
        shl     ah, 2
        add     ah, 1
.nonewbyte:
        mov     [edx], ah
        mov     edx, [edx+4]
        movzx   eax, al
        mov     eax, [edx+eax*4]
        pop     edx
        ret     4
;-----------------------------------------------------------------------------
align 4
putimage_init4bpp:
        add     eax, ecx
        push    ecx
        add     ecx, 1
        add     eax, 1
        shr     ecx, 1
        shr     eax, 1
        sub     eax, ecx
        pop     ecx
        ret
;-----------------------------------------------------------------------------
align 16
putimage_get4bpp:
        push    edx
        mov     edx, [esp+8]
        add     byte [edx], 80h
        jc      @f
        movzx   eax, byte [edx+1]
        mov     edx, [edx+4]
        and     eax, 0x0F
        mov     eax, [edx+eax*4]
        pop     edx
        ret     4
@@:
        movzx   eax, byte [esi]
        add     esi, 1
        mov     [edx+1], al
        shr     eax, 4
        mov     edx, [edx+4]
        mov     eax, [edx+eax*4]
        pop     edx
        ret     4
;-----------------------------------------------------------------------------
align 4
putimage_init32bpp:
        shl     eax, 2
        ret
;-----------------------------------------------------------------------------
align 16
putimage_get32bpp:
        lodsd
        ret     4
;-----------------------------------------------------------------------------
align 4
putimage_init15bpp:
putimage_init16bpp:
        add     eax, eax
        ret
;-----------------------------------------------------------------------------
align 16
putimage_get15bpp:
; 0RRRRRGGGGGBBBBB -> 00000000RRRRR000GGGGG000BBBBB000
        push    ecx edx
        movzx   eax, word [esi]
        add     esi, 2
        mov     ecx, eax
        mov     edx, eax
        and     eax, 0x1F
        and     ecx, 0x1F shl 5
        and     edx, 0x1F shl 10
        shl     eax, 3
        shl     ecx, 6
        shl     edx, 9
        or      eax, ecx
        or      eax, edx
        pop     edx ecx
        ret     4
;-----------------------------------------------------------------------------
align 16
putimage_get16bpp:
; RRRRRGGGGGGBBBBB -> 00000000RRRRR000GGGGGG00BBBBB000
        push    ecx edx
        movzx   eax, word [esi]
        add     esi, 2
        mov     ecx, eax
        mov     edx, eax
        and     eax, 0x1F
        and     ecx, 0x3F shl 5
        and     edx, 0x1F shl 11
        shl     eax, 3
        shl     ecx, 5
        shl     edx, 8
        or      eax, ecx
        or      eax, edx
        pop     edx ecx
        ret     4
;-----------------------------------------------------------------------------
;align 4
; eax x beginning
; ebx y beginning
; ecx x end
        ; edx y end
; edi color
;__sys_drawbar:
;        mov     esi, [current_slot]
;        add     eax, [esi+APPDATA.wnd_clientbox.left]
;        add     ecx, [esi+APPDATA.wnd_clientbox.left]
;        add     ebx, [esi+APPDATA.wnd_clientbox.top]
;        add     edx, [esi+APPDATA.wnd_clientbox.top]
;--------------------------------------
;align 4
;.forced:
;        call    vesa20_drawbar
;        call    [draw_pointer]
;        ret
;-----------------------------------------------------------------------------
align 4
kb_read:

        push    ecx edx

        mov     ecx, 0x1ffff; last 0xffff, new value in view of fast CPU's
      kr_loop:
        in      al, 0x64
        test    al, 1
        jnz     kr_ready
        loop    kr_loop
        mov     ah, 1
        jmp     kr_exit
      kr_ready:
        push    ecx
        mov     ecx, 32
      kr_delay:
        loop    kr_delay
        pop     ecx
        in      al, 0x60
        xor     ah, ah
      kr_exit:

        pop     edx ecx

        ret
;-----------------------------------------------------------------------------
align 4
kb_write:

        push    ecx edx

        mov     dl, al
;        mov     ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
;      kw_loop1:
;        in      al,0x64
;        test    al,0x20
;        jz      kw_ok1
;        loop    kw_loop1
;        mov     ah,1
;        jmp     kw_exit
;      kw_ok1:
        in      al, 0x60
        mov     ecx, 0x1ffff; last 0xffff, new value in view of fast CPU's
      kw_loop:
        in      al, 0x64
        test    al, 2
        jz      kw_ok
        loop    kw_loop
        mov     ah, 1
        jmp     kw_exit
      kw_ok:
        mov     al, dl
        out     0x60, al
        mov     ecx, 0x1ffff; last 0xffff, new value in view of fast CPU's
      kw_loop3:
        in      al, 0x64
        test    al, 2
        jz      kw_ok3
        loop    kw_loop3
        mov     ah, 1
        jmp     kw_exit
      kw_ok3:
        mov     ah, 8
      kw_loop4:
        mov     ecx, 0x1ffff; last 0xffff, new value in view of fast CPU's
      kw_loop5:
        in      al, 0x64
        test    al, 1
        jnz     kw_ok4
        loop    kw_loop5
        dec     ah
        jnz     kw_loop4
      kw_ok4:
        xor     ah, ah
      kw_exit:

        pop     edx ecx

        ret
;-----------------------------------------------------------------------------
align 4
kb_cmd:

        mov     ecx, 0x1ffff; last 0xffff, new value in view of fast CPU's
      c_wait:
        in      al, 0x64
        test    al, 2
        jz      c_send
        loop    c_wait
        jmp     c_error
      c_send:
        mov     al, bl
        out     0x64, al
        mov     ecx, 0x1ffff; last 0xffff, new value in view of fast CPU's
      c_accept:
        in      al, 0x64
        test    al, 2
        jz      c_ok
        loop    c_accept
      c_error:
        mov     ah, 1
        jmp     c_exit
      c_ok:
        xor     ah, ah
      c_exit:
        ret


setmouse:  ; set mousepicture -pointer
           ; ps2 mouse enable

;        mov     [MOUSE_PICTURE], dword mousepointer

        cli

        ret

if used _rdtsc
_rdtsc:
        bt      [cpu_caps], CAPS_TSC
        jnc     ret_rdtsc
        rdtsc
        ret
   ret_rdtsc:
        mov     edx, 0xffffffff
        mov     eax, 0xffffffff
        ret
end if

sys_msg_board_str:

        pushad
   @@:
        cmp     [esi], byte 0
        je      @f
        mov     ebx, 1
        movzx   ecx, byte [esi]
        call    sys_msg_board
        inc     esi
        jmp     @b
   @@:
        popad
        ret

sys_msg_board_byte:
; in: al = byte to display
; out: nothing
; destroys: nothing
        pushad
        mov     ecx, 2
        shl     eax, 24
        jmp     @f

sys_msg_board_word:
; in: ax = word to display
; out: nothing
; destroys: nothing
        pushad
        mov     ecx, 4
        shl     eax, 16
        jmp     @f

sys_msg_board_dword:
; in: eax = dword to display
; out: nothing
; destroys: nothing
        pushad
        mov     ecx, 8
@@:
        push    ecx
        rol     eax, 4
        push    eax
        and     al, 0xF
        cmp     al, 10
        sbb     al, 69h
        das
        mov     cl, al
        xor     ebx, ebx
        inc     ebx
        call    sys_msg_board
        pop     eax
        pop     ecx
        loop    @b
        popad
        ret

msg_board_data_size = 65536 ; Must be power of two

uglobal
  msg_board_data  rb msg_board_data_size
  msg_board_count dd 0x0
endg

sys_msg_board:

; ebx=1 : write :  bl byte to write
; ebx=2 :  read :  ebx=0 -> no data, ebx=1 -> data in al

        push    eax ebx                 ; Save eax and ebx, since we're restoring their order required.
        mov     eax, ebx
        mov     ebx, ecx

        mov     ecx, [msg_board_count]
        cmp     eax, 1
        jne     .smbl1

if defined debug_com_base

        push    dx ax

       @@:                              ; Wait for empty transmit register  (yes, this slows down system..)
        mov     dx, debug_com_base+5
        in      al, dx
        test    al, 1 shl 5
        jz      @r

        mov     dx, debug_com_base      ; Output the byte
        mov     al, bl
        out     dx, al

        pop     ax dx

end if

        mov     [msg_board_data+ecx], bl
; // if debug_direct_print == 1
        cmp     byte [debug_direct_print], 1
        jnz     @f
        pusha
iglobal
msg_board_pos   dd      (42*6)*65536+10 ; for printing debug output on the screen
endg
        lea     edx, [msg_board_data+ecx]
        mov     ecx, 0x40FFFFFF
        mov     ebx, [msg_board_pos]
        mov     edi, 1
        mov     esi, 1
        call    dtext
        popa
        add     word [msg_board_pos+2], 6
        cmp     bl, 10
        jnz     @f
        mov     word [msg_board_pos+2], (42*6)
        add     word [msg_board_pos], 10
        mov     ax, word [Screen_Max_Y]
        cmp     word [msg_board_pos], ax
        jbe     @f
        mov     word [msg_board_pos], 10
@@:
; // end if

if 0
        pusha
        mov     al, bl
        mov     edx, 402h
        out     dx, al
        popa
end if
        inc     ecx
        and     ecx, msg_board_data_size - 1
        mov     [msg_board_count], ecx

        pop     ebx eax
        ret
.smbl1:
        cmp     eax, 2
        jne     .smbl2
        test    ecx, ecx
        jz      .smbl21

        add     esp, 8                  ; Returning data in ebx and eax, so no need to restore them.
        mov     eax, msg_board_data+1
        mov     ebx, msg_board_data
        movzx   edx, byte [ebx]
        call    memmove
        dec     [msg_board_count]
        mov     [esp + 32], edx ;eax
        mov     [esp + 20], dword 1
        ret
.smbl21:
        mov     [esp+32], ecx
        mov     [esp+20], ecx
.smbl2:
        pop     ebx eax
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; 66 sys function.                                                ;;
;; in eax=66,ebx in [0..5],ecx,edx                                 ;;
;; out eax                                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
iglobal
align 4
f66call:
           dd sys_process_def.1   ; 1 = set keyboard mode
           dd sys_process_def.2   ; 2 = get keyboard mode
           dd sys_process_def.3   ; 3 = get keyboard ctrl, alt, shift
           dd sys_process_def.4   ; 4 = set system-wide hotkey
           dd sys_process_def.5   ; 5 = delete installed hotkey
           dd sys_process_def.6   ; 6 = disable input, work only hotkeys
           dd sys_process_def.7   ; 7 = enable input, opposition to f.66.6
endg
;-----------------------------------------------------------------------------
align 4
sys_process_def:
        dec     ebx
        cmp     ebx, 7
        jae     .not_support    ;if >=8 then or eax,-1

        mov     edi, [CURRENT_TASK]
        jmp     dword [f66call+ebx*4]

.not_support:
        or      eax, -1
        ret
;-----------------------------------------------------------------------------
align 4
.1:
        shl     edi, 8
        mov     [edi+SLOT_BASE + APPDATA.keyboard_mode], cl

        ret
;-----------------------------------------------------------------------------
align 4
.2:                             ; 2 = get keyboard mode
        shl     edi, 8
        movzx   eax, byte [SLOT_BASE+edi + APPDATA.keyboard_mode]
        mov     [esp+32], eax
        ret
;-----------------------------------------------------------------------------
align 4
.3:                             ;3 = get keyboard ctrl, alt, shift
        mov     eax, [kb_state]
        mov     [esp+32], eax
        ret
;-----------------------------------------------------------------------------
align 4
.4:
        mov     eax, hotkey_list
@@:
        cmp     dword [eax+8], 0
        jz      .found_free
        add     eax, 16
        cmp     eax, hotkey_list+16*256
        jb      @b
        mov     dword [esp+32], 1
        ret
.found_free:
        mov     [eax+8], edi
        mov     [eax+4], edx
        movzx   ecx, cl
        lea     ecx, [hotkey_scancodes+ecx*4]
        mov     edx, [ecx]
        mov     [eax], edx
        mov     [ecx], eax
        mov     [eax+12], ecx
        test    edx, edx
        jz      @f
        mov     [edx+12], eax
@@:
        and     dword [esp+32], 0
        ret
;-----------------------------------------------------------------------------
align 4
.5:
        movzx   ebx, cl
        lea     ebx, [hotkey_scancodes+ebx*4]
        mov     eax, [ebx]
.scan:
        test    eax, eax
        jz      .notfound
        cmp     [eax+8], edi
        jnz     .next
        cmp     [eax+4], edx
        jz      .found
.next:
        mov     eax, [eax]
        jmp     .scan
.notfound:
        mov     dword [esp+32], 1
        ret
.found:
        mov     ecx, [eax]
        jecxz   @f
        mov     edx, [eax+12]
        mov     [ecx+12], edx
@@:
        mov     ecx, [eax+12]
        mov     edx, [eax]
        mov     [ecx], edx
        xor     edx, edx
        mov     [eax+4], edx
        mov     [eax+8], edx
        mov     [eax+12], edx
        mov     [eax], edx
        mov     [esp+32], edx
        ret
;-----------------------------------------------------------------------------
align 4
.6:
        pushfd
        cli
        mov     eax, [PID_lock_input]
        test    eax, eax
        jnz     @f
; get current PID
        mov     eax, [CURRENT_TASK]
        shl     eax, 5
        mov     eax, [eax+CURRENT_TASK+TASKDATA.pid]
; set current PID for lock input
        mov     [PID_lock_input], eax
@@:
        popfd
        ret
;-----------------------------------------------------------------------------
align 4
.7:
        mov     eax, [PID_lock_input]
        test    eax, eax
        jz      @f
; get current PID
        mov     ebx, [CURRENT_TASK]
        shl     ebx, 5
        mov     ebx, [ebx+CURRENT_TASK+TASKDATA.pid]
; compare current lock input with current PID
        cmp     ebx, eax
        jne     @f

        xor     eax, eax
        mov     [PID_lock_input], eax
@@:
        ret
;-----------------------------------------------------------------------------
uglobal
  PID_lock_input dd 0x0
endg
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; 61 sys function.                                                ;;
;; in eax=61,ebx in [1..3]                                         ;;
;; out eax                                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
iglobal
align 4
f61call:
           dd sys_gs.1   ; resolution
           dd sys_gs.2   ; bits per pixel
           dd sys_gs.3   ; bytes per scanline
endg


align 4

sys_gs:                         ; direct screen access
        dec     ebx
        cmp     ebx, 2
        ja      .not_support
        jmp     dword [f61call+ebx*4]
.not_support:
        or      [esp+32], dword -1
        ret


.1:                             ; resolution
        mov     eax, [Screen_Max_X]
        shl     eax, 16
        mov     ax, word [Screen_Max_Y]
        add     eax, 0x00010001
        mov     [esp+32], eax
        ret
.2:                             ; bits per pixel
        mov     eax, [_display.bpp]
        mov     [esp+32], eax
        ret
.3:                             ; bytes per scanline
        mov     eax, [_display.pitch]
        mov     [esp+32], eax
        ret

align 4  ;  system functions

syscall_setpixel:                       ; SetPixel

        mov     eax, ebx
        mov     ebx, ecx
        mov     ecx, edx
        mov     edx, [TASK_BASE]
        add     eax, [edx-twdw+WDATA.box.left]
        add     ebx, [edx-twdw+WDATA.box.top]
        mov     edi, [current_slot]
        add     eax, [edi+APPDATA.wnd_clientbox.left]
        add     ebx, [edi+APPDATA.wnd_clientbox.top]
        xor     edi, edi ; no force
        and     ecx, 0xFBFFFFFF  ;negate 0x04000000 save to mouseunder area
;        jmp     [putpixel]
        jmp     __sys_putpixel

align 4

syscall_writetext:                      ; WriteText

        mov     eax, [TASK_BASE]
        mov     ebp, [eax-twdw+WDATA.box.left]
        push    esi
        mov     esi, [current_slot]
        add     ebp, [esi+APPDATA.wnd_clientbox.left]
        shl     ebp, 16
        add     ebp, [eax-twdw+WDATA.box.top]
        add     bp, word[esi+APPDATA.wnd_clientbox.top]
        pop     esi
        test    ecx, 0x08000000  ; redirect the output to the user area
        jnz     @f
        add     ebx, ebp
align 4
@@:
        mov     eax, edi
        test    ecx, 0x08000000  ; redirect the output to the user area
        jnz     dtext
        xor     edi, edi
        jmp     dtext

align 4

syscall_drawrect:                       ; DrawRect

        mov     edi, edx ; color + gradient
        and     edi, 0x80FFFFFF
        test    bx, bx  ; x.size
        je      .drectr
        test    cx, cx ; y.size
        je      .drectr

        mov     eax, ebx ; bad idea
        mov     ebx, ecx

        movzx   ecx, ax ; ecx - x.size
        shr     eax, 16 ; eax - x.coord
        movzx   edx, bx ; edx - y.size
        shr     ebx, 16 ; ebx - y.coord
        mov     esi, [current_slot]

        add     eax, [esi + APPDATA.wnd_clientbox.left]
        add     ebx, [esi + APPDATA.wnd_clientbox.top]
        add     ecx, eax
        add     edx, ebx
;        jmp     [drawbar]
        jmp     vesa20_drawbar
.drectr:
        ret

align 4
syscall_getscreensize:                  ; GetScreenSize
        mov     ax, word [Screen_Max_X]
        shl     eax, 16
        mov     ax, word [Screen_Max_Y]
        mov     [esp + 32], eax
        ret

align 4

syscall_cdaudio:                        ; CD

        cmp     ebx, 4
        jb      .audio
        jz      .eject
        cmp     ebx, 5
        jnz     .ret
.load:
        call    .reserve
        call    LoadMedium
        ;call    .free
        jmp     .free
;        ret
.eject:
        call    .reserve
        call    clear_CD_cache
        call    allow_medium_removal
        call    EjectMedium
;        call    .free
        jmp     .free
;        ret
.audio:
        call    sys_cd_audio
        mov     [esp+36-4], eax
.ret:
        ret

.reserve:
        call    reserve_cd
        mov     eax, ecx
        shr     eax, 1
        and     eax, 1
        inc     eax
        mov     [ChannelNumber], ax
        mov     eax, ecx
        and     eax, 1
        mov     [DiskNumber], al
        call    reserve_cd_channel
        and     ebx, 3
        inc     ebx
        mov     [cdpos], ebx
        add     ebx, ebx
        mov     cl, 8
        sub     cl, bl
        mov     al, [DRIVE_DATA+1]
        shr     al, cl
        test    al, 2
        jz      .free;.err
        ret
.free:
        call    free_cd_channel
        and     [cd_status], 0
        ret
.err:
        call    .free
;        pop     eax
        ret
;-----------------------------------------------------------------------------
align 4
syscall_getpixel_WinMap:                       ; GetPixel WinMap
        cmp     ebx, [Screen_Max_X]
        jbe     @f
        cmp     ecx, [Screen_Max_Y]
        jbe     @f
        xor     eax, eax
        jmp     .store
;--------------------------------------
align 4
@@:
        mov     eax, [d_width_calc_area + ecx*4]
        add     eax, [_WinMapAddress]
        movzx   eax, byte[eax+ebx]        ; get value for current point
;--------------------------------------
align 4
.store:
        mov     [esp + 32], eax
        ret
;-----------------------------------------------------------------------------
align 4
syscall_getpixel:                       ; GetPixel
        mov     ecx, [Screen_Max_X]
        inc     ecx
        xor     edx, edx
        mov     eax, ebx
        div     ecx
        mov     ebx, edx
        xchg    eax, ebx
        and     ecx, 0xFBFFFFFF  ;negate 0x04000000 use mouseunder area
        call    dword [GETPIXEL]; eax - x, ebx - y
        mov     [esp + 32], ecx
        ret
;-----------------------------------------------------------------------------
align 4
syscall_getarea:
;eax = 36
;ebx = pointer to bufer for img BBGGRRBBGGRR...
;ecx = [size x]*65536 + [size y]
;edx = [start x]*65536 + [start y]
        pushad
        mov     edi, ebx
        mov     eax, edx
        shr     eax, 16
        mov     ebx, edx
        and     ebx, 0xffff
        dec     eax
        dec     ebx
     ; eax - x, ebx - y
        mov     edx, ecx

        shr     ecx, 16
        and     edx, 0xffff
        mov     esi, ecx
     ; ecx - size x, edx - size y

        mov     ebp, edx
        dec     ebp
        lea     ebp, [ebp*3]

        imul    ebp, esi

        mov     esi, ecx
        dec     esi
        lea     esi, [esi*3]

        add     ebp, esi
        add     ebp, edi

        add     ebx, edx
;--------------------------------------
align 4
.start_y:
        push    ecx edx
;--------------------------------------
align 4
.start_x:
        push    eax ebx ecx
        add     eax, ecx

        and     ecx, 0xFBFFFFFF  ;negate 0x04000000 use mouseunder area
        call    dword [GETPIXEL]; eax - x, ebx - y

        mov     [ebp], cx
        shr     ecx, 16
        mov     [ebp+2], cl

        pop     ecx ebx eax
        sub     ebp, 3
        dec     ecx
        jnz     .start_x
        pop     edx ecx
        dec     ebx
        dec     edx
        jnz     .start_y
        popad
        ret
;-----------------------------------------------------------------------------
align 4
syscall_putarea_backgr:
;eax = 25
;ebx = pointer to bufer for img BBGGRRBBGGRR...
;ecx = [size x]*65536 + [size y]
;edx = [start x]*65536 + [start y]
        pushad
        mov     edi, ebx
        mov     eax, edx
        shr     eax, 16
        mov     ebx, edx
        and     ebx, 0xffff
        dec     eax
        dec     ebx
; eax - x, ebx - y
        mov     edx, ecx
        shr     ecx, 16
        and     edx, 0xffff
        mov     esi, ecx
; ecx - size x, edx - size y
        mov     ebp, edx
        dec     ebp
        shl     ebp, 2

        imul    ebp, esi

        mov     esi, ecx
        dec     esi
        shl     esi, 2

        add     ebp, esi
        add     ebp, edi

        add     ebx, edx
;--------------------------------------
align 4
.start_y:
        push    ecx edx
;--------------------------------------
align 4
.start_x:
        push    eax ecx
        add     eax, ecx

        mov     ecx, [ebp]
        rol     ecx, 8
        test    cl, cl        ; transparensy = 0
        jz      .no_put

        xor     cl, cl
        ror     ecx, 8

        pushad
        mov     edx, [d_width_calc_area + ebx*4]
        add     edx, [_WinMapAddress]
        movzx   edx, byte [eax+edx]
        cmp     dl, byte 1
        jne     @f

        call    dword [PUTPIXEL]; eax - x, ebx - y
;--------------------------------------
align 4
@@:
        popad
;--------------------------------------
align 4
.no_put:
        pop     ecx eax

        sub     ebp, 4
        dec     ecx
        jnz     .start_x

        pop     edx ecx
        dec     ebx
        dec     edx
        jnz     .start_y

        popad
        ret
;-----------------------------------------------------------------------------
align 4
syscall_drawline:                       ; DrawLine

        mov     edi, [TASK_BASE]
        movzx   eax, word[edi-twdw+WDATA.box.left]
        mov     ebp, eax
        mov     esi, [current_slot]
        add     ebp, [esi+APPDATA.wnd_clientbox.left]
        add     ax, word[esi+APPDATA.wnd_clientbox.left]
        add     ebp, ebx
        shl     eax, 16
        movzx   ebx, word[edi-twdw+WDATA.box.top]
        add     eax, ebp
        mov     ebp, ebx
        add     ebp, [esi+APPDATA.wnd_clientbox.top]
        add     bx, word[esi+APPDATA.wnd_clientbox.top]
        add     ebp, ecx
        shl     ebx, 16
        xor     edi, edi
        add     ebx, ebp
        mov     ecx, edx
;        jmp     [draw_line]
        jmp     __sys_draw_line


align 4
syscall_reserveportarea:                ; ReservePortArea and FreePortArea

        call    r_f_port_area
        mov     [esp+32], eax
        ret

align 4
syscall_threads:                        ; CreateThreads
;
;   ecx=thread entry point
;   edx=thread stack pointer
;
; on return : eax = pid

        xor     ebx, ebx
        call    new_sys_threads

        mov     [esp+32], eax
        ret

align 4

paleholder:
        ret
;------------------------------------------------------------------------------
align 4
calculate_fast_getting_offset_for_WinMapAddress:
; calculate data area for fast getting offset to _WinMapAddress
        xor     eax, eax
        mov     ecx, [_display.height]
        mov     edi, d_width_calc_area
        cld
@@:
        stosd
        add     eax, [_display.width]
        dec     ecx
        jnz     @r
        ret
;------------------------------------------------------------------------------
align 4
calculate_fast_getting_offset_for_LFB:
; calculate data area for fast getting offset to LFB
        xor     eax, eax
        mov     ecx, [_display.height]
        mov     edi, BPSLine_calc_area
        cld
@@:
        stosd
        add     eax, [_display.pitch]
        dec     ecx
        jnz     @r
        ret
;------------------------------------------------------------------------------
align 4
set_screen:
; in:
; eax - new Screen_Max_X
; ecx - new BytesPerScanLine
; edx - new Screen_Max_Y

        pushfd
        cli

        mov     [Screen_Max_X], eax
        mov     [Screen_Max_Y], edx
        mov     [_display.pitch], ecx

        mov     [screen_workarea.right], eax
        mov     [screen_workarea.bottom], edx

        push    ebx
        push    esi
        push    edi

        pushad

        cmp     [do_not_touch_winmap], 1
        je      @f

        stdcall kernel_free, [_WinMapAddress]

        mov     eax, [_display.width]
        mul     [_display.height]
        mov     [_WinMapSize], eax

        stdcall kernel_alloc, eax
        mov     [_WinMapAddress], eax
        test    eax, eax
        jz      .epic_fail
; store for f.18.24
        mov     eax, [_display.width]
        mov     [display_width_standard], eax

        mov     eax, [_display.height]
        mov     [display_height_standard], eax
@@:
        call    calculate_fast_getting_offset_for_WinMapAddress
; for Qemu or non standart video cards
; Unfortunately [BytesPerScanLine] does not always
;                             equal to [_display.width] * [ScreenBPP] / 8
        call    calculate_fast_getting_offset_for_LFB
        popad

        call    repos_windows
        xor     eax, eax
        xor     ebx, ebx
        mov     ecx, [Screen_Max_X]
        mov     edx, [Screen_Max_Y]
        call    calculatescreen
        pop     edi
        pop     esi
        pop     ebx

        popfd
        ret

.epic_fail:
        hlt                     ; Houston, we've had a problem

; --------------- APM ---------------------
uglobal
apm_entry       dp      0
apm_vf          dd      0
endg

align 4
sys_apm:
        xor     eax, eax
        cmp     word [apm_vf], ax       ; Check APM BIOS enable
        jne     @f
        inc     eax
        or      dword [esp + 44], eax   ; error
        add     eax, 7
        mov     dword [esp + 32], eax   ; 32-bit protected-mode interface not supported
        ret

@@:
;       xchg    eax, ecx
;       xchg    ebx, ecx

        cmp     dx, 3
        ja      @f
        and     [esp + 44], byte 0xfe    ; emulate func 0..3 as func 0
        mov     eax, [apm_vf]
        mov     [esp + 32], eax
        shr     eax, 16
        mov     [esp + 28], eax
        ret

@@:

        mov     esi, [master_tab+(OS_BASE shr 20)]
        xchg    [master_tab], esi
        push    esi
        mov     edi, cr3
        mov     cr3, edi                ;flush TLB

        call    pword [apm_entry]       ;call APM BIOS

        xchg    eax, [esp]
        mov     [master_tab], eax
        mov     eax, cr3
        mov     cr3, eax
        pop     eax

        mov     [esp + 4 ], edi
        mov     [esp + 8], esi
        mov     [esp + 20], ebx
        mov     [esp + 24], edx
        mov     [esp + 28], ecx
        mov     [esp + 32], eax
        setc    al
        and     [esp + 44], byte 0xfe
        or      [esp + 44], al
        ret
; -----------------------------------------

align 4

undefined_syscall:                      ; Undefined system call
        mov     [esp + 32], dword -1
        ret

align 4
system_shutdown:          ; shut down the system

        cmp     byte [BOOT_VARS+0x9030], 1
        jne     @F
        ret
@@:
        call    stop_all_services
        movi    eax, 3
        call    sys_cd_audio

yes_shutdown_param:
        cli

if ~ defined extended_primary_loader
; load kernel.mnt to 0x7000:0
        mov     ebx, kernel_file_load
        pushad
        call    file_system_lfn
        popad

        mov     esi, restart_kernel_4000+OS_BASE+0x10000 ; move kernel re-starter to 0x4000:0
        mov     edi, OS_BASE+0x40000
        mov     ecx, 1000
        rep movsb
end if

;        mov     esi, BOOT_VAR    ; restore 0x0 - 0xffff
;        mov     edi, OS_BASE
;        mov     ecx, 0x10000/4
;        cld
;        rep movsd

        call    IRQ_mask_all

if 0
        mov     word [OS_BASE+0x467+0], pr_mode_exit
        mov     word [OS_BASE+0x467+2], 0x1000

        mov     al, 0x0F
        out     0x70, al
        mov     al, 0x05
        out     0x71, al

        mov     al, 0xFE
        out     0x64, al

        hlt
        jmp     $-1

else
        cmp     byte [OS_BASE + 0x9030], 2
        jnz     no_acpi_power_off

; scan for RSDP
; 1) The first 1 Kb of the Extended BIOS Data Area (EBDA).
        movzx   eax, word [OS_BASE + 0x40E]
        shl     eax, 4
        jz      @f
        mov     ecx, 1024/16
        call    scan_rsdp
        jnc     .rsdp_found
@@:
; 2) The BIOS read-only memory space between 0E0000h and 0FFFFFh.
        mov     eax, 0xE0000
        mov     ecx, 0x2000
        call    scan_rsdp
        jc      no_acpi_power_off
.rsdp_found:
        mov     esi, [eax+16]   ; esi contains physical address of the RSDT
        mov     ebp, [ipc_tmp]
        stdcall map_page, ebp, esi, PG_MAP
        lea     eax, [esi+1000h]
        lea     edx, [ebp+1000h]
        stdcall map_page, edx, eax, PG_MAP
        and     esi, 0xFFF
        add     esi, ebp
        cmp     dword [esi], 'RSDT'
        jnz     no_acpi_power_off
        mov     ecx, [esi+4]
        sub     ecx, 24h
        jbe     no_acpi_power_off
        shr     ecx, 2
        add     esi, 24h
.scan_fadt:
        lodsd
        mov     ebx, eax
        lea     eax, [ebp+2000h]
        stdcall map_page, eax, ebx, PG_MAP
        lea     eax, [ebp+3000h]
        add     ebx, 0x1000
        stdcall map_page, eax, ebx, PG_MAP
        and     ebx, 0xFFF
        lea     ebx, [ebx+ebp+2000h]
        cmp     dword [ebx], 'FACP'
        jz      .fadt_found
        loop    .scan_fadt
        jmp     no_acpi_power_off
.fadt_found:
; ebx is linear address of FADT
        mov     edi, [ebx+40] ; physical address of the DSDT
        lea     eax, [ebp+4000h]
        stdcall map_page, eax, edi, PG_MAP
        lea     eax, [ebp+5000h]
        lea     esi, [edi+0x1000]
        stdcall map_page, eax, esi, PG_MAP
        and     esi, 0xFFF
        sub     edi, esi
        cmp     dword [esi+ebp+4000h], 'DSDT'
        jnz     no_acpi_power_off
        mov     eax, [esi+ebp+4004h] ; DSDT length
        sub     eax, 36+4
        jbe     no_acpi_power_off
        add     esi, 36
.scan_dsdt:
        cmp     dword [esi+ebp+4000h], '_S5_'
        jnz     .scan_dsdt_cont
        cmp     byte [esi+ebp+4000h+4], 12h ; DefPackage opcode
        jnz     .scan_dsdt_cont
        mov     dl, [esi+ebp+4000h+6]
        cmp     dl, 4 ; _S5_ package must contain 4 bytes
                      ; ...in theory; in practice, VirtualBox has 2 bytes
        ja      .scan_dsdt_cont
        cmp     dl, 1
        jb      .scan_dsdt_cont
        lea     esi, [esi+ebp+4000h+7]
        xor     ecx, ecx
        cmp     byte [esi], 0 ; 0 means zero byte, 0Ah xx means byte xx
        jz      @f
        cmp     byte [esi], 0xA
        jnz     no_acpi_power_off
        inc     esi
        mov     cl, [esi]
@@:
        inc     esi
        cmp     dl, 2
        jb      @f
        cmp     byte [esi], 0
        jz      @f
        cmp     byte [esi], 0xA
        jnz     no_acpi_power_off
        inc     esi
        mov     ch, [esi]
@@:
        jmp     do_acpi_power_off
.scan_dsdt_cont:
        inc     esi
        cmp     esi, 0x1000
        jb      @f
        sub     esi, 0x1000
        add     edi, 0x1000
        push    eax
        lea     eax, [ebp+4000h]
        stdcall map_page, eax, edi, PG_MAP
        push    PG_MAP
        lea     eax, [edi+1000h]
        push    eax
        lea     eax, [ebp+5000h]
        push    eax
        stdcall map_page
        pop     eax
@@:
        dec     eax
        jnz     .scan_dsdt
        jmp     no_acpi_power_off
do_acpi_power_off:
        mov     edx, [ebx+48]
        test    edx, edx
        jz      .nosmi
        mov     al, [ebx+52]
        out     dx, al
        mov     edx, [ebx+64]
@@:
        in      ax, dx
        test    al, 1
        jz      @b
.nosmi:
        and     cx, 0x0707
        shl     cx, 2
        or      cx, 0x2020
        mov     edx, [ebx+64]
        in      ax, dx
        and     ax, 203h
        or      ah, cl
        out     dx, ax
        mov     edx, [ebx+68]
        test    edx, edx
        jz      @f
        in      ax, dx
        and     ax, 203h
        or      ah, ch
        out     dx, ax
@@:
        jmp     $


no_acpi_power_off:
        mov     word [OS_BASE+0x467+0], pr_mode_exit
        mov     word [OS_BASE+0x467+2], 0x1000

        mov     al, 0x0F
        out     0x70, al
        mov     al, 0x05
        out     0x71, al

        mov     al, 0xFE
        out     0x64, al

        hlt
        jmp     $-1

scan_rsdp:
        add     eax, OS_BASE
.s:
        cmp     dword [eax], 'RSD '
        jnz     .n
        cmp     dword [eax+4], 'PTR '
        jnz     .n
        xor     edx, edx
        xor     esi, esi
@@:
        add     dl, [eax+esi]
        inc     esi
        cmp     esi, 20
        jnz     @b
        test    dl, dl
        jz      .ok
.n:
        add     eax, 10h
        loop    .s
        stc
.ok:
        ret
end if

if ~ lang eq sp
diff16 "end of .text segment",0,$
end if

include "data32.inc"

__REV__ = __REV

if ~ lang eq sp
diff16 "end of kernel code",0,$
end if
