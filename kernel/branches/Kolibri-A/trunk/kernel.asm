;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Copyright (C) KolibriOS team 2004-2010. All rights reserved.
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
;; Artem Jerdev (art_zh)
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

include 'macros.inc'

$Revision$


USE_COM_IRQ	equ 1	   ; make irq 3 and irq 4 available for PCI devices

; Enabling the next line will enable serial output console
;debug_com_base  equ 0x3f8  ; 0x3f8 is com1, 0x2f8 is com2, 0x3e8 is com3, 0x2e8 is com4, no irq's are used

include "proc32.inc"
include "kglobals.inc"

include "const.inc"
max_processes	 equ   255
tss_step	 equ   (128+8192) ; tss & i/o - 65535 ports, * 256=557056*4


os_stack       equ  (os_data_l-gdts)	; GDTs
os_code        equ  (os_code_l-gdts)
graph_data     equ  (3+graph_data_l-gdts)
tss0	       equ  (tss0_l-gdts)
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
		  org	0x0
		  jmp	start_of_code

version db    'Kolibri OS  version 0.7.7.0+     ',13,10,13,10,0

include "boot/bootstr.inc"     ; language-independent boot messages
include "boot/preboot.inc"

include "boot/booteng.inc"     ; english system boot messages

include "boot/bootcode.inc"    ; 16 bit system boot code
include "bus/pci/pci16.inc"
include "detect/biosdisk.inc"

diff16 "end of code16  ",0,$

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                      ;;
;;                  SWITCH TO 32 BIT PROTECTED MODE                     ;;
;;                                                                      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; CR0 Flags - Protected mode and Paging

	mov ecx, CR0_PE

; Enabling 32 bit protected mode

	sidt	[cs:old_ints_h]

	cli				; disable all irqs
	cld
	mov	al,255			; mask all irqs
	out	0xa1,al
	out	0x21,al
   l.5: in	al, 0x64		; Enable A20
	test	al, 2
	jnz	l.5
	mov	al, 0xD1
	out	0x64, al
   l.6: in	al, 0x64
	test	al, 2
	jnz	l.6
	mov	al, 0xDF
	out	0x60, al
   l.7: in	al, 0x64
	test	al, 2
	jnz	l.7
	mov	al, 0xFF
	out	0x64, al

	lgdt	[cs:tmp_gdt]		; Load GDT
	mov	eax, cr0		; protected mode
	or	eax, ecx
	and	eax, 10011111b *65536*256 + 0xffffff ; caching enabled
	mov	cr0, eax
	jmp	pword os_code:B32	; jmp to enable 32 bit mode

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

diff16 "end of tmp_gdt ",0,$

include "data16.inc"

diff16 "end of data16  ",0,$

use32
org $+0x10000

align 4
B32:
diff16 "32-bit code start ",0,$
	   mov	 ax,os_stack	   ; Selector for os
	   mov	 ds,ax
	   mov	 es,ax
	   mov	 fs,ax
	   mov	 gs,ax
	   mov	 ss,ax
	   mov	 esp,0x4ec00	   ; Set stack

;-------------------------------------------------------------------------------
	   call preinit_mem	; (init.inc)

	   call test_cpu	; (init.inc - to be moved to bus/CPU.inc)
	   bts [cpu_caps-OS_BASE], CAPS_TSC	;force use rdtsc

	   call init_BIOS32	; (init.inc - to be removed later)

; PCIe extended config space access
	   call rs7xx_pcie_init	; (bus/HT.inc)

; MEMORY MODEL
	   call init_mem	; (init.inc)
	   call init_page_map	; (init.inc)

; ENABLE PAGING

	   mov eax, sys_pgdir-OS_BASE
	   mov cr3, eax

	   mov eax,cr0
	   or eax,CR0_PG+CR0_WP
	   mov cr0,eax

	   lgdt [gdts]
	   jmp pword os_code:high_code

align 4
bios32_entry	dd ?
tmp_page_tabs	dd ?

use16
org $-0x10000
include "boot/shutdown.inc" ; shutdown or restart
org $+0x10000
use32

__DEBUG__ fix 1
__DEBUG_LEVEL__ fix 1
include 'init.inc'

org OS_BASE+$

align 4
high_code:
	   mov ax, os_stack
	   mov bx, app_data
	   mov cx, app_tls
	   mov ss, ax
	   add esp, OS_BASE

	   mov ds, bx
	   mov es, bx
	   mov fs, cx
	   mov gs, bx

	   bt [cpu_caps], CAPS_PGE
	   jnc @F

	   or dword [sys_pgdir+(OS_BASE shr 20)], PG_GLOBAL

	   mov ebx, cr4
	   or ebx, CR4_PGE
	   mov cr4, ebx
@@:
	   xor eax, eax
	   mov dword [sys_pgdir], eax
	   mov dword [sys_pgdir+4], eax

	   mov eax, cr3
	   mov cr3, eax 	  ; flush TLB

; SAVE REAL MODE VARIABLES
	mov	ax, [BOOT_VAR + 0x9031]
	mov	[IDEContrRegsBaseAddr], ax
; --------------- APM ---------------------

; init selectors
    mov ebx,[BOOT_VAR+0x9040]			; offset of APM entry point
    movzx eax,word [BOOT_VAR+0x9050]	; real-mode segment base address of
						; protected-mode 32-bit code segment
    movzx ecx,word [BOOT_VAR+0x9052]	; real-mode segment base address of
						; protected-mode 16-bit code segment
    movzx edx,word [BOOT_VAR+0x9054]	; real-mode segment base address of
						; protected-mode 16-bit data segment

    shl    eax, 4
    mov    [dword apm_code_32 + 2], ax
    shr    eax, 16
    mov    [dword apm_code_32 + 4], al

    shl    ecx, 4
    mov    [dword apm_code_16 + 2], cx
    shr    ecx, 16
    mov    [dword apm_code_16 + 4], cl

    shl    edx, 4
    mov    [dword apm_data_16 + 2], dx
    shr    edx, 16
    mov    [dword apm_data_16 + 4], dl

    mov    dword[apm_entry], ebx
    mov    word [apm_entry + 4], apm_code_32 - gdts

    mov    eax, [BOOT_VAR + 0x9044]    ; version & flags
    mov    [apm_vf], eax
; -----------------------------------------
;        movzx eax,byte [BOOT_VAR+0x9010]       ; mouse port
;        mov   [0xF604],byte 1  ;al
		mov	al, [BOOT_VAR+0x901F]		; DMA access
	mov	[allow_dma_access], al
	movzx eax, byte [BOOT_VAR+0x9000]	 ; bpp
	mov [ScreenBPP],al

	mov [_display.bpp], eax
	mov [_display.vrefresh], 60
	mov [_display.disable_mouse],  __sys_disable_mouse

	movzx eax,word [BOOT_VAR+0x900A]  ; X max
	mov [_display.width], eax
	dec   eax
	mov   [Screen_Max_X],eax
	mov   [screen_workarea.right],eax
	movzx eax,word [BOOT_VAR+0x900C]  ; Y max
	mov [_display.height], eax
	dec   eax
	mov   [Screen_Max_Y],eax
	mov   [screen_workarea.bottom],eax
	movzx eax,word [BOOT_VAR+0x9008]  ; screen mode
	mov   [SCR_MODE],eax
	mov   eax,[BOOT_VAR+0x9014]	  ; Vesa 1.2 bnk sw add
	mov   [BANK_SWITCH],eax
	mov   [BytesPerScanLine],word 640*4	    ; Bytes PerScanLine
	cmp   [SCR_MODE],word 0x13	    ; 320x200
	je    @f
	cmp   [SCR_MODE],word 0x12	    ; VGA 640x480
	je    @f
	movzx eax, word[BOOT_VAR+0x9001]	; for other modes
	mov   [BytesPerScanLine],ax
	mov [_display.pitch], eax
@@:
	mov eax, [_display.width]
	mul [_display.height]
	mov [_WinMapSize], eax

	mov	esi, BOOT_VAR+0x9080
	movzx	ecx, byte [esi-1]
	mov	[NumBiosDisks], ecx
	mov	edi, BiosDisksData
	rep	movsd


; GRAPHICS ADDRESSES

	and	byte [BOOT_VAR+0x901e],0x0
	mov	eax,[BOOT_VAR+0x9018]
	mov	[LFBAddress],eax

	cmp	[SCR_MODE],word 0100000000000000b
	jge	setvesa20
	mov	eax, 0xDEADBEEF
	hlt
;        ===  EGA, VGA & Vesa 1.2 modes not supported ===
setvesa20:
v20ga32:
	mov	[PUTPIXEL],dword Vesa20_putpixel32
	mov	[GETPIXEL],dword Vesa20_getpixel32

; -------- Fast System Call init ----------
.SEnP:
; AMD SYSCALL/SYSRET
	   mov ecx, MSR_AMD_EFER
	   rdmsr
	   or eax, 1   ; bit_0 - System Call Extension (SCE)
	   wrmsr

	; Bits of EDX :
	; Bit 31..16 During the SYSRET instruction, this field is copied into the CS register
	;  and the contents of this field, plus 8, are copied into the SS register.
	; Bit 15..0 During the SYSCALL instruction, this field is copied into the CS register
	;  and the contents of this field, plus 8, are copied into the SS register.

	   mov edx, 0x1B000B	; RING3 task stack will be used for fast syscalls!

	   mov eax, syscall_entry
	   mov ecx, MSR_AMD_STAR
	   wrmsr
.noSYSCALL:
; -----------------------------------------
	stdcall alloc_page
	stdcall map_page, tss-0xF80, eax, PG_SW 	; lower 0xF80 bytes might be used for something
	stdcall alloc_page
	inc	eax
	mov	[SLOT_BASE+256+APPDATA.io_map], eax
	stdcall map_page, tss+0x80, eax, PG_SW
	stdcall alloc_page
	inc	eax
	mov	dword [SLOT_BASE+256+APPDATA.io_map+4], eax
	stdcall map_page, tss+0x1080, eax, PG_SW

; LOAD IDT

	   call build_interrupt_table ;lidt is executed
	  ;lidt [idtreg]

	   call init_kernel_heap
	   stdcall kernel_alloc, RING0_STACK_SIZE+512
	   mov [os_stack_seg], eax

	   lea esp, [eax+RING0_STACK_SIZE]

	   mov [tss._ss0], os_stack
	   mov [tss._esp0], esp
	   mov [tss._esp], esp
	   mov [tss._cs],os_code
	   mov [tss._ss],os_stack
	   mov [tss._ds],app_data
	   mov [tss._es],app_data
	   mov [tss._fs],app_data
	   mov [tss._gs],app_data
	   mov [tss._io],128
;Add IO access table - bit array of permitted ports
	   mov edi, tss._io_map_0
	   xor eax, eax
	   mov ecx, 2047
	   rep stosd		     ; access to 65504 ports granted
	   not eax		     ; the last 32 ports blocked
	   stosd

	   mov	ax,tss0
	   ltr	ax

	   mov [LFBSize], 0x800000
	   call init_LFB
	   call init_fpu
	   call init_malloc
;-
	   stdcall alloc_kernel_space, 0x51000
	   mov [default_io_map], eax

	   add eax, 0x2000
	   mov [ipc_tmp], eax
	   mov ebx, 0x1000

	   add eax, 0x40000
	   mov [proc_mem_map], eax

	   add eax, 0x8000
	   mov [proc_mem_pdir], eax

	   add eax, ebx
	   mov [proc_mem_tab], eax

	   add eax, ebx
	   mov [tmp_task_pdir], eax

	   add eax, ebx
	   mov [tmp_task_ptab], eax

	   add eax, ebx
	   mov [ipc_pdir], eax

	   add eax, ebx
	   mov [ipc_ptab], eax

	   stdcall kernel_alloc, (unpack.LZMA_BASE_SIZE+(unpack.LZMA_LIT_SIZE shl \
				 (unpack.lc+unpack.lp)))*4

	   mov [unpack.p], eax

	   call init_events
	   mov eax, srv.fd-SRV_FD_OFFSET
	   mov [srv.fd], eax
	   mov [srv.bk], eax

	   mov edi, irq_tab
	   xor eax, eax
	   mov ecx, 16
	   rep stosd

;Set base of graphic segment to linear address of LFB
	mov	eax,[LFBAddress]	  ; set for gs
	mov	[graph_data_l+2],ax
	shr	eax,16
	mov	[graph_data_l+4],al
	mov	[graph_data_l+7],ah

	stdcall kernel_alloc, [_WinMapSize]
	mov [_WinMapAddress], eax

	xor  eax,eax
	inc  eax
	mov [CURRENT_TASK],eax		;dword 1
	mov [TASK_COUNT],eax		;dword 1
	mov [TASK_BASE],dword TASK_DATA
	mov [current_slot], SLOT_BASE+256

; set background

	mov   [BgrDrawMode],eax
	mov   [BgrDataWidth],eax
	mov   [BgrDataHeight],eax
	mov    [mem_BACKGROUND], 4
	mov [img_background], static_background_data

	mov	[SLOT_BASE + 256 + APPDATA.dir_table], sys_pgdir - OS_BASE

	stdcall kernel_alloc, 0x10000/8
	mov	edi, eax
	mov	[network_free_ports], eax
	or	eax, -1
	mov	ecx, 0x10000/32
	rep	stosd

; REDIRECT ALL IRQ'S TO INT'S 0x20-0x2f

	call  rerouteirqs

; Initialize system V86 machine
	call	init_sys_v86

; TIMER SET TO 1/100 S

	mov   al,0x34		   ; set to 100Hz
	out   0x43,al
	mov   al,0x9b		   ; lsb    1193180 / 1193
	out   0x40,al
	mov   al,0x2e		   ; msb
	out   0x40,al

; Enable timer IRQ (IRQ0) and hard drives IRQs (IRQ14, IRQ15)
; they are used: when partitions are scanned, hd_read relies on timer
; Also enable IRQ2, because in some configurations
; IRQs from slave controller are not delivered until IRQ2 on master is enabled
	mov	al, 0xFA
	out	0x21, al
	mov	al, 0x3F
	out	0xA1, al

;!!!!!!!!!!!!!!!!!!!!!!!!!!
include 'detect/disks.inc'
;!!!!!!!!!!!!!!!!!!!!!!!!!!

  call Parser_params

; READ RAMDISK IMAGE FROM HD

;!!!!!!!!!!!!!!!!!!!!!!!
include 'boot/rdload.inc'
;!!!!!!!!!!!!!!!!!!!!!!!
;    mov    [dma_hdd],1
; CALCULATE FAT CHAIN FOR RAMDISK

	call  calculatefatchain

; LOAD VMODE DRIVER

;!!!!!!!!!!!!!!!!!!!!!!!
include 'vmodeld.inc'
;!!!!!!!!!!!!!!!!!!!!!!!

if 0
  mov ax,[OS_BASE+0x10000+bx_from_load]
  cmp ax,'r1'		; if using not ram disk, then load librares and parameters {SPraid.simba}
  je  no_lib_load
; LOADING LIBRARES
   stdcall dll.Load,@IMPORT		    ; loading librares for kernel (.obj files)
   call load_file_parse_table		    ; prepare file parse table
   call set_kernel_conf 		    ; configure devices and gui
no_lib_load:
end if

; LOAD FONTS I and II

	stdcall read_file, char, FONT_I, 0, 2304
	stdcall read_file, char2, FONT_II, 0, 2560

	mov   esi,boot_fonts
	call  boot_log

; PRINT AMOUNT OF MEMORY
	mov	esi, boot_memdetect
	call	boot_log

	movzx	ecx, word [boot_y]
	or	ecx, (10+29*6) shl 16 ; "Determining amount of memory"
	sub	ecx, 10
	mov	edx, 0xFFFFFF
	mov	ebx, [MEM_AMOUNT]
	shr	ebx, 20
	xor	edi,edi
	mov	eax, 0x00040000
		inc		edi
	call	display_number_force

; BUILD SCHEDULER

	call   build_scheduler ; sys32.inc

	mov    esi,boot_devices
	call   boot_log

	mov  [pci_access_enabled],1


; SET PRELIMINARY WINDOW STACK AND POSITIONS

	mov   esi,boot_windefs
	call  boot_log
	call  set_window_defaults

; SET BACKGROUND DEFAULTS

	mov   esi,boot_bgr
	call  boot_log
	call  init_background
	call  calculatebackground

; RESERVE SYSTEM IRQ'S JA PORT'S

;        mov   esi,boot_resirqports
;        call  boot_log
;        call  reserve_irqs_ports


; SET UP OS TASK

	mov  esi,boot_setostask
	call boot_log

	xor  eax, eax
	mov  dword [SLOT_BASE+APPDATA.fpu_state], fpu_data
	mov  dword [SLOT_BASE+APPDATA.exc_handler], eax
	mov  dword [SLOT_BASE+APPDATA.except_mask], eax

	; name for OS/IDLE process

	mov dword [SLOT_BASE+256+APPDATA.app_name],   dword 'OS/I'
	mov dword [SLOT_BASE+256+APPDATA.app_name+4], dword 'DLE '
	mov edi, [os_stack_seg]
	mov dword [SLOT_BASE+256+APPDATA.pl0_stack], edi
	add edi, 0x2000-512
	mov dword [SLOT_BASE+256+APPDATA.fpu_state], edi
	mov dword [SLOT_BASE+256+APPDATA.saved_esp0], edi ; just for case
	; [SLOT_BASE+256+APPDATA.io_map] was set earlier

	mov esi, fpu_data
	mov ecx, 512/4
	cld
	rep movsd

	mov dword [SLOT_BASE+256+APPDATA.exc_handler], eax
	mov dword [SLOT_BASE+256+APPDATA.except_mask], eax

	mov ebx, SLOT_BASE+256+APP_OBJ_OFFSET
	mov  dword [SLOT_BASE+256+APPDATA.fd_obj], ebx
	mov  dword [SLOT_BASE+256+APPDATA.bk_obj], ebx

	mov  dword [SLOT_BASE+256+APPDATA.cur_dir], sysdir_path
	mov dword [SLOT_BASE+256+APPDATA.tls_base], eax

	; task list
	mov  dword [TASK_DATA+TASKDATA.mem_start],eax	; process base address
	inc  eax
	mov  dword [CURRENT_TASK],eax
	mov  dword [TASK_COUNT],eax
	mov  [current_slot], SLOT_BASE+256
	mov  [TASK_BASE],dword TASK_DATA
	mov  byte[TASK_DATA+TASKDATA.wnd_number],al	; on screen number
	mov  dword [TASK_DATA+TASKDATA.pid], eax	; process id number

	call init_display
	mov eax, [def_cursor]
	mov [SLOT_BASE+APPDATA.cursor],eax
	mov [SLOT_BASE+APPDATA.cursor+256],eax

  ; READ TSC / SECOND

	mov   esi,boot_tsc
	call  boot_log
	cli
	rdtsc ;call  _rdtsc
	mov   ecx,eax
	mov   esi,250		    ; wait 1/4 a second
	call  delay_ms
	rdtsc ;call  _rdtsc
	sti
	sub   eax,ecx
	shl   eax,2
	mov   [CPU_FREQ],eax	      ; save tsc / sec
;       mov ebx, 1000000
;       div ebx
; faster division possible:
	mov	edx, 2251799814
	mul	edx
	shr	edx, 19
	mov [stall_mcs], edx
; PRINT CPU FREQUENCY
	mov	esi, boot_cpufreq
	call	boot_log

	mov	ebx, edx
	movzx	ecx, word [boot_y]
	add	ecx, (10+17*6) shl 16 - 10 ; 'CPU frequency is '
	mov	edx, 0xFFFFFF
	xor	edi,edi
	mov	eax, 0x00040000
		inc		edi
	call	display_number_force

; SET VARIABLES

	call  set_variables

; SET MOUSE

	;call   detect_devices
	stdcall load_driver, szPS2MDriver
;        stdcall load_driver, szCOM_MDriver

	mov   esi,boot_setmouse
	call  boot_log
	call  setmouse


; STACK AND FDC

	call  stack_init
	call  fdc_init


; LOAD DEFAULT SKIN

	call	load_default_skin

;protect io permission map

	   mov esi, [default_io_map]
	   stdcall map_page,esi,[SLOT_BASE+256+APPDATA.io_map], PG_MAP
	   add esi, 0x1000
	   stdcall map_page,esi,[SLOT_BASE+256+APPDATA.io_map+4], PG_MAP

	   stdcall map_page,tss._io_map_0,\
		   [SLOT_BASE+256+APPDATA.io_map], PG_MAP
	   stdcall map_page,tss._io_map_1,\
		   [SLOT_BASE+256+APPDATA.io_map+4], PG_MAP

  mov ax,[OS_BASE+0x10000+bx_from_load]
  cmp ax,'r1'		; if not rused ram disk - load network configuration from files {SPraid.simba}
  je  no_st_network
	call set_network_conf
  no_st_network:

	call init_userDMA	; <<<<<<<<< ============== core/memory.inc =================
	mov	esi, boot_uDMA_ok
	call	boot_log

; LOAD FIRST APPLICATION
	cli

	cmp   byte [BOOT_VAR+0x9030],1
	jne   no_load_vrr_m

	mov	ebp, vrr_m
	call	fs_execute_from_sysdir

;        cmp   eax,2                  ; if vrr_m app found (PID=2)
	sub   eax,2
	jz    first_app_found

no_load_vrr_m:

	mov	ebp, firstapp
	call	fs_execute_from_sysdir

;        cmp   eax,2                  ; continue if a process has been loaded
	sub   eax,2
	jz    first_app_found

	mov	esi, boot_failed
	call	boot_log

	mov   eax, 0xDEADBEEF	     ; otherwise halt
	hlt

first_app_found:

	cli

	;mov   [TASK_COUNT],dword 2
	push  1
	pop   dword [CURRENT_TASK]	; set OS task fisrt

; SET KEYBOARD PARAMETERS
	mov   al, 0xf6	       ; reset keyboard, scan enabled
	call  kb_write

	; wait until 8042 is ready
	xor ecx,ecx
      @@:
	in     al,64h
	and    al,00000010b
	loopnz @b

       ; mov   al, 0xED       ; svetodiody - only for testing!
       ; call  kb_write
       ; call  kb_read
       ; mov   al, 111b
       ; call  kb_write
       ; call  kb_read

	mov   al, 0xF3	     ; set repeat rate & delay
	call  kb_write
;        call  kb_read
	mov   al, 0 ; 30 250 ;00100010b ; 24 500  ;00100100b  ; 20 500
	call  kb_write
;        call  kb_read
     ;// mike.dld [
	call  set_lights
     ;// mike.dld ]


; Setup serial output console (if enabled)

if defined debug_com_base

	; enable Divisor latch

	mov	dx, debug_com_base+3
	mov	al, 1 shl 7
	out	dx, al

	; Set speed to 115200 baud (max speed)

	mov	dx, debug_com_base
	mov	al, 0x01
	out	dx, al

	mov	dx, debug_com_base+1
	mov	al, 0x00
	out	dx, al

	; No parity, 8bits words, one stop bit, dlab bit back to 0

	mov	dx, debug_com_base+3
	mov	al, 3
	out	dx, al

	; disable interrupts

	mov	dx, debug_com_base+1
	mov	al, 0
	out	dx, al

	; clear +  enable fifo (64 bits)

	mov	dx, debug_com_base+2
	mov	al, 0x7 + 1 shl 5
	out	dx, al


end if

; START MULTITASKING

if preboot_blogesc
	mov	esi, boot_tasking
	call	boot_log
.bll1:	in	al, 0x60	; wait for ESC key press
	cmp	al, 129
	jne	.bll1
end if

;       mov   [ENABLE_TASKSWITCH],byte 1        ; multitasking enabled

; UNMASK ALL IRQ'S

;        mov   esi,boot_allirqs
;        call  boot_log
;
;        cli                          ;guarantee forbidance of interrupts.
;        mov   al,0                   ; unmask all irq's
;        out   0xA1,al
;        out   0x21,al
;
;        mov   ecx,32
;
;     ready_for_irqs:
;
;        mov   al,0x20                ; ready for irqs
;        out   0x20,al
;        out   0xa0,al
;
;        loop  ready_for_irqs         ; flush the queue

	stdcall attach_int_handler, 1, irq1, 0

;        mov    [dma_hdd],1
	cmp	[IDEContrRegsBaseAddr], 0
	setnz	[dma_hdd]
	mov [timer_ticks_enable],1		; for cd driver

	sti
	call change_task

	jmp osloop

;        jmp   $                      ; wait here for timer to take control

	; Fly :)

include 'unpacker.inc'
include 'fdo.inc'

align 4
boot_log:
	 pushad

	mov   ebx,10*65536
	mov   bx,word [boot_y]
	add   [boot_y],dword 10
	mov   ecx,0x80ffffff   ; ASCIIZ string with white color
		xor	  edi,edi
	mov   edx,esi
		inc	  edi
	call  dtext

;        mov   [novesachecksum],1000
;        call  checkVga_N13

	popad

	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                                    ;
;                    MAIN OS LOOP START                              ;
;                                                                    ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 32
osloop:
	call   [draw_pointer]
	call	window_check_events
	call	mouse_check_events
	call   checkmisc
;        call   checkVga_N13
	call   stack_handler
	call   checkidle
	call   check_fdd_motor_status
;        call   check_ATAPI_device_event
	jmp    osloop
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                                    ;
;                      MAIN OS LOOP END                              ;
;                                                                    ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 4
checkidle:
	pushad
	call	change_task
	jmp	idle_loop_entry
  idle_loop:
	cmp	eax,[idlemem]	  ; eax == [timer_ticks]
	jne	idle_exit
	rdtsc	;call _rdtsc
	mov	ecx,eax
	hlt
	rdtsc	;call _rdtsc
	sub	eax,ecx
	add	[idleuse],eax
  idle_loop_entry:
	mov	eax,[timer_ticks] ; eax =  [timer_ticks]
	cmp	[check_idle_semaphore],0
	je	idle_loop
	dec	[check_idle_semaphore]
  idle_exit:
	mov	[idlemem],eax	  ; eax == [timer_ticks]
	popad
	ret

uglobal
  idlemem		dd   0x0
  idleuse		dd   0x0
  idleusesec		dd   0x0
  check_idle_semaphore	dd   0x0
endg



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
	; removed
	ret

setirqreadports:
	; removed
	ret

iglobal
  process_number dd 0x1
endg

set_variables:

	mov   ecx,0x100 		      ; flush port 0x60
.fl60:	in    al,0x60
	loop  .fl60
	push  eax

	mov   ax,[BOOT_VAR+0x900c]
	shr   ax,1
	shl   eax,16
	mov   ax,[BOOT_VAR+0x900A]
	shr   ax,1
	mov   [MOUSE_X],eax

	xor   eax,eax
	mov   [BTN_ADDR],dword BUTTON_INFO    ; address of button list

	mov   byte [MOUSE_BUFF_COUNT],al		 ; mouse buffer
	mov   byte [KEY_COUNT],al		  ; keyboard buffer
	mov   byte [BTN_COUNT],al		  ; button buffer
;        mov   [MOUSE_X],dword 100*65536+100    ; mouse x/y

     ;!! IP 04.02.2005:
	mov   byte [DONT_SWITCH],al ; change task if possible
	pop   eax
	ret

align 4
;input  eax=43,bl-byte of output, ecx - number of port
sys_outport:
     and   [esp+32],dword 1	; for backward compatibility: operation failed
    ret

display_number:
;It is not optimization
	mov	eax, ebx
	mov	ebx, ecx
	mov	ecx, edx
	mov	edx, esi
	mov	esi, edi
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
    push  eax
    and   eax,0x3fffffff
    cmp   eax,0xffff		; length > 0 ?
    pop   eax
    jge   cont_displ
    ret
   cont_displ:
     push  eax
     and   eax,0x3fffffff
     cmp   eax,61*0x10000	 ; length <= 60 ?
     pop   eax
     jb    cont_displ2
     ret
   cont_displ2:

     pushad

     cmp   al,1 		 ; ecx is a pointer ?
     jne   displnl1
     mov   ebp,ebx
     add   ebp,4
     mov   ebp,[ebp+std_application_base_address]
     mov   ebx,[ebx+std_application_base_address]
 displnl1:
     sub   esp,64

    test   ah,ah		  ; DECIMAL
    jnz   no_display_desnum
    shr   eax,16
    and   eax,0xC03f
;     and   eax,0x3f
    push  eax
    and   eax,0x3f
    mov   edi,esp
    add   edi,4+64-1
    mov   ecx,eax
    mov   eax,ebx
    mov   ebx,10
 d_desnum:
     xor   edx,edx
     call  division_64_bits
     div   ebx
     add   dl,48
     mov   [edi],dl
     dec   edi
     loop  d_desnum
     pop   eax
     call  normalize_number
     call  draw_num_text
     add   esp,64
     popad
     ret
   no_display_desnum:

     cmp   ah,0x01		 ; HEXADECIMAL
     jne   no_display_hexnum
     shr   eax,16
     and   eax,0xC03f
;     and   eax,0x3f
     push  eax
     and   eax,0x3f
     mov   edi,esp
     add   edi,4+64-1
     mov   ecx,eax
     mov   eax,ebx
     mov   ebx,16
   d_hexnum:
     xor   edx,edx
     call  division_64_bits
     div   ebx
   hexletters = __fdo_hexdigits
     add   edx,hexletters
     mov   dl,[edx]
     mov   [edi],dl
     dec   edi
     loop  d_hexnum
     pop   eax
     call  normalize_number
     call  draw_num_text
     add   esp,64
     popad
     ret
   no_display_hexnum:

     cmp   ah,0x02		 ; BINARY
     jne   no_display_binnum
     shr   eax,16
     and   eax,0xC03f
;     and   eax,0x3f
     push  eax
     and   eax,0x3f
     mov   edi,esp
     add   edi,4+64-1
     mov   ecx,eax
     mov   eax,ebx
     mov   ebx,2
   d_binnum:
     xor   edx,edx
     call  division_64_bits
     div   ebx
     add   dl,48
     mov   [edi],dl
     dec   edi
     loop  d_binnum
     pop   eax
     call  normalize_number
     call  draw_num_text
     add   esp,64
     popad
     ret
   no_display_binnum:

     add   esp,64
     popad
     ret

normalize_number:
     test  ah,0x80
     jz   .continue
     mov  ecx,48
     and   eax,0x3f
@@:
     inc   edi
     cmp   [edi],cl
     jne   .continue
     dec   eax
     cmp   eax,1
     ja    @r
     mov   al,1
.continue:
     and   eax,0x3f
     ret

division_64_bits:
     test  [esp+1+4],byte 0x40
     jz   .continue
     push  eax
     mov   eax,ebp
     div   ebx
     mov   ebp,eax
     pop   eax
.continue:
     ret

draw_num_text:
     mov   esi,eax
     mov   edx,64+4
     sub   edx,eax
     add   edx,esp
     mov   ebx,[esp+64+32-8+4]
; add window start x & y
     mov   ecx,[TASK_BASE]

     mov   edi,[CURRENT_TASK]
     shl   edi,8

     mov   eax,[ecx-twdw+WDATA.box.left]
     add   eax,[edi+SLOT_BASE+APPDATA.wnd_clientbox.left]
     shl   eax,16
     add   eax,[ecx-twdw+WDATA.box.top]
     add   eax,[edi+SLOT_BASE+APPDATA.wnd_clientbox.top]
     add   ebx,eax
     mov   ecx,[esp+64+32-12+4]
	and	ecx, not 0x80000000	; force counted string
	mov	eax, [esp+64+8] 	; background color (if given)
	mov	edi, [esp+64+4]
     jmp   dtext

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


	and  [esp+32],dword 0
	dec	ebx				; MIDI
	jnz  nsyse1
	cmp  ecx,0x100

	jb   nsyse1
	mov	esi,65535
	cmp	esi,ecx

	jb   nsyse1
	mov  [midi_base],cx	;bx
	mov  word [mididp],cx	;bx
	inc  cx 		;bx
	mov  word [midisp],cx	;bx
	ret

iglobal
midi_base dw 0
endg

   nsyse1:
	dec	ebx				 ; KEYBOARD
	jnz  nsyse2
	mov  edi,[TASK_BASE]
	mov  eax,[edi+TASKDATA.mem_start]
	add  eax,edx

	dec	ecx
	jnz  kbnobase
	mov  ebx,keymap
	mov  ecx,128
	call memmove
	ret
   kbnobase:
	dec  ecx
	jnz  kbnoshift

	mov  ebx,keymap_shift
	mov  ecx,128
	call memmove
	ret
   kbnoshift:
	dec  ecx
	jnz  kbnoalt
	mov  ebx,keymap_alt
	mov  ecx,128
	call memmove
	ret
   kbnoalt:
	sub  ecx,6
	jnz  kbnocountry
	mov  word [keyboard],dx
	ret
   kbnocountry:
	mov  [esp+32],dword 1
	ret
   nsyse2:
	dec  ebx			    ; CD
	jnz  nsyse4

	test ecx,ecx
	jz   nosesl

	cmp  ecx, 4
	ja   nosesl
	mov  [cd_base],cl

	dec	ecx
	jnz  noprma
	mov  [cdbase],0x1f0
	mov  [cdid],0xa0
   noprma:

	dec	ecx
	jnz  noprsl
	mov  [cdbase],0x1f0
	mov  [cdid],0xb0
   noprsl:
	dec	ecx
	jnz  nosema
	mov  [cdbase],0x170
	mov  [cdid],0xa0
   nosema:
	dec	ecx
	jnz  nosesl
	mov  [cdbase],0x170
	mov  [cdid],0xb0
   nosesl:
	ret

iglobal
cd_base db 0

endg
   nsyse4:

	sub  ebx,2		 ; SYSTEM LANGUAGE
	jnz  nsyse5
	mov  [syslang],ecx
	ret
   nsyse5:

	sub  ebx,2		; HD BASE
	jnz  nsyse7

	test ecx,ecx
	jz   nosethd

	cmp  ecx,4
	ja   nosethd
	mov  [hd_base],cl

	cmp  ecx,1
	jnz  noprmahd
	mov  [hdbase],0x1f0
	and  dword [hdid],0x0
	mov  dword [hdpos],ecx
;     call set_FAT32_variables
   noprmahd:

	cmp  ecx,2
	jnz  noprslhd
	mov  [hdbase],0x1f0
	mov  [hdid],0x10
	mov  dword [hdpos],ecx
;     call set_FAT32_variables
   noprslhd:

	cmp  ecx,3
	jnz  nosemahd
	mov  [hdbase],0x170
	and  dword [hdid],0x0
	mov  dword [hdpos],ecx
;     call set_FAT32_variables
   nosemahd:

	cmp  ecx,4
	jnz  noseslhd
	mov  [hdbase],0x170
	mov  [hdid],0x10
	mov  dword [hdpos],ecx
;     call set_FAT32_variables
   noseslhd:
	call  reserve_hd1
	call  reserve_hd_channel
	call  free_hd_channel
	and   dword [hd1_status],0	  ; free
   nosethd:
	ret

iglobal
hd_base db 0
endg

nsyse7:

;     cmp  eax,8                      ; HD PARTITION
	dec  ebx
	jnz  nsyse8
	mov  [fat32part],ecx
;     call set_FAT32_variables
	call  reserve_hd1
	call  reserve_hd_channel
	call  free_hd_channel
;       pusha
	call  choice_necessity_partition_1
;       popa
	and dword [hd1_status],0	; free
	ret

nsyse8:
;     cmp  eax,11                     ; ENABLE LBA READ
	and  ecx,1
	sub  ebx,3
	jnz  no_set_lba_read
	mov  [lba_read_enabled],ecx
	ret

no_set_lba_read:
;     cmp  eax,12                     ; ENABLE PCI ACCESS
	dec  ebx
	jnz  no_set_pci_access
	mov  [pci_access_enabled],ecx
	ret
no_set_pci_access:

;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
include 'vmodeint.inc'
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

sys_setup_err:
	or  [esp+32],dword -1
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
	dec	ebx
	jnz  ngsyse1
	movzx eax,[midi_base]
	mov  [esp+32],eax
	ret
ngsyse1:
;     cmp  eax,2
	dec	ebx
	jnz  ngsyse2

	mov  edi,[TASK_BASE]
	mov  ebx,[edi+TASKDATA.mem_start]
	add  ebx,edx

;     cmp  ebx,1
	dec	ecx
	jnz  kbnobaseret
	mov  eax,keymap
	mov  ecx,128
	call memmove
	ret
kbnobaseret:
;     cmp  ebx,2
	dec	ecx
	jnz  kbnoshiftret

	mov  eax,keymap_shift
	mov  ecx,128
	call memmove
	ret
kbnoshiftret:
;     cmp  ebx,3
	dec	ecx
	jne  kbnoaltret

	mov  eax,keymap_alt
	mov  ecx,128
	call memmove
	ret
kbnoaltret:
;     cmp  ebx,9
	sub	ecx,6
	jnz  ngsyse2
	movzx eax,word [keyboard]
	mov  [esp+32],eax
	ret


ngsyse2:
;         cmp  eax,3
	dec	ebx
	jnz  ngsyse3
	movzx eax,[cd_base]
	mov  [esp+32],eax
	ret
ngsyse3:
;         cmp  eax,5
	sub	ebx,2
	jnz  ngsyse5
	mov  eax,[syslang]
	mov  [esp+32],eax
	ret
ngsyse5:
;     cmp  eax,7
	sub	ebx,2
	jnz  ngsyse7
	movzx eax,[hd_base]
	mov  [esp+32],eax
	ret
ngsyse7:
;     cmp  eax,8
	dec	ebx
	jnz  ngsyse8
	mov eax,[fat32part]
	mov  [esp+32],eax
	ret
ngsyse8:
;     cmp  eax,9
	dec	ebx
	jnz  ngsyse9
	mov  eax,[timer_ticks] ;[0xfdf0]
	mov  [esp+32],eax
	ret
ngsyse9:
;     cmp  eax,11
	sub	ebx,2
	jnz  ngsyse11
	mov eax,[lba_read_enabled]
	mov  [esp+32],eax
	ret
ngsyse11:
;     cmp  eax,12
	dec	ebx
	jnz  ngsyse12
	mov eax,[pci_access_enabled]
	mov  [esp+32],eax
	ret
ngsyse12:
	mov  [esp+32],dword 1
	ret


get_timer_ticks:
	mov eax,[timer_ticks]
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

	   cmp ebx, 7
	   ja msset
	   jmp [mousefn+ebx*4]
msscreen:
	   mov	eax,[MOUSE_X]
	   shl	eax,16
	   mov	ax,[MOUSE_Y]
	   mov	[esp+36-4],eax
	   ret
mswin:
	   mov	eax,[MOUSE_X]
	   shl	eax,16
	   mov	ax,[MOUSE_Y]
	   mov	esi,[TASK_BASE]
	   mov	bx, word [esi-twdw+WDATA.box.left]
	   shl	ebx,16
	   mov	bx, word [esi-twdw+WDATA.box.top]
	   sub	eax,ebx

	   mov	edi,[CURRENT_TASK]
	   shl	edi,8
	   sub	ax,word[edi+SLOT_BASE+APPDATA.wnd_clientbox.top]
	   rol	eax,16
	   sub	ax,word[edi+SLOT_BASE+APPDATA.wnd_clientbox.left]
	   rol	eax,16
	   mov	[esp+36-4],eax
	   ret
msbutton:
	   movzx eax,byte [BTN_DOWN]
	   mov	[esp+36-4],eax
	   ret
msz:
	   mov	 edi, [TASK_COUNT]
	   movzx edi, word [WIN_POS + edi*2]
	   cmp	 edi, [CURRENT_TASK]
	   jne	 @f
	   mov	 ax,[MOUSE_SCROLL_H]
	   shl	 eax,16
	   mov	 ax,[MOUSE_SCROLL_V]
	   mov	 [esp+36-4],eax
	   and	 [MOUSE_SCROLL_H],word 0
	   and	 [MOUSE_SCROLL_V],word 0
	   ret
       @@:
	   and	[esp+36-4],dword 0
;           ret
msset:
	   ret

app_load_cursor:
	   cmp ecx, OS_BASE
	   jae msset
	   stdcall load_cursor, ecx, edx
	   mov [esp+36-4], eax
	   ret

app_set_cursor:
	   stdcall set_cursor, ecx
	   mov [esp+36-4], eax
	   ret

app_delete_cursor:
	   stdcall delete_cursor, ecx
	   mov [esp+36-4], eax
	   ret

is_input:

   push edx
   mov	dx,word [midisp]
   in	al,dx
   and	al,0x80
   pop	edx
   ret

is_output:

   push edx
   mov	dx,word [midisp]
   in	al,dx
   and	al,0x40
   pop	edx
   ret


get_mpu_in:

   push edx
   mov	dx,word [mididp]
   in	al,dx
   pop	edx
   ret


put_mpu_out:

   push edx
   mov	dx,word [mididp]
   out	dx,al
   pop	edx
   ret



align 4

sys_midi:
	cmp  [mididp],0
	jnz  sm0
	mov  [esp+36],dword 1
	ret
sm0:
	and  [esp+36],dword 0
	dec  ebx
	jnz  smn1
 ;    call setuart
su1:
	call is_output
	test al,al
	jnz  su1
	mov  dx,word [midisp]
	mov  al,0xff
	out  dx,al
su2:
	mov  dx,word [midisp]
	mov  al,0xff
	out  dx,al
	call is_input
	test al,al
	jnz  su2
	call get_mpu_in
	cmp  al,0xfe
	jnz  su2
su3:
	call is_output
	test  al,al
	jnz  su3
	mov  dx,word [midisp]
	mov  al,0x3f
	out  dx,al
	ret
smn1:
	dec  ebx
	jnz  smn2
sm10:
	call get_mpu_in
	call is_output
	test al,al
	jnz  sm10
	mov  al,bl
	call put_mpu_out
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

     mov ecx, [current_slot]
     mov eax, [ecx+APPDATA.tls_base]
     test eax, eax
     jz @F

     stdcall user_free, eax
@@:

     mov   eax,[TASK_BASE]
     mov   [eax+TASKDATA.state], 3  ; terminate this program

    waitterm:		 ; wait here for termination
     mov   ebx,100
     call  delay_hs
     jmp   waitterm

iglobal
align 4
sys_system_table:
	dd	exit_for_anyone 	; 1 = obsolete
	dd	sysfn_terminate 	; 2 = terminate thread
	dd	sysfn_activate		; 3 = activate window
	dd	sysfn_getidletime	; 4 = get idle time
	dd	sysfn_getcpuclock	; 5 = get cpu clock
	dd	sysfn_saveramdisk	; 6 = save ramdisk
	dd	sysfn_getactive 	; 7 = get active window
	dd	sysfn_sound_flag	; 8 = get/set sound_flag
	dd	sysfn_shutdown		; 9 = shutdown with parameter
	dd	sysfn_minimize		; 10 = minimize window
	dd	sysfn_getdiskinfo	; 11 = get disk subsystem info
	dd	sysfn_lastkey		; 12 = get last pressed key
	dd	sysfn_getversion	; 13 = get kernel version
	dd	sysfn_waitretrace	; 14 = wait retrace
	dd	sysfn_centermouse	; 15 = center mouse cursor
	dd	sysfn_getfreemem	; 16 = get free memory size
	dd	sysfn_getallmem 	; 17 = get total memory size
	dd	sysfn_terminate2	; 18 = terminate thread using PID
					;                 instead of slot
	dd	sysfn_mouse_acceleration; 19 = set/get mouse acceleration
	dd	sysfn_meminfo		; 20 = get extended memory info
	dd	sysfn_pid_to_slot	; 21 = get slot number for pid
	dd	sysfn_min_rest_window	; 22 = minimize and restore any window
sysfn_num = ($ - sys_system_table)/4
endg

sys_system:
	dec	ebx
	cmp	ebx, sysfn_num
	jae	@f
	jmp	dword [sys_system_table + ebx*4]
@@:
	ret


sysfn_shutdown: 	 ; 18.9 = system shutdown
     cmp  ecx,1
     jl   exit_for_anyone
     cmp  ecx,4
     jg   exit_for_anyone
     mov  [BOOT_VAR+0x9030],cl

     mov  eax,[TASK_COUNT]
     mov  [SYS_SHUTDOWN],al
     mov  [shutdown_processes],eax
     and  dword [esp+32], 0
 exit_for_anyone:
     ret
  uglobal
   shutdown_processes: dd 0x0
  endg

sysfn_terminate:	; 18.2 = TERMINATE
     cmp  ecx,2
     jb   noprocessterminate
     mov  edx,[TASK_COUNT]
     cmp  ecx,edx
     ja   noprocessterminate
     mov  eax,[TASK_COUNT]
     shl  ecx,5
     mov  edx,[ecx+CURRENT_TASK+TASKDATA.pid]
     add  ecx,CURRENT_TASK+TASKDATA.state
     cmp  byte [ecx], 9
     jz   noprocessterminate

     ;call MEM_Heap_Lock      ;guarantee that process isn't working with heap
     mov  [ecx],byte 3	     ; clear possible i40's
     ;call MEM_Heap_UnLock

     cmp  edx,[application_table_status]    ; clear app table stat
     jne  noatsc

     and  [application_table_status],0
   noatsc:
   noprocessterminate:
     ret

sysfn_terminate2:
;lock application_table_status mutex
.table_status:
    cli
    cmp    [application_table_status],0
    je	   .stf
    sti
    call   change_task
    jmp    .table_status
.stf:
    call   set_application_table_status
    mov    eax,ecx
    call   pid_to_slot
    test   eax,eax
    jz	   .not_found
    mov    ecx,eax
    cli
    call   sysfn_terminate
    and    [application_table_status],0
    sti
    and    dword [esp+32],0
    ret
.not_found:
    mov    [application_table_status],0
    or	   dword [esp+32],-1
    ret

sysfn_activate: 	; 18.3 = ACTIVATE WINDOW
     cmp  ecx,2
     jb   .nowindowactivate
     cmp  ecx,[TASK_COUNT]
     ja   .nowindowactivate

     mov   [window_minimize], 2   ; restore window if minimized

     movzx esi, word [WIN_STACK + ecx*2]
     cmp   esi, [TASK_COUNT]
     je    .nowindowactivate ; already active

     mov   edi, ecx
     shl   edi, 5
     add   edi, window_data
     movzx esi, word [WIN_STACK + ecx * 2]
     lea   esi, [WIN_POS + esi * 2]
     call  waredraw
.nowindowactivate:
     ret

sysfn_getidletime:		; 18.4 = GET IDLETIME
     mov  eax,[idleusesec]
     mov  [esp+32], eax
     ret

sysfn_getcpuclock:		; 18.5 = GET TSC/SEC
     mov  eax,[CPU_FREQ]
     mov  [esp+32], eax
     ret

;  SAVE ramdisk to /hd/1/menuet.img
;!!!!!!!!!!!!!!!!!!!!!!!!
   include 'blkdev/rdsave.inc'
;!!!!!!!!!!!!!!!!!!!!!!!!
align 4
sysfn_getactive:	; 18.7 = get active window
     mov  eax, [TASK_COUNT]
   movzx  eax, word [WIN_POS + eax*2]
     mov  [esp+32],eax
     ret

sysfn_sound_flag:	; 18.8 = get/set sound_flag
;     cmp  ecx,1
     dec  ecx
     jnz  nogetsoundflag
     movzx  eax,byte [sound_flag] ; get sound_flag
     mov  [esp+32],eax
     ret
 nogetsoundflag:
;     cmp  ecx,2
     dec  ecx
     jnz  nosoundflag
     xor  byte [sound_flag], 1
 nosoundflag:
     ret

sysfn_minimize: 	; 18.10 = minimize window
     mov   [window_minimize],1
     ret
align 4
sysfn_getdiskinfo:	; 18.11 = get disk info table
;     cmp  ecx,1
     dec  ecx
     jnz  full_table
  small_table:
     call for_all_tables
     mov ecx,10
     cld
     rep movsb
     ret
   for_all_tables:
     mov edi,edx
     mov esi,DRIVE_DATA
     ret
  full_table:
;     cmp  ecx,2
     dec  ecx
     jnz  exit_for_anyone
     call for_all_tables
     mov ecx,16384
     cld
     rep movsd
     ret

sysfn_lastkey:		; 18.12 = return 0 (backward compatibility)
	and	dword [esp+32], 0
	ret

sysfn_getversion:	; 18.13 = get kernel ID and version
     mov edi,ecx
     mov esi,version_inf
     mov ecx,version_end-version_inf
     rep movsb
     ret

sysfn_waitretrace:     ; 18.14 = sys wait retrace
     ;wait retrace functions
 sys_wait_retrace:
     mov edx,0x3da
 WaitRetrace_loop:
     in al,dx
     test al,1000b
     jz WaitRetrace_loop
     and [esp+32],dword 0
     ret

align 4
sysfn_centermouse:	; 18.15 = mouse centered
; removed here by <Lrz>
;     call  mouse_centered
;* mouse centered - start code- Mario79
;mouse_centered:
;        push  eax
	mov   eax,[Screen_Max_X]
	shr   eax,1
	mov   [MOUSE_X],ax
	mov   eax,[Screen_Max_Y]
	shr   eax,1
	mov   [MOUSE_Y],ax
;        ret
;* mouse centered - end code- Mario79
	xor   eax,eax
	and   [esp+32],eax
;        pop   eax

     ret
align 4
sysfn_mouse_acceleration: ; 18.19 = set/get mouse features
     test ecx,ecx  ; get mouse speed factor
     jnz  .set_mouse_acceleration
     xor  eax,eax
     mov  ax,[mouse_speed_factor]
     mov  [esp+32],eax
     ret
 .set_mouse_acceleration:
;     cmp  ecx,1  ; set mouse speed factor
     dec  ecx
     jnz  .get_mouse_delay
     mov  [mouse_speed_factor],dx
     ret
 .get_mouse_delay:
;     cmp  ecx,2  ; get mouse delay
     dec  ecx
     jnz  .set_mouse_delay
     mov  eax,[mouse_delay]
     mov  [esp+32],eax
     ret
 .set_mouse_delay:
;     cmp  ecx,3  ; set mouse delay
     dec  ecx
     jnz  .set_pointer_position
     mov  [mouse_delay],edx
     ret
 .set_pointer_position:
;     cmp  ecx,4  ; set mouse pointer position
     dec   ecx
     jnz  .set_mouse_button
     mov   [MOUSE_Y],dx    ;y
     ror   edx,16
     mov   [MOUSE_X],dx    ;x
     rol   edx,16
     ret
 .set_mouse_button:
;     cmp   ecx,5  ; set mouse button features
     dec   ecx
     jnz  .end
     mov   [BTN_DOWN],dl
     mov   [mouse_active],1
 .end:
     ret

sysfn_getfreemem:
     mov eax, [pg_data.pages_free]
     shl eax, 2
     mov [esp+32],eax
     ret

sysfn_getallmem:
     mov  eax,[MEM_AMOUNT]
     shr eax, 10
     mov  [esp+32],eax
     ret

; // Alver, 2007-22-08 // {
sysfn_pid_to_slot:
     mov   eax, ecx
     call  pid_to_slot
     mov   [esp+32], eax
     ret

sysfn_min_rest_window:
     pushad
     mov   eax, edx	 ; ebx - operating
     shr   ecx, 1
     jnc    @f
     call  pid_to_slot
@@:
     or    eax, eax	 ; eax - number of slot
     jz    .error
     cmp   eax, 255	    ; varify maximal slot number
     ja    .error
     movzx eax, word [WIN_STACK + eax*2]
     shr   ecx, 1
     jc    .restore
 ; .minimize:
     call  minimize_window
     jmp   .exit
.restore:
     call  restore_minimized_window
.exit:
     popad
     xor   eax, eax
     mov   [esp+32], eax
     ret
.error:
     popad
     xor   eax, eax
     dec   eax
     mov   [esp+32], eax
     ret
; } \\ Alver, 2007-22-08 \\

uglobal
;// mike.dld, 2006-29-01 [
screen_workarea RECT
;// mike.dld, 2006-29-01 ]
window_minimize db 0
sound_flag	db 0
endg

iglobal
version_inf:
  db 0,7,7,0  ; version 0.7.7.0
  db UID_KOLIBRI
  dd __REV__
version_end:
endg

UID_NONE=0
UID_MENUETOS=1	 ;official
UID_KOLIBRI=2	 ;russian

sys_cachetodiskette:
	cmp	ebx, 1
	jne	.no_floppy_a_save
	mov	[flp_number], 1
	jmp	.save_image_on_floppy
.no_floppy_a_save:
	cmp	ebx, 2
	jne	.no_floppy_b_save
	mov	[flp_number], 2
.save_image_on_floppy:
	call	save_image
	mov	[esp + 32], dword 0
	cmp	[FDC_Status], 0
	je	.yes_floppy_save
.no_floppy_b_save:
	mov	[esp + 32], dword 1
.yes_floppy_save:
	ret

uglobal
;  bgrchanged  dd  0x0
align 4
bgrlockpid dd 0
bgrlock db 0
endg

sys_background:

    cmp   ebx,1 			   ; BACKGROUND SIZE
    jnz   nosb1
    test  ecx,ecx
;    cmp   ecx,0
    jz	  sbgrr
    test  edx,edx
;    cmp   edx,0
    jz	  sbgrr
@@:
;;Maxis use atomic bts for mutexes  4.4.2009
	bts	dword [bgrlock], 0
	jnc	@f
	call	change_task
	jmp	@b
@@:
    mov   [BgrDataWidth],ecx
    mov   [BgrDataHeight],edx
;    mov   [bgrchanged],1

    pushad
; return memory for old background
	mov	eax, [img_background]
	cmp	eax, static_background_data
	jz	@f
	stdcall kernel_free, eax
@@:
; calculate RAW size
    xor  eax,eax
    inc  eax
    cmp  [BgrDataWidth],eax
    jae   @f
    mov [BgrDataWidth],eax
@@:
    cmp  [BgrDataHeight],eax
    jae   @f
    mov [BgrDataHeight],eax
@@:
    mov  eax,[BgrDataWidth]
    imul eax,[BgrDataHeight]
    lea  eax,[eax*3]
    mov  [mem_BACKGROUND],eax
; get memory for new background
    stdcall kernel_alloc, eax
    test eax, eax
    jz .memfailed
    mov [img_background], eax
    jmp .exit
.memfailed:
; revert to static monotone data
	mov	[img_background], static_background_data
	xor	eax, eax
	inc	eax
	mov	[BgrDataWidth], eax
	mov	[BgrDataHeight], eax
	mov	[mem_BACKGROUND], 4
.exit:
    popad
	mov	[bgrlock], 0

  sbgrr:
    ret

nosb1:

    cmp   ebx,2 			   ; SET PIXEL
    jnz   nosb2

    mov   eax, [img_background]
    test  ecx, ecx
    jz	  @f
    cmp   eax, static_background_data
    jz	  .ret
@@:
    mov ebx, [mem_BACKGROUND]
    add ebx, 4095
    and ebx, -4096
    sub ebx, 4
    cmp   ecx, ebx
    ja	 .ret

    mov   ebx,[eax+ecx]
    and   ebx,0xFF000000 ;255*256*256*256
    and   edx,0x00FFFFFF ;255*256*256+255*256+255
    add   edx,ebx
    mov   [eax+ecx],edx
.ret:
    ret
nosb2:

    cmp   ebx,3 			   ; DRAW BACKGROUND
    jnz   nosb3
draw_background_temp:
;    cmp   [bgrchanged],1 ;0
;    je    nosb31
;draw_background_temp:
;    mov   [bgrchanged],1 ;0
    mov    [background_defined], 1
    mov    byte[BACKGROUND_CHANGED], 1
    call  force_redraw_background
   nosb31:
    ret
  nosb3:

    cmp   ebx,4 			   ; TILED / STRETCHED
    jnz   nosb4
    cmp   ecx,[BgrDrawMode]
    je	  nosb41
    mov   [BgrDrawMode],ecx
;    mov   [bgrchanged],1
   nosb41:
    ret
  nosb4:

    cmp   ebx,5 			   ; BLOCK MOVE TO BGR
    jnz   nosb5
    cmp   [img_background], static_background_data
    jnz   @f
    test  edx, edx
    jnz   .fin
    cmp   esi, 4
    ja	  .fin
  @@:
  ; bughere
    mov   eax, ecx
    mov   ebx, edx
    add   ebx, [img_background]   ;IMG_BACKGROUND
    mov   ecx, esi
    call  memmove
  .fin:
    ret
  nosb5:

	cmp	ebx, 6
	jnz	nosb6
;;Maxis use atomic bts for mutex 4.4.2009
@@:
	bts	dword [bgrlock], 0
	jnc	@f
	call	change_task
	jmp	@b
@@:
	mov	eax, [CURRENT_TASK]
	mov	[bgrlockpid], eax
	cmp	[img_background], static_background_data
	jz	.nomem
	stdcall user_alloc, [mem_BACKGROUND]
	mov	[esp+32], eax
	test	eax, eax
	jz	.nomem
	mov	ebx, eax
	shr	ebx, 12
	or	dword [page_tabs+(ebx-1)*4], DONT_FREE_BLOCK
	mov	esi, [img_background]
	shr	esi, 12
	mov	ecx, [mem_BACKGROUND]
	add	ecx, 0xFFF
	shr	ecx, 12
.z:
	mov	eax, [page_tabs+ebx*4]
	test	al, 1
	jz	@f
	call	free_page
@@:
	mov	eax, [page_tabs+esi*4]
	or	al, PG_UW
	mov	[page_tabs+ebx*4], eax
	mov	eax, ebx
	shl	eax, 12
	invlpg	[eax]
	inc	ebx
	inc	esi
	loop	.z
	ret
.nomem:
	and	[bgrlockpid], 0
	mov	[bgrlock], 0
nosb6:
	cmp	ebx, 7
	jnz	nosb7
	cmp	[bgrlock], 0
	jz	.err
	mov	eax, [CURRENT_TASK]
	cmp	[bgrlockpid], eax
	jnz	.err
	mov	eax, ecx
	mov	ebx, ecx
	shr	eax, 12
	mov	ecx, [page_tabs+(eax-1)*4]
	test	cl, USED_BLOCK+DONT_FREE_BLOCK
	jz	.err
	jnp	.err
	push	eax
	shr	ecx, 12
	dec	ecx
@@:
	and	dword [page_tabs+eax*4], 0
	mov	edx, eax
	shl	edx, 12
	push eax
	invlpg	[edx]
	pop eax
	inc	eax
	loop	@b
	pop	eax
	and	dword [page_tabs+(eax-1)*4], not DONT_FREE_BLOCK
	stdcall user_free, ebx
	mov	[esp+32], eax
	and	[bgrlockpid], 0
	mov	[bgrlock], 0
	ret
.err:
	and	dword [esp+32], 0
	ret

nosb7:
    ret

force_redraw_background:
    and   [draw_data+32 + RECT.left], 0
    and   [draw_data+32 + RECT.top], 0
    push  eax ebx
    mov   eax,[Screen_Max_X]
    mov   ebx,[Screen_Max_Y]
    mov   [draw_data+32 + RECT.right],eax
    mov   [draw_data+32 + RECT.bottom],ebx
    pop   ebx eax
    inc   byte[REDRAW_BACKGROUND]
    ret

align 4

sys_getbackground:
;    cmp   eax,1                                  ; SIZE
    dec   ebx
    jnz   nogb1
    mov   eax,[BgrDataWidth]
    shl   eax,16
    mov   ax,[BgrDataHeight]
    mov   [esp+32],eax
    ret

nogb1:
;    cmp   eax,2                                  ; PIXEL
    dec   ebx
    jnz   nogb2

	mov	eax, [img_background]
	test	ecx, ecx
	jz	@f
	cmp	eax, static_background_data
	jz	.ret
@@:
    mov ebx, [mem_BACKGROUND]
    add ebx, 4095
    and ebx, -4096
    sub ebx, 4
    cmp ecx, ebx
    ja	.ret

    mov   eax,[ecx+eax]

    and   eax, 0xFFFFFF
    mov   [esp+32],eax
.ret:
    ret
  nogb2:

;    cmp   eax,4                                  ; TILED / STRETCHED
    dec   ebx
    dec   ebx
    jnz   nogb4
    mov   eax,[BgrDrawMode]
  nogb4:
    mov   [esp+32],eax
    ret

align 4

sys_getkey:
	mov	[esp + 32],dword 1
	; test main buffer
	mov	ebx, [CURRENT_TASK]			     ; TOP OF WINDOW STACK
	movzx	ecx, word [WIN_STACK + ebx * 2]
	mov	edx, [TASK_COUNT]
	cmp	ecx, edx
	jne	.finish
	cmp	[KEY_COUNT], byte 0
	je	.finish
	movzx	eax, byte [KEY_BUFF]
	shl	eax, 8
	push	eax
	dec	byte [KEY_COUNT]
	and	byte [KEY_COUNT], 127
	movzx	ecx, byte [KEY_COUNT]
	add	ecx, 2
	mov	eax, KEY_BUFF + 1
	mov	ebx, KEY_BUFF
	call	memmove
	pop	eax
.ret_eax:
	mov	[esp + 32], eax
	ret
.finish:
; test hotkeys buffer
	mov	ecx, hotkey_buffer
@@:
	cmp	[ecx], ebx
	jz	.found
	add	ecx, 8
	cmp	ecx, hotkey_buffer + 120 * 8
	jb	@b
	ret
.found:
	mov	ax, [ecx + 6]
	shl	eax, 16
	mov	ah, [ecx + 4]
	mov	al, 2
	and	dword [ecx + 4], 0
	and	dword [ecx], 0
	jmp	.ret_eax

align 4

sys_getbutton:

	mov	ebx, [CURRENT_TASK]			    ; TOP OF WINDOW STACK
	mov	[esp + 32], dword 1
	movzx	ecx, word [WIN_STACK + ebx * 2]
	mov	edx, [TASK_COUNT] ; less than 256 processes
	cmp	ecx, edx
	jne	.exit
	movzx	eax, byte [BTN_COUNT]
	test	eax, eax
	jz	.exit
	mov	eax, [BTN_BUFF]
	and	al, 0xFE				    ; delete left button bit
	mov	[BTN_COUNT], byte 0
	mov	[esp + 32], eax
.exit:
	ret


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

    cmp  ecx,-1 	; who am I ?
    jne  .no_who_am_i
    mov  ecx,[CURRENT_TASK]
  .no_who_am_i:
	cmp	ecx, max_processes
	ja	.nofillbuf

; +4: word: position of the window of thread in the window stack
	mov	ax, [WIN_STACK + ecx * 2]
	mov	[ebx+4], ax
; +6: word: number of the thread slot, which window has in the window stack
;           position ecx (has no relation to the specific thread)
	mov	ax, [WIN_POS + ecx * 2]
	mov	[ebx+6], ax

	shl	ecx, 5

; +0: dword: memory usage
	mov	eax, [ecx+CURRENT_TASK+TASKDATA.cpu_usage]
	mov	[ebx], eax
; +10: 11 bytes: name of the process
	push	ecx
	lea	eax, [ecx*8+SLOT_BASE+APPDATA.app_name]
	add	ebx, 10
	mov	ecx, 11
	call	memmove
	pop	ecx

; +22: address of the process in memory
; +26: size of used memory - 1
	push	edi
	lea	edi, [ebx+12]
	xor	eax, eax
	mov	edx, 0x100000*16
	cmp	ecx, 1 shl 5
	je	.os_mem
	mov	edx, [SLOT_BASE+ecx*8+APPDATA.mem_size]
	mov	eax, std_application_base_address
.os_mem:
	stosd
	lea	eax, [edx-1]
	stosd

; +30: PID/TID
	mov	eax, [ecx+CURRENT_TASK+TASKDATA.pid]
	stosd

    ; window position and size
	push	esi
	lea	esi, [ecx + window_data + WDATA.box]
	movsd
	movsd
	movsd
	movsd

    ; Process state (+50)
	mov	eax, dword [ecx+CURRENT_TASK+TASKDATA.state]
	stosd

    ; Window client area box
	lea	esi, [ecx*8 + SLOT_BASE + APPDATA.wnd_clientbox]
	movsd
	movsd
	movsd
	movsd

    ; Window state
	mov	al, [ecx+window_data+WDATA.fl_wstate]
	stosb

	pop	esi
	pop	edi

.nofillbuf:
    ; return number of processes

    mov    eax,[TASK_COUNT]
    mov    [esp+32],eax
    ret

align 4
sys_clock:
	cli
  ; Mikhail Lisovin  xx Jan 2005
  @@:	mov   al, 10
	out   0x70, al
	in    al, 0x71
	test  al, al
	jns   @f
	mov   esi, 1
	call  delay_ms
	jmp   @b
  @@:
  ; end Lisovin's fix

	xor   al,al	      ; seconds
	out   0x70,al
	in    al,0x71
	movzx ecx,al
	mov   al,02	      ; minutes
	shl   ecx,16
	out   0x70,al
	in    al,0x71
	movzx edx,al
	mov   al,04	      ; hours
	shl   edx,8
	out   0x70,al
	in    al,0x71
	add   ecx,edx
	movzx edx,al
	add   ecx,edx
	sti
	mov	[esp + 32], ecx
	ret


align 4

sys_date:

	cli
  @@:	mov   al, 10
	out   0x70, al
	in    al, 0x71
	test  al, al
	jns   @f
	mov   esi, 1
	call  delay_ms
	jmp   @b
  @@:

	mov	ch,0
	mov	al,7		; date
	out	0x70,al
	in	al,0x71
	mov	cl,al
	mov	al,8		; month
	shl	ecx,16
	out	0x70,al
	in	al,0x71
	mov	ch,al
	mov	al,9		; year
	out	0x70,al
	in	al,0x71
	mov	cl,al
	sti
	mov	[esp+32], ecx
	ret


; redraw status

sys_redrawstat:
	cmp	ebx, 1
	jne	no_widgets_away
	; buttons away
	mov	ecx,[CURRENT_TASK]
  sys_newba2:
	mov	edi,[BTN_ADDR]
	cmp	[edi], dword 0	; empty button list ?
	je	end_of_buttons_away
	movzx	ebx, word [edi]
	inc	ebx
	mov	eax,edi
  sys_newba:
	dec	ebx
	jz	end_of_buttons_away

	add	eax, 0x10
	cmp	cx, [eax]
	jnz	sys_newba

	push	eax ebx ecx
	mov	ecx,ebx
	inc	ecx
	shl	ecx, 4
	mov	ebx, eax
	add	eax, 0x10
	call	memmove
	dec	dword [edi]
	pop	ecx ebx eax

	jmp	sys_newba2

  end_of_buttons_away:

	ret

  no_widgets_away:

	cmp	ebx, 2
	jnz	srl1

	mov	edx, [TASK_BASE]      ; return whole screen draw area for this app
	add	edx, draw_data - CURRENT_TASK
	mov	[edx + RECT.left], 0
	mov	[edx + RECT.top], 0
	mov	eax, [Screen_Max_X]
	mov	[edx + RECT.right], eax
	mov	eax, [Screen_Max_Y]
	mov	[edx + RECT.bottom], eax

	mov	edi, [TASK_BASE]
	or	[edi - twdw + WDATA.fl_wdrawn], 1   ; no new position & buttons from app
	call	sys_window_mouse
	ret

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
	dd	sys_sheduler.00
	dd	change_task
	dd	sys_sheduler.02
	dd	sys_sheduler.03
	dd	sys_sheduler.04
endg
sys_sheduler:
;rewritten by <Lrz>  29.12.2009
	jmp	dword [sheduler+ebx*4]
;.shed_counter:
.00:
	mov eax,[context_counter]
	mov [esp+32],eax
	ret

.02:
;.perf_control:
	inc	ebx			;before ebx=2, ebx=3
	cmp	ebx,ecx 		;if ecx=3, ebx=3
	jz	cache_disable

	dec	ebx			;ebx=2
	cmp	ebx,ecx 		;
	jz	cache_enable		;if ecx=2 and ebx=2

	dec	ebx			;ebx=1
	cmp	ebx,ecx
	jz	is_cache_enabled	;if ecx=1 and ebx=1

	dec	ebx
	test	ebx,ecx 		;ebx=0 and ecx=0
	jz	modify_pce		;if ecx=0

	ret

.03:
;.rdmsr_instr:
;now counter in ecx
;(edx:eax) esi:edi => edx:esi
	mov	eax,esi
	mov	ecx,edx
	rdmsr
	mov	[esp+32],eax
	mov	[esp+20],edx		;ret in ebx?
	ret

.04:
;.wrmsr_instr:
;now counter in ecx
;(edx:eax) esi:edi => edx:esi
	; Fast Call MSR can't be destroy
	; Но MSR_AMD_EFER можно изменять, т.к. в этом регистре лиш
	; включаются/выключаются расширенные возможности
	cmp	edx,MSR_SYSENTER_CS
	je	@f
	cmp	edx,MSR_SYSENTER_ESP
	je	@f
	cmp	edx,MSR_SYSENTER_EIP
	je	@f
	cmp	edx,MSR_AMD_STAR
	je	@f

	mov	eax,esi
	mov	ecx,edx
	wrmsr
	; mov   [esp + 32], eax
	; mov   [esp + 20], edx ;ret in ebx?
@@:
	ret

cache_disable:
       mov eax,cr0
       or  eax,01100000000000000000000000000000b
       mov cr0,eax
       wbinvd ;set MESI
ret

cache_enable:
       mov eax,cr0
       and eax,10011111111111111111111111111111b
       mov cr0,eax
ret

is_cache_enabled:
       mov eax,cr0
       mov ebx,eax
       and eax,01100000000000000000000000000000b
       jz cache_disabled
       mov [esp+32],ebx
cache_disabled:
       mov dword [esp+32],eax ;0
ret

modify_pce:
       mov eax,cr4
;       mov ebx,0
;       or  bx,100000000b ;pce
;       xor eax,ebx ;invert pce
       bts eax,8 ;pce=cr4[8]
       mov cr4,eax
       mov [esp+32],eax
ret
;---------------------------------------------------------------------------------------------


; check if pixel is allowed to be drawn

checkpixel:
	push eax edx

	mov  edx,[Screen_Max_X]     ; screen x size
	inc  edx
	imul edx, ebx
	add  eax, [_WinMapAddress]
	mov  dl, [eax+edx] ; lea eax, [...]

	xor  ecx, ecx
	mov  eax, [CURRENT_TASK]
	cmp  al, dl
	setne cl

	pop  edx eax
	ret

iglobal
  cpustring db 'CPU',0
endg

uglobal
background_defined    db    0	 ; diamond, 11.04.2006
endg

align 4
; check misc

checkmisc:

    cmp   [ctrl_alt_del], 1
    jne   nocpustart

	mov	ebp, cpustring
	call	fs_execute_from_sysdir

    mov   [ctrl_alt_del], 0

nocpustart:
    cmp   [mouse_active], 1
    jne   mouse_not_active
    mov   [mouse_active], 0
    xor   edi, edi
    mov   ecx,	[TASK_COUNT]
set_mouse_event:
    add   edi, 256
    or	  [edi+SLOT_BASE+APPDATA.event_mask], dword 100000b
    loop  set_mouse_event

mouse_not_active:
    cmp   byte[BACKGROUND_CHANGED], 0
    jz	  no_set_bgr_event
    xor   edi, edi
    mov   ecx, [TASK_COUNT]
set_bgr_event:
    add   edi, 256
    or	  [edi+SLOT_BASE+APPDATA.event_mask], 16
    loop  set_bgr_event
    mov   byte[BACKGROUND_CHANGED], 0
no_set_bgr_event:
    cmp   byte[REDRAW_BACKGROUND], 0		   ; background update ?
    jz	  nobackgr
    cmp    [background_defined], 0
    jz	  nobackgr
;    mov   [draw_data+32 + RECT.left],dword 0
;    mov   [draw_data+32 + RECT.top],dword 0
;    mov   eax,[Screen_Max_X]
;    mov   ebx,[Screen_Max_Y]
;    mov   [draw_data+32 + RECT.right],eax
;    mov   [draw_data+32 + RECT.bottom],ebx
@@:
    call  drawbackground
    xor   eax, eax
    xchg  al, [REDRAW_BACKGROUND]
    test  al, al				   ; got new update request?
    jnz   @b
    mov   [draw_data+32 + RECT.left], eax
    mov   [draw_data+32 + RECT.top], eax
    mov   [draw_data+32 + RECT.right], eax
    mov   [draw_data+32 + RECT.bottom], eax
    mov   [MOUSE_BACKGROUND],byte 0

nobackgr:

    ; system shutdown request

    cmp  [SYS_SHUTDOWN],byte 0
    je	 noshutdown

    mov  edx,[shutdown_processes]

    cmp  [SYS_SHUTDOWN],dl
    jne  no_mark_system_shutdown

    lea   ecx,[edx-1]
    mov   edx,OS_BASE+0x3040
    jecxz @f
markz:
    mov   [edx+TASKDATA.state],byte 3
    add   edx,0x20
    loop  markz
@@:

  no_mark_system_shutdown:

    call [_display.disable_mouse]

    dec  byte [SYS_SHUTDOWN]
    je	 system_shutdown

noshutdown:


    mov   eax,[TASK_COUNT]		    ; termination
    mov   ebx,TASK_DATA+TASKDATA.state
    mov   esi,1

newct:
    mov   cl,[ebx]
    cmp   cl,byte 3
    jz	  terminate
    cmp   cl,byte 4
    jz	  terminate

    add   ebx,0x20
    inc   esi
    dec   eax
    jnz   newct
    ret

; redraw screen

redrawscreen:

; eax , if process window_data base is eax, do not set flag/limits

	 pushad
	 push  eax

;;;         mov   ebx,2
;;;         call  delay_hs

	 ;mov   ecx,0               ; redraw flags for apps
	 xor   ecx,ecx
       newdw2:

	 inc   ecx
	 push  ecx

	 mov   eax,ecx
	 shl   eax,5
	 add   eax,window_data

	 cmp   eax,[esp+4]
	 je    not_this_task
				   ; check if window in redraw area
	 mov   edi,eax

	 cmp   ecx,1		   ; limit for background
	 jz    bgli

	 mov   eax, [edi + WDATA.box.left]
	 mov   ebx, [edi + WDATA.box.top]
	 mov   ecx, [edi + WDATA.box.width]
	 mov   edx, [edi + WDATA.box.height]
	 add   ecx,eax
	 add   edx,ebx

	 mov   ecx,[draw_limits.bottom]   ; ecx = area y end     ebx = window y start
	 cmp   ecx,ebx
	 jb    ricino

	 mov   ecx,[draw_limits.right]	 ; ecx = area x end     eax = window x start
	 cmp   ecx,eax
	 jb    ricino

	 mov   eax, [edi + WDATA.box.left]
	 mov   ebx, [edi + WDATA.box.top]
	 mov   ecx, [edi + WDATA.box.width]
	 mov   edx, [edi + WDATA.box.height]
	 add   ecx, eax
	 add   edx, ebx

	 mov   eax,[draw_limits.top]	; eax = area y start     edx = window y end
	 cmp   edx,eax
	 jb    ricino

	 mov   eax,[draw_limits.left]	 ; eax = area x start     ecx = window x end
	 cmp   ecx,eax
	 jb    ricino

	bgli:

	 cmp   dword[esp], 1
	 jnz   .az
;         cmp   byte[BACKGROUND_CHANGED], 0
;         jnz   newdw8
	 cmp   byte[REDRAW_BACKGROUND], 0
	 jz    .az
	 mov   dl, 0
	 lea   eax,[edi+draw_data-window_data]
	 mov   ebx,[draw_limits.left]
	 cmp   ebx,[eax+RECT.left]
	 jae   @f
	 mov   [eax+RECT.left],ebx
	 mov   dl, 1
	@@:
	 mov   ebx,[draw_limits.top]
	 cmp   ebx,[eax+RECT.top]
	 jae   @f
	 mov   [eax+RECT.top],ebx
	 mov   dl, 1
	@@:
	 mov   ebx,[draw_limits.right]
	 cmp   ebx,[eax+RECT.right]
	 jbe   @f
	 mov   [eax+RECT.right],ebx
	 mov   dl, 1
	@@:
	 mov   ebx,[draw_limits.bottom]
	 cmp   ebx,[eax+RECT.bottom]
	 jbe   @f
	 mov   [eax+RECT.bottom],ebx
	 mov   dl, 1
	@@:
	 add   byte[REDRAW_BACKGROUND], dl
	 jmp   newdw8
	.az:

	 mov   eax,edi
	 add   eax,draw_data-window_data

	 mov   ebx,[draw_limits.left]	       ; set limits
	 mov   [eax + RECT.left], ebx
	 mov   ebx,[draw_limits.top]
	 mov   [eax + RECT.top], ebx
	 mov   ebx,[draw_limits.right]
	 mov   [eax + RECT.right], ebx
	 mov   ebx,[draw_limits.bottom]
	 mov   [eax + RECT.bottom], ebx

	 sub   eax,draw_data-window_data

	 cmp   dword [esp],1
	 jne   nobgrd
	 inc   byte[REDRAW_BACKGROUND]

       newdw8:
       nobgrd:

	 mov   [eax + WDATA.fl_redraw],byte 1	 ; mark as redraw

       ricino:

       not_this_task:

	 pop   ecx

	 cmp   ecx,[TASK_COUNT]
	 jle   newdw2

	 pop  eax
	 popad

	 ret

calculatebackground:   ; background

	mov   edi, [_WinMapAddress]		   ; set os to use all pixels
	mov   eax,0x01010101
	mov   ecx, [_WinMapSize]
	shr   ecx, 2
	rep   stosd

	mov   byte[REDRAW_BACKGROUND], 0	      ; do not draw background!
	mov   byte[BACKGROUND_CHANGED], 0

	ret

uglobal
  imax	  dd 0x0
endg



delay_ms:     ; delay in 1/1000 sec


	push  eax
	push  ecx

	mov   ecx,esi
	; <CPU clock fix by Sergey Kuzmin aka Wildwest>
	imul  ecx, 33941
	shr   ecx, 9
	; </CPU clock fix>

	in    al,0x61
	and   al,0x10
	mov   ah,al
	cld

 cnt1:	in    al,0x61
	and   al,0x10
	cmp   al,ah
	jz    cnt1

	mov   ah,al
	loop  cnt1

	pop   ecx
	pop   eax

	ret


set_app_param:
	mov	edi, [TASK_BASE]
	mov	eax, [edi + TASKDATA.event_mask]
	mov	[edi + TASKDATA.event_mask], ebx
	mov	[esp+32], eax
	ret



delay_hs:     ; delay in 1/100 secs
; ebx = delay time
	push  ecx
	push  edx

	mov   edx,[timer_ticks]

      newtic:
	mov   ecx,[timer_ticks]
	sub   ecx,edx
	cmp   ecx,ebx
	jae   zerodelay

	call  change_task

	jmp   newtic

      zerodelay:
	pop   edx
	pop   ecx

	ret

align 16	;very often call this subrutine
memmove:       ; memory move in bytes

; eax = from
; ebx = to
; ecx = no of bytes
    test ecx, ecx
    jle  .ret

    push esi edi ecx

    mov  edi, ebx
    mov  esi, eax

    test ecx, not 11b
    jz	 @f

    push ecx
    shr  ecx, 2
    rep  movsd
    pop  ecx
    and  ecx, 11b
    jz	 .finish
  @@:
    rep  movsb

  .finish:
    pop  ecx edi esi
  .ret:
    ret


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

sys_programirq:
	; removed
    mov   dword [esp+32], 1	; operation failed
    ret


align 4

get_irq_data:
	; removed
     mov   dword [esp+32], -1
     ret


set_io_access_rights:
	;removed
     ret

r_f_port_area:
	; removed; always returns 0
     xor   eax, eax
     ret


reserve_free_irq:

     xor   esi, esi
     inc   esi
     cmp   ecx, 16
     jae   ril1

     push  ecx
     lea   ecx, [irq_owner + 4 * ecx]
     mov   edx, [ecx]
     mov   eax, [TASK_BASE]
     mov   edi, [eax + TASKDATA.pid]
     pop   eax
     dec   ebx
     jnz   reserve_irq

     cmp   edx, edi
     jne   ril1
     dec   esi
     mov   [ecx], esi

     jmp   ril1

  reserve_irq:

     cmp   dword [ecx], 0
     jne   ril1

     mov   ebx, [f_irqs + 4 * eax]

     stdcall attach_int_handler, eax, ebx, dword 0

     mov   [ecx], edi

     dec   esi
   ril1:
     mov   [esp+32], esi ; return in eax
     ret

iglobal
f_irqs:
     dd 0x0
     dd 0x0
     dd p_irq2
     dd p_irq3
     dd p_irq4
     dd p_irq5
     dd p_irq6
     dd p_irq7
     dd p_irq8
     dd p_irq9
     dd p_irq10
     dd p_irq11
     dd 0x0
     dd 0x0
     dd p_irq14
     dd p_irq15

endg

drawbackground:
       inc   [mouse_pause]
       cmp   [SCR_MODE],word 0x12
       je   dbrv20
     dbrv12:
       cmp  [SCR_MODE],word 0100000000000000b
       jge  dbrv20
;       cmp  [SCR_MODE],word 0x13
;       je   dbrv20
;       call  vesa12_drawbackground
       dec   [mouse_pause]
       call   [draw_pointer]
       ret
     dbrv20:
       cmp   [BgrDrawMode],dword 1
       jne   bgrstr
       call  vesa20_drawbackground_tiled
       dec   [mouse_pause]
       call   [draw_pointer]
       ret
     bgrstr:
       call  vesa20_drawbackground_stretch
       dec   [mouse_pause]
       call   [draw_pointer]
       ret

align 4

syscall_putimage:			; PutImage
sys_putimage:
     test  ecx,0x80008000
     jnz   .exit
     test  ecx,0x0000FFFF
     jz    .exit
     test  ecx,0xFFFF0000
     jnz   @f
  .exit:
     ret
 @@:
	mov	edi,[current_slot]
	add	dx,word[edi+APPDATA.wnd_clientbox.top]
	rol	edx,16
	add	dx,word[edi+APPDATA.wnd_clientbox.left]
	rol	edx,16
  .forced:
	push	ebp esi 0
	mov	ebp, putimage_get24bpp
	mov	esi, putimage_init24bpp
sys_putimage_bpp:
;        cmp     [SCR_MODE], word 0x12
;        jz      @f   ;.doit
;        mov     eax, vesa12_putimage
;        cmp     [SCR_MODE], word 0100000000000000b
;        jae     @f
;        cmp     [SCR_MODE], word 0x13
;        jnz     .doit
;@@:
	mov	eax, vesa20_putimage
.doit:
	inc	[mouse_pause]
	call	eax
	dec	[mouse_pause]
	pop	ebp esi ebp
	jmp	[draw_pointer]
align 4
sys_putimage_palette:
; ebx = pointer to image
; ecx = [xsize]*65536 + [ysize]
; edx = [xstart]*65536 + [ystart]
; esi = number of bits per pixel, must be 8, 24 or 32
; edi = pointer to palette
; ebp = row delta
	mov	eax, [CURRENT_TASK]
	shl	eax, 8
	add	dx, word [eax+SLOT_BASE+APPDATA.wnd_clientbox.top]
	rol	edx, 16
	add	dx, word [eax+SLOT_BASE+APPDATA.wnd_clientbox.left]
	rol	edx, 16
.forced:
	cmp	esi, 1
	jnz	@f
	push	edi
	mov	eax, [edi+4]
	sub	eax, [edi]
	push	eax
	push	dword [edi]
	push	0ffffff80h
	mov	edi, esp
	call	put_mono_image
	add	esp, 12
	pop	edi
	ret
@@:
	cmp	esi, 2
	jnz	@f
	push	edi
	push	0ffffff80h
	mov	edi, esp
	call	put_2bit_image
	pop	eax
	pop	edi
	ret
@@:
	cmp	esi, 4
	jnz	@f
	push	edi
	push	0ffffff80h
	mov	edi, esp
	call	put_4bit_image
	pop	eax
	pop	edi
	ret
@@:
	push	ebp esi ebp
	cmp	esi, 8
	jnz	@f
	mov	ebp, putimage_get8bpp
	mov	esi, putimage_init8bpp
	jmp	sys_putimage_bpp
@@:
	cmp	esi, 15
	jnz	@f
	mov	ebp, putimage_get15bpp
	mov	esi, putimage_init15bpp
	jmp	sys_putimage_bpp
@@:
	cmp	esi, 16
	jnz	@f
	mov	ebp, putimage_get16bpp
	mov	esi, putimage_init16bpp
	jmp	sys_putimage_bpp
@@:
	cmp	esi, 24
	jnz	@f
	mov	ebp, putimage_get24bpp
	mov	esi, putimage_init24bpp
	jmp	sys_putimage_bpp
@@:
	cmp	esi, 32
	jnz	@f
	mov	ebp, putimage_get32bpp
	mov	esi, putimage_init32bpp
	jmp	sys_putimage_bpp
@@:
	pop	ebp esi ebp
	ret

put_mono_image:
	push	ebp esi ebp
	mov	ebp, putimage_get1bpp
	mov	esi, putimage_init1bpp
	jmp	sys_putimage_bpp
put_2bit_image:
	push	ebp esi ebp
	mov	ebp, putimage_get2bpp
	mov	esi, putimage_init2bpp
	jmp	sys_putimage_bpp
put_4bit_image:
	push	ebp esi ebp
	mov	ebp, putimage_get4bpp
	mov	esi, putimage_init4bpp
	jmp	sys_putimage_bpp

putimage_init24bpp:
	lea	eax, [eax*3]
putimage_init8bpp:
	ret

align 16
putimage_get24bpp:
	movzx	eax, byte [esi+2]
	shl	eax, 16
	mov	ax, [esi]
	add	esi, 3
	ret	4
align 16
putimage_get8bpp:
	movzx	eax, byte [esi]
	push	edx
	mov	edx, [esp+8]
	mov	eax, [edx+eax*4]
	pop	edx
	inc	esi
	ret	4

putimage_init1bpp:
	add	eax, ecx
	push	ecx
	add	eax, 7
	add	ecx, 7
	shr	eax, 3
	shr	ecx, 3
	sub	eax, ecx
	pop	ecx
	ret
align 16
putimage_get1bpp:
	push	edx
	mov	edx, [esp+8]
	mov	al, [edx]
	add	al, al
	jnz	@f
	lodsb
	adc	al, al
@@:
	mov	[edx], al
	sbb	eax, eax
	and	eax, [edx+8]
	add	eax, [edx+4]
	pop	edx
	ret	4

putimage_init2bpp:
	add	eax, ecx
	push	ecx
	add	ecx, 3
	add	eax, 3
	shr	ecx, 2
	shr	eax, 2
	sub	eax, ecx
	pop	ecx
	ret
align 16
putimage_get2bpp:
	push	edx
	mov	edx, [esp+8]
	mov	al, [edx]
	mov	ah, al
	shr	al, 6
	shl	ah, 2
	jnz	.nonewbyte
	lodsb
	mov	ah, al
	shr	al, 6
	shl	ah, 2
	add	ah, 1
.nonewbyte:
	mov	[edx], ah
	mov	edx, [edx+4]
	movzx	eax, al
	mov	eax, [edx+eax*4]
	pop	edx
	ret	4

putimage_init4bpp:
	add	eax, ecx
	push	ecx
	add	ecx, 1
	add	eax, 1
	shr	ecx, 1
	shr	eax, 1
	sub	eax, ecx
	pop	ecx
	ret
align 16
putimage_get4bpp:
	push	edx
	mov	edx, [esp+8]
	add	byte [edx], 80h
	jc	@f
	movzx	eax, byte [edx+1]
	mov	edx, [edx+4]
	and	eax, 0x0F
	mov	eax, [edx+eax*4]
	pop	edx
	ret	4
@@:
	movzx	eax, byte [esi]
	add	esi, 1
	mov	[edx+1], al
	shr	eax, 4
	mov	edx, [edx+4]
	mov	eax, [edx+eax*4]
	pop	edx
	ret	4

putimage_init32bpp:
	shl	eax, 2
	ret
align 16
putimage_get32bpp:
	lodsd
	ret	4

putimage_init15bpp:
putimage_init16bpp:
	add	eax, eax
	ret
align 16
putimage_get15bpp:
; 0RRRRRGGGGGBBBBB -> 00000000RRRRR000GGGGG000BBBBB000
	push	ecx edx
	movzx	eax, word [esi]
	add	esi, 2
	mov	ecx, eax
	mov	edx, eax
	and	eax, 0x1F
	and	ecx, 0x1F shl 5
	and	edx, 0x1F shl 10
	shl	eax, 3
	shl	ecx, 6
	shl	edx, 9
	or	eax, ecx
	or	eax, edx
	pop	edx ecx
	ret	4

align 16
putimage_get16bpp:
; RRRRRGGGGGGBBBBB -> 00000000RRRRR000GGGGGG00BBBBB000
	push	ecx edx
	movzx	eax, word [esi]
	add	esi, 2
	mov	ecx, eax
	mov	edx, eax
	and	eax, 0x1F
	and	ecx, 0x3F shl 5
	and	edx, 0x1F shl 11
	shl	eax, 3
	shl	ecx, 5
	shl	edx, 8
	or	eax, ecx
	or	eax, edx
	pop	edx ecx
	ret	4

; eax x beginning
; ebx y beginning
; ecx x end
	; edx y end
; edi color

__sys_drawbar:
	mov	esi,[current_slot]
	add	eax,[esi+APPDATA.wnd_clientbox.left]
	add	ecx,[esi+APPDATA.wnd_clientbox.left]
	add	ebx,[esi+APPDATA.wnd_clientbox.top]
	add	edx,[esi+APPDATA.wnd_clientbox.top]
  .forced:
    inc   [mouse_pause]
;        call    [disable_mouse]
    cmp   [SCR_MODE],word 0x12
    je	 dbv20
   sdbv20:
;    cmp  [SCR_MODE],word 0100000000000000b
;    jge  dbv20
;    cmp  [SCR_MODE],word 0x13
;    je   dbv20
;    call vesa12_drawbar
;    dec   [mouse_pause]
;    call   [draw_pointer]
;    ret
  dbv20:
    call vesa20_drawbar
    dec   [mouse_pause]
    call   [draw_pointer]
    ret



kb_read:

	push	ecx edx

	mov	ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
      kr_loop:
	in	al,0x64
	test	al,1
	jnz	kr_ready
	loop	kr_loop
	mov	ah,1
	jmp	kr_exit
      kr_ready:
	push	ecx
	mov	ecx,32
      kr_delay:
	loop	kr_delay
	pop	ecx
	in	al,0x60
	xor	ah,ah
      kr_exit:

	pop	edx ecx

	ret


kb_write:

	push	ecx edx

	mov	dl,al
;        mov     ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
;      kw_loop1:
;        in      al,0x64
;        test    al,0x20
;        jz      kw_ok1
;        loop    kw_loop1
;        mov     ah,1
;        jmp     kw_exit
;      kw_ok1:
	in	al,0x60
	mov	ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
      kw_loop:
	in	al,0x64
	test	al,2
	jz	kw_ok
	loop	kw_loop
	mov	ah,1
	jmp	kw_exit
      kw_ok:
	mov	al,dl
	out	0x60,al
	mov	ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
      kw_loop3:
	in	al,0x64
	test	al,2
	jz	kw_ok3
	loop	kw_loop3
	mov	ah,1
	jmp	kw_exit
      kw_ok3:
	mov	ah,8
      kw_loop4:
	mov	ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
      kw_loop5:
	in	al,0x64
	test	al,1
	jnz	kw_ok4
	loop	kw_loop5
	dec	ah
	jnz	kw_loop4
      kw_ok4:
	xor	ah,ah
      kw_exit:

	pop	edx ecx

	ret


kb_cmd:

	mov	ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
      c_wait:
	in	al,0x64
	test	al,2
	jz	c_send
	loop	c_wait
	jmp	c_error
      c_send:
	mov	al,bl
	out	0x64,al
	mov	ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
      c_accept:
	in	al,0x64
	test	al,2
	jz	c_ok
	loop	c_accept
      c_error:
	mov	ah,1
	jmp	c_exit
      c_ok:
	xor	ah,ah
      c_exit:
	ret


setmouse:  ; set mousepicture -pointer
	   ; ps2 mouse enable

     mov     [MOUSE_PICTURE],dword mousepointer

     cli

     ret

if used _rdtsc
_rdtsc:
     bt [cpu_caps], CAPS_TSC
     jnc ret_rdtsc
     rdtsc
     ret
   ret_rdtsc:
     mov   edx,0xffffffff
     mov   eax,0xffffffff
     ret
end if

rerouteirqs:

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

	mov	al,255		; mask all irq's
	out	0xA1,al
	call	pic_delay
	out	0x21,al
	call	pic_delay

	mov	ecx,0x1000
	cld
picl1:	call	pic_delay
	loop	picl1

	mov	al,255		; mask all irq's
	out	0xA1,al
	call	pic_delay
	out	0x21,al
	call	pic_delay

	cli

	ret


pic_delay:

	jmp	pdl1
pdl1:	ret


sys_msg_board_str:

     pushad
   @@:
     cmp    [esi],byte 0
     je     @f
     mov    eax,1
     movzx  ebx,byte [esi]
     call   sys_msg_board
     inc    esi
     jmp    @b
   @@:
     popad
     ret

sys_msg_board_byte:
; in: al = byte to display
; out: nothing
; destroys: nothing
	pushad
	mov	ecx, 2
	shl	eax, 24
	jmp	@f

sys_msg_board_word:
; in: ax = word to display
; out: nothing
; destroys: nothing
	pushad
	mov	ecx, 4
	shl	eax, 16
	jmp	@f

sys_msg_board_dword:
; in: eax = dword to display
; out: nothing
; destroys: nothing
	pushad
	mov	ecx, 8
@@:
	push	ecx
	rol	eax, 4
	push	eax
	and	al, 0xF
	cmp	al, 10
	sbb	al, 69h
	das
	mov	bl, al
	xor	eax, eax
	inc	eax
	call	sys_msg_board
	pop	eax
	pop	ecx
	loop	@b
	popad
	ret

uglobal
  msg_board_data: times 4096 db 0
  msg_board_count dd 0x0
endg

sys_msg_board:

; eax=1 : write :  bl byte to write
; eax=2 :  read :  ebx=0 -> no data, ebx=1 -> data in al

	mov	ecx, [msg_board_count]
	cmp	eax, 1
	jne	.smbl1

if defined debug_com_base

	push	dx ax

       @@:				; Wait for empty transmit register  (yes, this slows down system..)
	mov	dx, debug_com_base+5
	in	al, dx
	test	al, 1 shl 5
	jz	@r

	mov	dx, debug_com_base	; Output the byte
	mov	al, bl
	out	dx, al

	pop	ax dx

end if

	mov	[msg_board_data+ecx],bl
	inc	ecx
	and	ecx, 4095
	mov	[msg_board_count], ecx
	mov	[check_idle_semaphore], 5
	ret
.smbl1:
	cmp	eax, 2
	jne	.smbl2
	test	ecx, ecx
	jz	.smbl21
	mov	eax, msg_board_data+1
	mov	ebx, msg_board_data
	movzx	edx, byte [ebx]
	call	memmove
	dec	[msg_board_count]
	mov	[esp + 36], edx ;eax
	mov	[esp + 24], dword 1
	ret
.smbl21:
	mov	[esp+36], ecx
	mov	[esp+24], ecx
.smbl2:
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
	   dd sys_process_def.4
	   dd sys_process_def.5
endg




sys_process_def:
	dec	ebx
	cmp	ebx,5
	jae	.not_support	;if >=6 then or eax,-1

	mov	edi, [CURRENT_TASK]
	jmp	dword [f66call+ebx*4]

.not_support:
	or	eax,-1
	ret

.1:
     shl   edi,8
     mov   [edi+SLOT_BASE + APPDATA.keyboard_mode],cl

     ret

.2:				; 2 = get keyboard mode
     shl   edi,8
     movzx eax, byte [SLOT_BASE+edi + APPDATA.keyboard_mode]
     mov   [esp+32],eax
     ret
;     xor   eax,eax
;     movzx eax,byte [shift]
;     movzx ebx,byte [ctrl]
;     shl   ebx,2
;     add   eax,ebx
;     movzx ebx,byte [alt]
;     shl   ebx,3
;     add   eax,ebx
.3:				;3 = get keyboard ctrl, alt, shift
 ;// mike.dld [
     mov   eax, [kb_state]
 ;// mike.dld ]
     mov   [esp+32],eax
     ret

.4:
	mov	eax, hotkey_list
@@:
	cmp	dword [eax+8], 0
	jz	.found_free
	add	eax, 16
	cmp	eax, hotkey_list+16*256
	jb	@b
	mov	dword [esp+32], 1
	ret
.found_free:
	mov	[eax+8], edi
	mov	[eax+4], edx
	movzx	ecx, cl
	lea	ecx, [hotkey_scancodes+ecx*4]
	mov	edx, [ecx]
	mov	[eax], edx
	mov	[ecx], eax
	mov	[eax+12], ecx
	jecxz	@f
	mov	[edx+12], eax
@@:
	and	dword [esp+32], 0
	ret

.5:
	movzx	ebx, cl
	lea	ebx, [hotkey_scancodes+ebx*4]
	mov	eax, [ebx]
.scan:
	test	eax, eax
	jz	.notfound
	cmp	[eax+8], edi
	jnz	.next
	cmp	[eax+4], edx
	jz	.found
.next:
	mov	eax, [eax]
	jmp	.scan
.notfound:
	mov	dword [esp+32], 1
	ret
.found:
	mov	ecx, [eax]
	jecxz	@f
	mov	edx, [eax+12]
	mov	[ecx+12], edx
@@:
	mov	ecx, [eax+12]
	mov	edx, [eax]
	mov	[ecx], edx
	xor	edx, edx
	mov	[eax+4], edx
	mov	[eax+8], edx
	mov	[eax+12], edx
	mov	[eax], edx
	mov	[esp+32], edx
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; 61 sys function.                                                ;;
;; in eax=61,ebx in [1..3]                                         ;;
;; out eax                                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
iglobal
align 4
f61call:
	   dd sys_gs.1	 ; resolution
	   dd sys_gs.2	 ; bits per pixel
	   dd sys_gs.3	 ; bytes per scanline
endg


align 4

sys_gs: 			; direct screen access
	dec	ebx
	cmp	ebx,2
	ja	.not_support
	jmp	dword [f61call+ebx*4]
.not_support:
	or  [esp+32],dword -1
	ret


.1:				; resolution
     mov  eax,[Screen_Max_X]
     shl  eax,16
     mov  ax,[Screen_Max_Y]
     add  eax,0x00010001
     mov  [esp+32],eax
     ret
.2:				; bits per pixel
     movzx eax,byte [ScreenBPP]
     mov   [esp+32],eax
     ret
.3:				; bytes per scanline
     mov   eax,[BytesPerScanLine]
     mov   [esp+32],eax
     ret


;align 4 ; PCI functions
;
sys_pci:

     call  pci_api
     mov   [esp+36],eax
     ret


align 4  ;  system functions

syscall_setpixel:			; SetPixel

	mov	eax, ebx
	mov	ebx, ecx
	mov	ecx, edx
	mov	edx, [TASK_BASE]
	add	eax, [edx-twdw+WDATA.box.left]
	add	ebx, [edx-twdw+WDATA.box.top]
	mov	edi, [current_slot]
	add	eax, [edi+APPDATA.wnd_clientbox.left]
	add	ebx, [edi+APPDATA.wnd_clientbox.top]
	xor	edi, edi ; no force
;       mov     edi, 1
	call	[_display.disable_mouse]
	jmp	[putpixel]

align 4

syscall_writetext:			; WriteText

	mov   eax,[TASK_BASE]
	mov   ebp,[eax-twdw+WDATA.box.left]
	push  esi
	mov   esi,[current_slot]
	add   ebp,[esi+APPDATA.wnd_clientbox.left]
	shl   ebp,16
	add   ebp,[eax-twdw+WDATA.box.top]
	add   bp,word[esi+APPDATA.wnd_clientbox.top]
	pop   esi
	add   ebx,ebp
	mov   eax,edi
	xor   edi,edi
	jmp   dtext

align 4

syscall_openramdiskfile:		; OpenRamdiskFile

	mov	eax, ebx
	mov	ebx, ecx
	mov	ecx, edx
	mov	edx, esi
	mov	esi, 12
	call	fileread
	mov	[esp+32], eax
	ret

align 4

syscall_drawrect:			; DrawRect

	mov	edi, edx ; color + gradient
	and	edi, 0x80FFFFFF
	test	bx, bx	; x.size
	je	.drectr
	test	cx, cx ; y.size
	je	.drectr

	mov	eax, ebx ; bad idea
	mov	ebx, ecx

	movzx	ecx, ax ; ecx - x.size
	shr	eax, 16 ; eax - x.coord
	movzx	edx, bx ; edx - y.size
	shr	ebx, 16 ; ebx - y.coord
	mov	esi, [current_slot]

	add	eax, [esi + APPDATA.wnd_clientbox.left]
	add	ebx, [esi + APPDATA.wnd_clientbox.top]
	add	ecx, eax
	add	edx, ebx
	jmp	[drawbar]
.drectr:
	ret

align 4
syscall_getscreensize:			; GetScreenSize
	mov	ax, [Screen_Max_X]
	shl	eax, 16
	mov	ax, [Screen_Max_Y]
	mov	[esp + 32], eax
	ret

align 4


align 4

syscall_getpixel:			; GetPixel
     mov   ecx, [Screen_Max_X]
     inc   ecx
     xor   edx, edx
     mov   eax, ebx
     div   ecx
     mov   ebx, edx
     xchg  eax, ebx
     call  dword [GETPIXEL] ; eax - x, ebx - y
     mov   [esp + 32], ecx
     ret

align 4

syscall_getarea:
;eax = 36
;ebx = pointer to bufer for img BBGGRRBBGGRR...
;ecx = [size x]*65536 + [size y]
;edx = [start x]*65536 + [start y]
     pushad
	 inc   [mouse_pause]
; Check of use of the hardware cursor.
      cmp  [_display.disable_mouse],__sys_disable_mouse
	  jne  @f
; Since the test for the coordinates of the mouse should not be used,
; then use the call [disable_mouse] is not possible!
      cmp  dword [MOUSE_VISIBLE],dword 0
      jne  @f
      pushf
      cli
      call draw_mouse_under
      popf
      mov  [MOUSE_VISIBLE],dword 1
@@:
     mov   edi,ebx
     mov   eax,edx
     shr   eax,16
     mov   ebx,edx
     and   ebx,0xffff
     dec   eax
	   dec	 ebx
     ; eax - x, ebx - y
     mov   edx,ecx

     shr   ecx,16
     and   edx,0xffff
     mov   esi,ecx
     ; ecx - size x, edx - size y

	 mov   ebp,edx
	 dec   ebp
     lea   ebp,[ebp*3]

	 imul  ebp,esi

	 mov   esi,ecx
	 dec   esi
	 lea   esi,[esi*3]

     add   ebp,esi
     add   ebp,edi

     add   ebx,edx

.start_y:
     push  ecx edx
.start_x:
     push  eax ebx ecx
     add   eax,ecx

     call  dword [GETPIXEL] ; eax - x, ebx - y

     mov   [ebp],cx
     shr   ecx,16
     mov   [ebp+2],cl

     pop   ecx ebx eax
     sub   ebp,3
     dec   ecx
     jnz   .start_x
	 pop   edx ecx
	 dec   ebx
     dec   edx
     jnz   .start_y
     dec	[mouse_pause]
; Check of use of the hardware cursor.
      cmp  [_display.disable_mouse],__sys_disable_mouse
	  jne  @f
	 call  [draw_pointer]
@@:
     popad
     ret

align 4

syscall_drawline:			; DrawLine

	mov	edi, [TASK_BASE]
	movzx	eax, word[edi-twdw+WDATA.box.left]
	mov	ebp, eax
	mov	esi, [current_slot]
	add	ebp, [esi+APPDATA.wnd_clientbox.left]
	add	ax, word[esi+APPDATA.wnd_clientbox.left]
	add	ebp,ebx
	shl	eax, 16
	movzx	ebx, word[edi-twdw+WDATA.box.top]
	add	eax, ebp
	mov	ebp, ebx
	add	ebp, [esi+APPDATA.wnd_clientbox.top]
	add	bx, word[esi+APPDATA.wnd_clientbox.top]
	add	ebp, ecx
	shl	ebx, 16
	xor	edi, edi
	add	ebx, ebp
	mov	ecx, edx
	jmp	[draw_line]

align 4

syscall_getirqowner:			; GetIrqOwner

     cmp   ebx,16
     jae   .err

     cmp   [irq_rights + 4 * ebx], dword 2
     je    .err

     mov   eax,[4 * ebx + irq_owner]
     mov   [esp+32],eax

     ret
.err:
     or    dword [esp+32], -1
     ret

align 4

syscall_reserveportarea:		; ReservePortArea and FreePortArea

     call  r_f_port_area
     mov   [esp+32],eax
     ret

align 4

syscall_threads:			; CreateThreads
; eax=1 create thread
;
;   ebx=thread start
;   ecx=thread stack value
;
; on return : eax = pid

     call  new_sys_threads

     mov   [esp+32],eax
     ret

align 4

stack_driver_stat:

     call  app_stack_handler		; Stack status

;     mov   [check_idle_semaphore],5    ; enable these for zero delay
;     call  change_task                 ; between sent packet

     mov   [esp+32],eax
     ret

align 4

socket: 				; Socket interface
     call  app_socket_handler

;     mov   [check_idle_semaphore],5    ; enable these for zero delay
;     call  change_task                 ; between sent packet

     mov   [esp+36],eax
     mov   [esp+24],ebx
     ret

align 4

read_from_hd:				; Read from hd - fn not in use

     mov   edi,[TASK_BASE]
     add   edi,TASKDATA.mem_start
     add   eax,[edi]
     add   ecx,[edi]
     add   edx,[edi]
     call  file_read

     mov   [esp+36],eax
     mov   [esp+24],ebx

     ret

paleholder:
	ret

align 4
set_screen:
	cmp eax, [Screen_Max_X]
	jne .set

	cmp edx, [Screen_Max_Y]
	jne .set
	ret
.set:
	pushfd
	cli

	mov [Screen_Max_X], eax
	mov [Screen_Max_Y], edx
	mov [BytesPerScanLine], ecx

	mov [screen_workarea.right],eax
	mov [screen_workarea.bottom], edx

	push ebx
	push esi
	push edi

	pushad

	stdcall kernel_free, [_WinMapAddress]

	mov eax, [_display.width]
	mul [_display.height]
	mov [_WinMapSize], eax

	stdcall kernel_alloc, eax
	mov [_WinMapAddress], eax
	test eax, eax
	jz .epic_fail

	popad

	call	repos_windows
	xor eax, eax
	xor ebx, ebx
	mov	ecx, [Screen_Max_X]
	mov	edx, [Screen_Max_Y]
	call	calculatescreen
	pop edi
	pop esi
	pop ebx

	popfd
	ret

.epic_fail:
	hlt			; Houston, we've had a problem

; --------------- APM ---------------------
uglobal
apm_entry	dp	0
apm_vf		dd	0
endg

align 4
sys_apm:
	xor	eax,eax
	cmp	word [apm_vf], ax	; Check APM BIOS enable
	jne	@f
	inc	eax
	or	dword [esp + 44], eax	; error
	add	eax,7
	mov	dword [esp + 32], eax	; 32-bit protected-mode interface not supported
	ret

@@:
;       xchg    eax, ecx
;       xchg    ebx, ecx

	cmp	dx, 3
	ja	@f
	and	[esp + 44], byte 0xfe	 ; emulate func 0..3 as func 0
	mov	eax,[apm_vf]
	mov	[esp + 32], eax
	shr	eax, 16
	mov	[esp + 28], eax
	ret

@@:

	mov	esi,[master_tab+(OS_BASE shr 20)]
	xchg	[master_tab], esi
	push	esi
	mov	edi, cr3
	mov	cr3, edi		;flush TLB

	call	pword [apm_entry]	;call APM BIOS

	xchg	eax, [esp]
	mov	[master_tab], eax
	mov	eax, cr3
	mov	cr3, eax
	pop eax

	mov	[esp + 4 ], edi
	mov	[esp + 8], esi
	mov	[esp + 20], ebx
	mov	[esp + 24], edx
	mov	[esp + 28], ecx
	mov	[esp + 32], eax
	setc	al
	and	[esp + 44], byte 0xfe
	or	[esp + 44], al
	ret
; -----------------------------------------

align 4

undefined_syscall:			; Undefined system call
     mov   [esp + 32], dword -1
     ret

align 4
system_shutdown:	  ; shut down the system

	   cmp byte [BOOT_VAR+0x9030], 1
	   jne @F
	   ret
@@:
	   call stop_all_services

yes_shutdown_param:
	   cli

	   mov	eax, kernel_file ; load kernel.mnt to 0x7000:0
	   push 12
	   pop	esi
	   xor	ebx,ebx
	   or	ecx,-1
	   mov	edx, OS_BASE+0x70000
	   call fileread

	   mov	esi, restart_kernel_4000+OS_BASE+0x10000 ; move kernel re-starter to 0x4000:0
	   mov	edi,OS_BASE+0x40000
	   mov	ecx,1000
	   rep	movsb

	   mov	esi,OS_BASE+0x2F0000	; restore 0x0 - 0xffff
	   mov	edi, OS_BASE
	   mov	ecx,0x10000/4
	   cld
	   rep movsd

	   call restorefatchain

	   mov al, 0xFF
	   out 0x21, al
	   out 0xA1, al

if 0
	   mov	word [OS_BASE+0x467+0],pr_mode_exit
	   mov	word [OS_BASE+0x467+2],0x1000

	   mov	al,0x0F
	   out	0x70,al
	   mov	al,0x05
	   out	0x71,al

	   mov	al,0xFE
	   out	0x64,al

	   hlt
	   jmp $-1

else
	cmp	byte [OS_BASE + 0x9030], 2
	jnz	no_acpi_power_off

; scan for RSDP
; 1) The first 1 Kb of the Extended BIOS Data Area (EBDA).
	movzx	eax, word [OS_BASE + 0x40E]
	shl	eax, 4
	jz	@f
	mov	ecx, 1024/16
	call	scan_rsdp
	jnc	.rsdp_found
@@:
; 2) The BIOS read-only memory space between 0E0000h and 0FFFFFh.
	mov	eax, 0xE0000
	mov	ecx, 0x2000
	call	scan_rsdp
	jc	no_acpi_power_off
.rsdp_found:
	mov	esi, [eax+16]	; esi contains physical address of the RSDT
	mov	ebp, [ipc_tmp]
	stdcall map_page, ebp, esi, PG_MAP
	lea	eax, [esi+1000h]
	lea	edx, [ebp+1000h]
	stdcall map_page, edx, eax, PG_MAP
	and	esi, 0xFFF
	add	esi, ebp
	cmp	dword [esi], 'RSDT'
	jnz	no_acpi_power_off
	mov	ecx, [esi+4]
	sub	ecx, 24h
	jbe	no_acpi_power_off
	shr	ecx, 2
	add	esi, 24h
.scan_fadt:
	lodsd
	mov	ebx, eax
	lea	eax, [ebp+2000h]
	stdcall map_page, eax, ebx, PG_MAP
	lea	eax, [ebp+3000h]
	add	ebx, 0x1000
	stdcall map_page, eax, ebx, PG_MAP
	and	ebx, 0xFFF
	lea	ebx, [ebx+ebp+2000h]
	cmp	dword [ebx], 'FACP'
	jz	.fadt_found
	loop	.scan_fadt
	jmp	no_acpi_power_off
.fadt_found:
; ebx is linear address of FADT
	mov	edi, [ebx+40] ; physical address of the DSDT
	lea	eax, [ebp+4000h]
	stdcall map_page, eax, edi, PG_MAP
	lea	eax, [ebp+5000h]
	lea	esi, [edi+0x1000]
	stdcall map_page, eax, esi, PG_MAP
	and	esi, 0xFFF
	sub	edi, esi
	cmp	dword [esi+ebp+4000h], 'DSDT'
	jnz	no_acpi_power_off
	mov	eax, [esi+ebp+4004h] ; DSDT length
	sub	eax, 36+4
	jbe	no_acpi_power_off
	add	esi, 36
.scan_dsdt:
	cmp	dword [esi+ebp+4000h], '_S5_'
	jnz	.scan_dsdt_cont
	cmp	byte [esi+ebp+4000h+4], 12h ; DefPackage opcode
	jnz	.scan_dsdt_cont
	mov	dl, [esi+ebp+4000h+6]
	cmp	dl, 4 ; _S5_ package must contain 4 bytes
		      ; ...in theory; in practice, VirtualBox has 2 bytes
	ja	.scan_dsdt_cont
	cmp	dl, 1
	jb	.scan_dsdt_cont
	lea	esi, [esi+ebp+4000h+7]
	xor	ecx, ecx
	cmp	byte [esi], 0 ; 0 means zero byte, 0Ah xx means byte xx
	jz	@f
	cmp	byte [esi], 0xA
	jnz	no_acpi_power_off
	inc	esi
	mov	cl, [esi]
@@:
	inc	esi
	cmp	dl, 2
	jb	@f
	cmp	byte [esi], 0
	jz	@f
	cmp	byte [esi], 0xA
	jnz	no_acpi_power_off
	inc	esi
	mov	ch, [esi]
@@:
	jmp	do_acpi_power_off
.scan_dsdt_cont:
	inc	esi
	cmp	esi, 0x1000
	jb	@f
	sub	esi, 0x1000
	add	edi, 0x1000
	push	eax
	lea	eax, [ebp+4000h]
	stdcall map_page, eax, edi, PG_MAP
	push	PG_MAP
	lea	eax, [edi+1000h]
	push	eax
	lea	eax, [ebp+5000h]
	push	eax
	stdcall map_page
	pop	eax
@@:
	dec	eax
	jnz	.scan_dsdt
	jmp	no_acpi_power_off
do_acpi_power_off:
	mov	edx, [ebx+48]
	test	edx, edx
	jz	.nosmi
	mov	al, [ebx+52]
	out	dx, al
	mov	edx, [ebx+64]
@@:
	in	ax, dx
	test	al, 1
	jz	@b
.nosmi:
	and	cx, 0x0707
	shl	cx, 2
	or	cx, 0x2020
	mov	edx, [ebx+64]
	in	ax, dx
	and	ax, 203h
	or	ah, cl
	out	dx, ax
	mov	edx, [ebx+68]
	test	edx, edx
	jz	@f
	in	ax, dx
	and	ax, 203h
	or	ah, ch
	out	dx, ax
@@:
	jmp	$


no_acpi_power_off:
	   mov	word [OS_BASE+0x467+0],pr_mode_exit
	   mov	word [OS_BASE+0x467+2],0x1000

	   mov	al,0x0F
	   out	0x70,al
	   mov	al,0x05
	   out	0x71,al

	   mov	al,0xFE
	   out	0x64,al

	   hlt
	   jmp $-1

scan_rsdp:
	add	eax, OS_BASE
.s:
	cmp	dword [eax], 'RSD '
	jnz	.n
	cmp	dword [eax+4], 'PTR '
	jnz	.n
	xor	edx, edx
	xor	esi, esi
@@:
	add	dl, [eax+esi]
	inc	esi
	cmp	esi, 20
	jnz	@b
	test	dl, dl
	jz	.ok
.n:
	add	eax, 10h
	loop	.s
	stc
.ok:
	ret
end if

diff16 "End of 32-code ",0,$

include "data32.inc"

__REV__ = __REV

uglobals_size = $ - endofcode
diff16 "Zero-filled blk",0,endofcode
diff16 "End of kernel  ",0,$
