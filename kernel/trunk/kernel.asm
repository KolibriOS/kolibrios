;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved.
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


USE_COM_IRQ equ 1      ;make irq 3 and irq 4 available for PCI devices

include "proc32.inc"
include "kglobals.inc"
include "lang.inc"

include "const.inc"
max_processes	 equ   255
tss_step	 equ   (128+8192) ; tss & i/o - 65535 ports, * 256=557056*4


os_stack       equ  (os_data_l-gdts)	; GDTs
os_code        equ  (os_code_l-gdts)
graph_data     equ  (3+graph_data_l-gdts)
tss0	       equ  (tss0_l-gdts)
app_code       equ  (3+app_code_l-gdts)
app_data       equ  (3+app_data_l-gdts)
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

version db    'Kolibri OS  version 0.7.5.0      ',13,10,13,10,0

include "boot/bootstr.inc"     ; language-independent boot messages
include "boot/preboot.inc"

if lang eq en
include "boot/booteng.inc"     ; english system boot messages
else if lang eq ru
include "boot/bootru.inc"      ; russian system boot messages
include "boot/ru.inc"	       ; Russian font
else if lang eq et
include "boot/bootet.inc"      ; estonian system boot messages
include "boot/et.inc"	       ; Estonian font
else
include "boot/bootge.inc"      ; german system boot messages
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

include "data16.inc"

use32
org $+0x10000

align 4
B32:
	   mov	 ax,os_stack	   ; Selector for os
	   mov	 ds,ax
	   mov	 es,ax
	   mov	 fs,ax
	   mov	 gs,ax
	   mov	 ss,ax
	   mov	 esp,0x3ec00	   ; Set stack

; CLEAR 0x280000 - HEAP_BASE

	   xor	 eax,eax
	   mov	 edi,0x280000
	   mov	 ecx,(HEAP_BASE-OS_BASE-0x280000) / 4
	   cld
	   rep	 stosd

	   mov	 edi,0x40000
	   mov	 ecx,(0x90000-0x40000)/4
	   rep	 stosd

; CLEAR KERNEL UNDEFINED GLOBALS
	   mov	 edi, endofcode-OS_BASE
	   mov	 ecx, (uglobals_size/4)+4
	   rep	 stosd

; SAVE & CLEAR 0-0xffff

	   xor esi, esi
	   mov	 edi,0x2F0000
	   mov	 ecx,0x10000 / 4
	   rep	 movsd
	   xor edi, edi
	   mov	 ecx,0x10000 / 4
	   rep	 stosd

	   call test_cpu
	   bts [cpu_caps-OS_BASE], CAPS_TSC	;force use rdtsc

	   call init_BIOS32
; MEMORY MODEL
	   call mem_test
	   call init_mem
	   call init_page_map

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
	   mov ax,os_stack
	   mov bx,app_data
	   mov ss,ax
	   add	esp, OS_BASE

	   mov ds,bx
	   mov es,bx
	   mov fs,bx
	   mov gs,bx

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
    mov ebx,	[BOOT_VAR+0x9040]	       ; offset of APM entry point
    movzx eax, word [BOOT_VAR+0x9050] ; real-mode segment base address of
				      ; protected-mode 32-bit code segment
    movzx ecx, word [BOOT_VAR+0x9052] ; real-mode segment base address of
				      ; protected-mode 16-bit code segment
    movzx edx, word [BOOT_VAR+0x9054] ; real-mode segment base address of
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
;        movzx eax,byte [BOOT_VAR+0x9010]  ; mouse port
;        mov   [0xF604],byte 1  ;al
	mov	al, [BOOT_VAR+0x901F]	; DMA access
	mov	[allow_dma_access], al
	mov   al,[BOOT_VAR+0x9000]	  ; bpp
	mov   [ScreenBPP],al

	movzx eax,word [BOOT_VAR+0x900A]  ; X max
	dec   eax
	mov   [Screen_Max_X],eax
	mov   [screen_workarea.right],eax
	movzx eax,word [BOOT_VAR+0x900C]  ; Y max
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
	mov   ax,[BOOT_VAR+0x9001]	  ; for other modes
	mov   [BytesPerScanLine],ax
@@:
	mov	esi, BOOT_VAR+0x9080
	movzx	ecx, byte [esi-1]
	mov	[NumBiosDisks], ecx
	mov	edi, BiosDisksData
	rep	movsd

; GRAPHICS ADDRESSES

	mov	byte [BOOT_VAR+0x901e],0x0
	mov	eax,[BOOT_VAR+0x9018]
	mov	[LFBAddress],eax

	cmp	[SCR_MODE],word 0100000000000000b
	jge	setvesa20
	cmp	[SCR_MODE],word 0x13
	je	v20ga32
	mov	[PUTPIXEL],dword Vesa12_putpixel24  ; Vesa 1.2
	mov	[GETPIXEL],dword Vesa12_getpixel24
	cmp	[ScreenBPP],byte 24
	jz	ga24
	mov	[PUTPIXEL],dword Vesa12_putpixel32
	mov	[GETPIXEL],dword Vesa12_getpixel32
      ga24:
	jmp	v20ga24
      setvesa20:
	mov	[PUTPIXEL],dword Vesa20_putpixel24  ; Vesa 2.0
	mov	[GETPIXEL],dword Vesa20_getpixel24
	cmp	[ScreenBPP],byte 24
	jz	v20ga24
      v20ga32:
	mov	[PUTPIXEL],dword Vesa20_putpixel32
	mov	[GETPIXEL],dword Vesa20_getpixel32
      v20ga24:
	cmp	[SCR_MODE],word 0x12		    ; 16 C VGA 640x480
	jne	no_mode_0x12
	mov	[PUTPIXEL],dword VGA_putpixel
	mov	[GETPIXEL],dword Vesa20_getpixel32
      no_mode_0x12:

; -------- Fast System Call init ----------
; Intel SYSENTER/SYSEXIT (AMD CPU support it too)
	   bt [cpu_caps], CAPS_SEP
	   jnc .SEnP   ; SysEnter not Present
	   xor edx, edx
	   mov ecx, MSR_SYSENTER_CS
	   mov eax, os_code
	   wrmsr
	   mov ecx, MSR_SYSENTER_ESP
;           mov eax, sysenter_stack ; Check it
	   xor	   eax, eax
	   wrmsr
	   mov ecx, MSR_SYSENTER_EIP
	   mov eax, sysenter_entry
	   wrmsr
.SEnP:
; AMD SYSCALL/SYSRET
	   cmp byte[cpu_vendor], 'A'
	   jne .noSYSCALL
	   mov eax, 0x80000001
	   cpuid
	   test edx, 0x800  ; bit_11 - SYSCALL/SYSRET support
	   jz .noSYSCALL
	   mov ecx, MSR_AMD_EFER
	   rdmsr
	   or eax, 1   ; bit_0 - System Call Extension (SCE)
	   wrmsr

	; !!!! It`s dirty hack, fix it !!!
	; Bits of EDX :
	; Bit 31Ц16 During the SYSRET instruction, this field is copied into the CS register
	;  and the contents of this field, plus 8, are copied into the SS register.
	; Bit 15Ц0 During the SYSCALL instruction, this field is copied into the CS register
	;  and the contents of this field, plus 8, are copied into the SS register.

	; mov   edx, (os_code + 16) * 65536 + os_code
	   mov edx, 0x1B0008

	   mov eax, syscall_entry
	   mov ecx, MSR_AMD_STAR
	   wrmsr
.noSYSCALL:
; -----------------------------------------

; LOAD IDT

	   call build_interrupt_table
	   lidt [idtreg]

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
	   not eax
	   mov ecx, 8192/4
	   rep stosd		     ; access to 4096*8=65536 ports

	   mov	ax,tss0
	   ltr	ax

	   mov [LFBSize], 0x800000
	   call init_LFB
	   call init_fpu
	   call init_malloc

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

	mov [CURRENT_TASK],dword 1
	mov [TASK_COUNT],dword 1
	mov [TASK_BASE],dword TASK_DATA
	mov [current_slot], SLOT_BASE+256

; set background
	xor  eax,eax
	inc  eax
	mov   [BgrDrawMode],eax
	mov   [BgrDataWidth],eax
	mov   [BgrDataHeight],eax
	mov    [mem_BACKGROUND],4095
	stdcall kernel_alloc, [mem_BACKGROUND]
	mov [img_background], eax

	mov	[SLOT_BASE + 256 + APPDATA.dir_table], sys_pgdir - OS_BASE

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

  mov ax,[OS_BASE+0x10000+bx_from_load]
  cmp ax,'r1'		; if using not ram disk, then load librares and parameters {SPraid.simba}
  je  no_lib_load
; LOADING LIBRARES
   stdcall dll.Load,@IMPORT		    ; loading librares for kernel (.obj files)
   call load_file_parse_table		    ; prepare file parse table
   call set_kernel_conf 		    ; configure devices and gui
no_lib_load:

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
	mov	edi, 1
	mov	eax, 0x00040000
	call	display_number_force

; BUILD SCHEDULER

	call   build_scheduler ; sys32.inc

	mov    esi,boot_devices
	call   boot_log

	mov  [pci_access_enabled],1


; SET PRELIMINARY WINDOW STACK AND POSITIONS

	mov   esi,boot_windefs
	call  boot_log
	call  setwindowdefaults

; SET BACKGROUND DEFAULTS

	mov   esi,boot_bgr
	call  boot_log
	call  init_background
	call  calculatebackground

; RESERVE SYSTEM IRQ'S JA PORT'S

	mov   esi,boot_resirqports
	call  boot_log
	call  reserve_irqs_ports

; SET PORTS FOR IRQ HANDLERS

	mov  esi,boot_setrports
	call boot_log
	;call setirqreadports

; SET UP OS TASK

	mov  esi,boot_setostask
	call boot_log

	xor  eax, eax
	mov  dword [SLOT_BASE+APPDATA.fpu_state], fpu_data
	mov  dword [SLOT_BASE+APPDATA.fpu_handler], eax
	mov  dword [SLOT_BASE+APPDATA.sse_handler], eax

	; name for OS/IDLE process

	mov dword [SLOT_BASE+256+APPDATA.app_name],   dword 'OS/I'
	mov dword [SLOT_BASE+256+APPDATA.app_name+4], dword 'DLE '
	mov edi, [os_stack_seg]
	mov dword [SLOT_BASE+256+APPDATA.pl0_stack], edi
	add edi, 0x2000-512
	mov dword [SLOT_BASE+256+APPDATA.fpu_state], edi
	mov dword [SLOT_BASE+256+APPDATA.saved_esp0], edi ; just for case
	mov dword [SLOT_BASE+256+APPDATA.io_map],\
		  (tss._io_map_0-OS_BASE+PG_MAP)
	mov dword [SLOT_BASE+256+APPDATA.io_map+4],\
		  (tss._io_map_1-OS_BASE+PG_MAP)

	mov esi, fpu_data
	mov ecx, 512/4
	cld
	rep movsd

	mov dword [SLOT_BASE+256+APPDATA.fpu_handler], eax
	mov dword [SLOT_BASE+256+APPDATA.sse_handler], eax

	mov ebx, SLOT_BASE+256+APP_OBJ_OFFSET
	mov  dword [SLOT_BASE+256+APPDATA.fd_obj], ebx
	mov  dword [SLOT_BASE+256+APPDATA.bk_obj], ebx

	mov  dword [SLOT_BASE+256+APPDATA.cur_dir], sysdir_path

	; task list
	mov  [CURRENT_TASK],dword 1
	mov  [TASK_COUNT],dword 1
	mov  [current_slot], SLOT_BASE+256
	mov  [TASK_BASE],dword TASK_DATA
	mov  [TASK_DATA+TASKDATA.wnd_number], 1 ; on screen number
	mov  [TASK_DATA+TASKDATA.pid], 1	; process id number
	mov  [TASK_DATA+TASKDATA.mem_start], 0	; process base address

	call init_cursors
        mov eax, [def_cursor]
	mov [SLOT_BASE+APPDATA.cursor],eax
	mov [SLOT_BASE+APPDATA.cursor+256],eax

  ; READ TSC / SECOND

	mov   esi,boot_tsc
	call  boot_log
	cli
	call  _rdtsc
	mov   ecx,eax
	mov   esi,250		    ; wait 1/4 a second
	call  delay_ms
	call  _rdtsc
	sti
	sub   eax,ecx
	shl   eax,2
	mov   [CPU_FREQ],eax	      ; save tsc / sec
	mov ebx, 1000000
	div ebx
	mov [stall_mcs], eax

; SET VARIABLES

	call  set_variables

; SET MOUSE

	;call   detect_devices
	stdcall load_driver, szPS2MDriver
	stdcall load_driver, szCOM_MDriver

	mov   esi,boot_setmouse
	call  boot_log
	call  setmouse


; STACK AND FDC

	call  stack_init
	call  fdc_init

; PALETTE FOR 320x200 and 640x480 16 col

	cmp   [SCR_MODE],word 0x12
	jne   no_pal_vga
	mov   esi,boot_pal_vga
	call  boot_log
	call  paletteVGA
      no_pal_vga:

	cmp   [SCR_MODE],word 0x13
	jne   no_pal_ega
	mov   esi,boot_pal_ega
	call  boot_log
	call  palette320x200
      no_pal_ega:

; LOAD DEFAULT SKIN

	call	load_default_skin

;protect io permission map

	   mov esi, [default_io_map]
	   stdcall map_page,esi,(tss._io_map_0-OS_BASE), PG_MAP
	   add esi, 0x1000
	   stdcall map_page,esi,(tss._io_map_1-OS_BASE), PG_MAP

	   stdcall map_page,tss._io_map_0,\
		   (tss._io_map_0-OS_BASE), PG_MAP
	   stdcall map_page,tss._io_map_1,\
		   (tss._io_map_1-OS_BASE), PG_MAP

  mov ax,[OS_BASE+0x10000+bx_from_load]
  cmp ax,'r1'		; if not rused ram disk - load network configuration from files {SPraid.simba}
  je  no_st_network
	call set_network_conf
  no_st_network:

; LOAD FIRST APPLICATION
	cli

	cmp   byte [BOOT_VAR+0x9030],1
	jne   no_load_vrr_m

	mov	ebp, vrr_m
	call	fs_execute_from_sysdir

	cmp   eax,2		     ; if vrr_m app found (PID=2)
	je    first_app_found

no_load_vrr_m:

	mov	ebp, firstapp
	call	fs_execute_from_sysdir

	cmp   eax,2		     ; continue if a process has been loaded
	je    first_app_found

	mov	esi, boot_failed
	call	boot_log

	mov   eax, 0xDEADBEEF	     ; otherwise halt
	hlt

first_app_found:

	cli

	;mov   [TASK_COUNT],dword 2
	mov   [CURRENT_TASK],dword 1	   ; set OS task fisrt

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

	mov   esi,boot_allirqs
	call  boot_log

	cli			     ;guarantee forbidance of interrupts.
	mov   al,0		     ; unmask all irq's
	out   0xA1,al
	out   0x21,al

	mov   ecx,32

     ready_for_irqs:

	mov   al,0x20		     ; ready for irqs
	out   0x20,al
	out   0xa0,al

	loop  ready_for_irqs	     ; flush the queue

	stdcall attach_int_handler, dword 1, irq1, dword 0

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
	 mov   ecx,0x80ffffff	; ASCIIZ string with white color
	 mov   edx,esi
	 mov   edi,1
	 call  dtext

	 mov   [novesachecksum],1000
	 call  checkVga_N13

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
	call   checkbuttons
	call   checkwindows
;       call   check_window_move_request
	call   checkmisc
	call   checkVga_N13
	call   stack_handler
	call   checkidle
	call   check_fdd_motor_status
	call   check_ATAPI_device_event
	jmp    osloop
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                                    ;
;                      MAIN OS LOOP END                              ;
;                                                                    ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

checkidle:
	pushad

	cmp  [check_idle_semaphore],0
	jne  no_idle_state

	call change_task
	mov  eax,[idlemem]
	mov  ebx,[timer_ticks] ;[0xfdf0]
	cmp  eax,ebx
	jnz  idle_exit
	call _rdtsc
	mov  ecx,eax
      idle_loop:
	hlt
	cmp  [check_idle_semaphore],0
	jne  idle_loop_exit
	mov  eax,[timer_ticks] ;[0xfdf0]
	cmp  ebx,eax
	jz   idle_loop
      idle_loop_exit:
	mov  [idlemem],eax
	call _rdtsc
	sub  eax,ecx
	mov  ebx,[idleuse]
	add  ebx,eax
	mov  [idleuse],ebx

	popad
	ret

      idle_exit:

	mov  ebx,[timer_ticks] ;[0xfdf0]
	mov  [idlemem],ebx
	call change_task

	popad
	ret

      no_idle_state:

	dec  [check_idle_semaphore]

	mov  ebx,[timer_ticks] ;[0xfdf0]
	mov  [idlemem],ebx
	call change_task

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

	pushad

	mov  [irq_owner+4*0], 1    ; timer
	;mov  [irq_owner+4*1], 1    ; keyboard
	mov  [irq_owner+4*6], 1    ; floppy diskette
	mov  [irq_owner+4*13], 1   ; math co-pros
	mov  [irq_owner+4*14], 1   ; ide I
	mov  [irq_owner+4*15], 1   ; ide II

	; RESERVE PORTS
	mov   edi,1		       ; 0x00-0x2d
	mov   [RESERVED_PORTS],edi
	shl   edi,4
	mov   [RESERVED_PORTS+edi+0],dword 1
	mov   [RESERVED_PORTS+edi+4],dword 0x0
	mov   [RESERVED_PORTS+edi+8],dword 0x2d

	inc   dword [RESERVED_PORTS]	      ; 0x30-0x4d
	mov   edi,[RESERVED_PORTS]
	shl   edi,4
	mov   [RESERVED_PORTS+edi+0],dword 1
	mov   [RESERVED_PORTS+edi+4],dword 0x30
	mov   [RESERVED_PORTS+edi+8],dword 0x4d

	inc   dword [RESERVED_PORTS]	      ; 0x50-0xdf
	mov   edi,[RESERVED_PORTS]
	shl   edi,4
	mov   [RESERVED_PORTS+edi+0],dword 1
	mov   [RESERVED_PORTS+edi+4],dword 0x50
	mov   [RESERVED_PORTS+edi+8],dword 0xdf

	inc   dword [RESERVED_PORTS]	      ; 0xe5-0xff
	mov   edi,[RESERVED_PORTS]
	shl   edi,4
	mov   [RESERVED_PORTS+edi+0],dword 1
	mov   [RESERVED_PORTS+edi+4],dword 0xe5
	mov   [RESERVED_PORTS+edi+8],dword 0xff

	popad
	ret

setirqreadports:

	mov   [irq12read+0],dword 0x60 + 0x01000000  ; read port 0x60 , byte
	mov   [irq12read+4],dword 0		     ; end of port list
	;mov   [irq04read+0],dword 0x3f8 + 0x01000000 ; read port 0x3f8 , byte
	;mov   [irq04read+4],dword 0                  ; end of port list
	;mov   [irq03read+0],dword 0x2f8 + 0x01000000 ; read port 0x2f8 , byte
	;mov   [irq03read+4],dword 0                  ; end of port list

	ret

iglobal
  process_number dd 0x1
endg

set_variables:

	mov   ecx,0x100 		      ; flush port 0x60
.fl60:	in    al,0x60
	loop  .fl60
	mov   [MOUSE_BUFF_COUNT],byte 0 		; mouse buffer
	mov   [KEY_COUNT],byte 0		 ; keyboard buffer
	mov   [BTN_COUNT],byte 0		 ; button buffer
;        mov   [MOUSE_X],dword 100*65536+100    ; mouse x/y

	push  eax
	mov   ax,[BOOT_VAR+0x900c]
	shr   ax,1
	shl   eax,16
	mov   ax,[BOOT_VAR+0x900A]
	shr   ax,1
	mov   [MOUSE_X],eax
	pop   eax

        mov   [BTN_ADDR],dword BUTTON_INFO    ; address of button list

     ;!! IP 04.02.2005:
	mov   [next_usage_update], 100
	mov   byte [DONT_SWITCH], 0 ; change task if possible

	ret

;* mouse centered - start code- Mario79
mouse_centered:
	push  eax
	mov   eax,[Screen_Max_X]
	shr   eax,1
	mov   [MOUSE_X],ax
	mov   eax,[Screen_Max_Y]
	shr   eax,1
	mov   [MOUSE_Y],ax
	pop   eax
	ret
;* mouse centered - end code- Mario79

align 4

sys_outport:

    mov   edi,ebx	   ; separate flag for read / write
    and   ebx,65535

    mov   ecx,[RESERVED_PORTS]
    test  ecx,ecx
    jne   sopl8
    mov   [esp+36],dword 1
    ret

  sopl8:
    mov   edx,[TASK_BASE]
    mov   edx,[edx+0x4]
    and   ebx,65535
    cld
  sopl1:

    mov   esi,ecx
    shl   esi,4
    add   esi,RESERVED_PORTS
    cmp   edx,[esi+0]
    jne   sopl2
    cmp   ebx,[esi+4]
    jb	  sopl2
    cmp   ebx,[esi+8]
    jg	  sopl2
    jmp   sopl3

  sopl2:

    dec   ecx
    jnz   sopl1
    mov   [esp+36],dword 1
    ret

  sopl3:

    test  edi,0x80000000 ; read ?
    jnz   sopl4

    mov   dx,bx 	 ; write
    out   dx,al
    mov   [esp+36],dword 0
    ret

  sopl4:

    mov   dx,bx 	 ; read
    in	  al,dx
    and   eax,0xff
    mov   [esp+36],dword 0
    mov   [esp+24],eax
    ret

display_number:

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
	xor	edi, edi
display_number_force:
     push  eax
     and   eax,0x3fffffff
     cmp   eax,0xffff		 ; length > 0 ?
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

     cmp   ah,0 		 ; DECIMAL
     jne   no_display_desnum
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


     mov  [esp+36],dword 0
     cmp  eax,1 		     ; MIDI
     jnz  nsyse1
     cmp  ebx,0x100
     jb   nsyse1
     mov  edx,65535
     cmp  edx,ebx
     jb   nsyse1
     mov  [midi_base],bx
     mov  word [mididp],bx
     inc  bx
     mov  word [midisp],bx
     ret

iglobal
midi_base dw 0
endg

   nsyse1:

     cmp  eax,2 		     ; KEYBOARD
     jnz  nsyse2
     cmp  ebx,1
     jnz  kbnobase
     mov  edi,[TASK_BASE]
     add  ecx,[edi+TASKDATA.mem_start]
     mov  eax,ecx
     mov  ebx,keymap
     mov  ecx,128
     call memmove
     ret
   kbnobase:
     cmp  ebx,2
     jnz  kbnoshift
     mov  edi,[TASK_BASE]
     add  ecx,[edi+TASKDATA.mem_start]
     mov  eax,ecx
     mov  ebx,keymap_shift
     mov  ecx,128
     call memmove
     ret
   kbnoshift:
     cmp  ebx,3
     jne  kbnoalt
     mov  edi,[TASK_BASE]
     add  ecx,[edi+TASKDATA.mem_start]
     mov  eax,ecx
     mov  ebx,keymap_alt
     mov  ecx,128
     call memmove
     ret
   kbnoalt:
     cmp  ebx,9
     jnz  kbnocountry
     mov  word [keyboard],cx
     ret
   kbnocountry:
     mov  [esp+36],dword 1
     ret
   nsyse2:
     cmp  eax,3 		     ; CD
     jnz  nsyse4
     test ebx,ebx
     jz   nosesl
     cmp  ebx, 4
     ja   nosesl
     mov  [cd_base],bl
     cmp  ebx,1
     jnz  noprma
     mov  [cdbase],0x1f0
     mov  [cdid],0xa0
   noprma:
     cmp  ebx,2
     jnz  noprsl
     mov  [cdbase],0x1f0
     mov  [cdid],0xb0
   noprsl:
     cmp  ebx,3
     jnz  nosema
     mov  [cdbase],0x170
     mov  [cdid],0xa0
   nosema:
     cmp  ebx,4
     jnz  nosesl
     mov  [cdbase],0x170
     mov  [cdid],0xb0
   nosesl:
     ret

cd_base db 0

   nsyse4:

     cmp  eax,5 		     ; SYSTEM LANGUAGE
     jnz  nsyse5
     mov  [syslang],ebx
     ret
   nsyse5:

     cmp  eax,7 		     ; HD BASE
     jne  nsyse7
     test ebx,ebx
     jz   nosethd
     cmp  ebx,4
     ja   nosethd
     mov  [hd_base],bl
     cmp  ebx,1
     jnz  noprmahd
     mov  [hdbase],0x1f0
     mov  [hdid],0x0
     mov  [hdpos],1
;     call set_FAT32_variables
   noprmahd:
     cmp  ebx,2
     jnz  noprslhd
     mov  [hdbase],0x1f0
     mov  [hdid],0x10
     mov  [hdpos],2
;     call set_FAT32_variables
   noprslhd:
     cmp  ebx,3
     jnz  nosemahd
     mov  [hdbase],0x170
     mov  [hdid],0x0
     mov  [hdpos],3
;     call set_FAT32_variables
   nosemahd:
     cmp  ebx,4
     jnz  noseslhd
     mov  [hdbase],0x170
     mov  [hdid],0x10
     mov  [hdpos],4
;     call set_FAT32_variables
   noseslhd:
    call  reserve_hd1
    call  reserve_hd_channel
    call  free_hd_channel
    mov   [hd1_status],0	; free
   nosethd:
     ret

iglobal
hd_base db 0
endg

nsyse7:

     cmp  eax,8 		     ; HD PARTITION
     jne  nsyse8
     mov  [fat32part],ebx
;     call set_FAT32_variables
    call  reserve_hd1
    call  reserve_hd_channel
    call  free_hd_channel
     pusha
     call  choice_necessity_partition_1
     popa
    mov   [hd1_status],0	; free
     ret

nsyse8:
     cmp  eax,11		     ; ENABLE LBA READ
     jne  no_set_lba_read
     and  ebx,1
     mov  [lba_read_enabled],ebx
     ret

no_set_lba_read:
     cmp  eax,12                     ; ENABLE PCI ACCESS
     jne  no_set_pci_access
     and  ebx,1
     mov  [pci_access_enabled],ebx
     ret
   no_set_pci_access:

;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
include 'vmodeint.inc'
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

sys_setup_err:
     mov  [esp+36],dword -1
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

     cmp  eax,1
     jne  ngsyse1
     movzx eax,[midi_base]
     mov  [esp+36],eax
     ret
ngsyse1:

     cmp  eax,2
     jne  ngsyse2
     cmp  ebx,1
     jnz  kbnobaseret
     mov  edi,[TASK_BASE]
     add  ecx,[edi+TASKDATA.mem_start]
     mov  ebx,ecx
     mov  eax,keymap
     mov  ecx,128
     call memmove
     ret
kbnobaseret:
     cmp  ebx,2
     jnz  kbnoshiftret
     mov  edi,[TASK_BASE]
     add  ecx,[edi+TASKDATA.mem_start]
     mov  ebx,ecx
     mov  eax,keymap_shift
     mov  ecx,128
     call memmove
     ret
kbnoshiftret:
     cmp  ebx,3
     jne  kbnoaltret
     mov  edi,[TASK_BASE]
     add  ecx,[edi+TASKDATA.mem_start]
     mov  ebx,ecx
     mov  eax,keymap_alt
     mov  ecx,128
     call memmove
     ret
kbnoaltret:
     cmp  ebx,9
     jnz  ngsyse2
     movzx eax,word [keyboard]
     mov  [esp+36],eax
     ret
ngsyse2:

         cmp  eax,3
         jnz  ngsyse3
         movzx eax,[cd_base]
         mov  [esp+36],eax
         ret
ngsyse3:
         cmp  eax,5
         jnz  ngsyse5
         mov  eax,[syslang]
         mov  [esp+36],eax
         ret
ngsyse5:
     cmp  eax,7
     jnz  ngsyse7
     movzx eax,[hd_base]
     mov  [esp+36],eax
     ret
ngsyse7:
     cmp  eax,8
     jnz  ngsyse8
     mov eax,[fat32part]
     mov  [esp+36],eax
     ret
ngsyse8:
     cmp  eax,9
     jne  ngsyse9
     mov  eax,[timer_ticks] ;[0xfdf0]
     mov  [esp+36],eax
     ret
ngsyse9:
     cmp  eax,11
     jnz  ngsyse11
     mov eax,[lba_read_enabled]
     mov  [esp+36],eax
     ret
ngsyse11:
     cmp  eax,12
     jnz  ngsyse12
     mov eax,[pci_access_enabled]
     mov  [esp+36],eax
     ret
ngsyse12:
     mov  [esp+36],dword 1
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

	   cmp eax, 7
	   ja msset
	   jmp [mousefn+eax*4]
msscreen:
	   mov	eax,[MOUSE_X]
	   shl	eax,16
	   mov	ax,[MOUSE_Y]
	   mov	[esp+36],eax
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
	   mov	[esp+36],eax
	   ret
msbutton:
	   movzx eax,byte [BTN_DOWN]
	   mov	[esp+36],eax
	   ret
msz:
	   mov	 edi, [TASK_COUNT]
	   movzx edi, word [WIN_POS + edi*2]
	   cmp	 edi, [CURRENT_TASK]
	   jne	 @f
	   mov	 ax,[MOUSE_SCROLL_H]
	   shl	 eax,16
	   mov	 ax,[MOUSE_SCROLL_V]
	   mov	 [esp+36],eax
	   mov	 [MOUSE_SCROLL_H],word 0
	   mov	 [MOUSE_SCROLL_V],word 0
	   ret
       @@:
	   mov	[esp+36],dword 0
	   ret
msset:
	   ret

app_load_cursor:
      ;     add ebx, new_app_base
	   cmp ebx, OS_BASE
	   jae msset
	   stdcall load_cursor, ebx, ecx
	   mov [esp+36], eax
	   ret

app_set_cursor:
	   stdcall set_cursor, ebx
	   mov [esp+36], eax
	   ret

app_delete_cursor:
	   stdcall delete_cursor, ebx
	   mov [esp+36], eax
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


setuart:

 su1:
   call is_output
   cmp	al,0
   jnz	su1
   mov	dx,word [midisp]
   mov	al,0xff
   out	dx,al
 su2:
   mov	dx,word [midisp]
   mov	al,0xff
   out	dx,al
   call is_input
   cmp	al,0
   jnz	su2
   call get_mpu_in
   cmp	al,0xfe
   jnz	su2
 su3:
   call is_output
   cmp	al,0
   jnz	su3
   mov	dx,word [midisp]
   mov	al,0x3f
   out	dx,al

   ret


align 4

sys_midi:

     cmp  [mididp],0
     jnz  sm0
     mov  [esp+36],dword 1
     ret
   sm0:

     cmp  eax,1
     mov  [esp+36],dword 0
     jnz  smn1
     call setuart
     ret
   smn1:

     cmp  eax,2
     jnz  smn2
   sm10:
     call get_mpu_in
     call is_output
     test al,al
     jnz  sm10
     mov  al,bl
     call put_mpu_out
     ret
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
     mov  [application_table_status],0
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
    mov    [application_table_status],0
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

sysfn_getactive:	; 18.7 = get active window
     mov  eax, [TASK_COUNT]
   movzx  eax, word [WIN_POS + eax*2]
     mov  [esp+32],eax
     ret

sysfn_sound_flag:	; 18.8 = get/set sound_flag
     cmp  ecx,1
     jne  nogetsoundflag
     movzx  eax,byte [sound_flag] ; get sound_flag
     mov  [esp+32],eax
     ret
 nogetsoundflag:
     cmp  ecx,2
     jnz  nosoundflag
     xor  byte [sound_flag], 1
 nosoundflag:
     ret

sysfn_minimize: 	; 18.10 = minimize window
     mov   [window_minimize],1
     ret

sysfn_getdiskinfo:	; 18.11 = get disk info table
     cmp  ecx,1
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
     cmp  ecx,2
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
     mov edi,ebx
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

sysfn_centermouse:	; 18.15 = mouse centered
     call  mouse_centered
     and [esp+32],dword 0
     ret

sysfn_mouse_acceleration: ; 18.19 = set/get mouse features
     cmp  ecx,0  ; get mouse speed factor
     jnz  .set_mouse_acceleration
     xor  eax,eax
     mov  ax,[mouse_speed_factor]
     mov  [esp+32],eax
     ret
 .set_mouse_acceleration:
     cmp  ecx,1  ; set mouse speed factor
     jnz  .get_mouse_delay
     mov  [mouse_speed_factor],dx
     ret
 .get_mouse_delay:
     cmp  ecx,2  ; get mouse delay
     jnz  .set_mouse_delay
     mov  eax,[mouse_delay]
     mov  [esp+32],eax
     ret
 .set_mouse_delay:
     cmp  ecx,3  ; set mouse delay
     jnz  .set_pointer_position
     mov  [mouse_delay],edx
     ret
 .set_pointer_position:
     cmp  ecx,4  ; set mouse pointer position
     jnz  .set_mouse_button
     mov   [MOUSE_Y],dx    ;y
     ror   edx,16
     mov   [MOUSE_X],dx    ;x
     rol   edx,16
     ret
 .set_mouse_button:
     cmp   ecx,5  ; set mouse button features
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
  db 0,7,5,0  ; version 0.7.5.0
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
bgrlock db 0
bgrlockpid dd 0
endg

sys_background:

    cmp   ebx,1 			   ; BACKGROUND SIZE
    jnz   nosb1
    cmp   ecx,0
    je	  sbgrr
    cmp   edx,0
    je	  sbgrr
@@:
	mov	al, 1
	xchg	[bgrlock], al
	test	al, al
	jz	@f
	call	change_task
	jmp	@b
@@:
    mov   [BgrDataWidth],ecx
    mov   [BgrDataHeight],edx
;    mov   [bgrchanged],1

    pushad
; return memory for old background
    stdcall kernel_free, [img_background]
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
    jz .exit_mem
    mov [img_background], eax
.exit_mem:
    popad
	mov	[bgrlock], 0

  sbgrr:
    ret

nosb1:

    cmp   ebx,2 			   ; SET PIXEL
    jnz   nosb2

    mov ebx, [mem_BACKGROUND]
    add ebx, 4095
    and ebx, -4096
    sub ebx, 4
    cmp   ecx, ebx
    ja   @F

    mov   eax,[img_background]
    mov   ebx,[eax+ecx]
    and   ebx,0xFF000000 ;255*256*256*256
    and   edx,0x00FFFFFF ;255*256*256+255*256+255
    add   edx,ebx
    mov   [eax+ecx],edx
@@:
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
    call  force_redraw_background
    mov    [REDRAW_BACKGROUND], byte 2
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
@@:
	mov	al, 1
	xchg	[bgrlock], al
	test	al, al
	jz	@f
	call	change_task
	jmp	@b
@@:
	mov	eax, [CURRENT_TASK]
	mov	[bgrlockpid], eax
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
    mov   [draw_data+32 + RECT.left],dword 0
    mov   [draw_data+32 + RECT.top],dword 0
    push  eax ebx
    mov   eax,[Screen_Max_X]
    mov   ebx,[Screen_Max_Y]
    mov   [draw_data+32 + RECT.right],eax
    mov   [draw_data+32 + RECT.bottom],ebx
    pop   ebx eax
    mov   byte [REDRAW_BACKGROUND], 1
    ret

align 4

sys_getbackground:

    cmp   eax,1 				 ; SIZE
    jnz   nogb1
    mov   eax,[BgrDataWidth]
    shl   eax,16
    mov   ax,[BgrDataHeight]
    mov   [esp+36],eax
    ret

nogb1:

    cmp   eax,2 				 ; PIXEL
    jnz   nogb2

    mov ecx, [mem_BACKGROUND]
    add ecx, 4095
    and ecx, -4096
    sub ecx, 4
    cmp ebx, ecx
    ja  @F

    mov   eax,[img_background]
    mov   eax,[ebx+eax]

    and   eax, 0xFFFFFF
    mov   [esp+36],eax
@@:
    ret
  nogb2:

    cmp   eax,4 				 ; TILED / STRETCHED
    jnz   nogb4
    mov   eax,[BgrDrawMode]
  nogb4:
    mov   [esp+36],eax
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
	shl	eax, 8
; // Alver 22.06.2008 // {
        mov       al, byte [btn_down_determ]
        and       al,0xFE                                       ; delete left button bit
; } \\ Alver \\
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


sys_drawwindow:

    mov   eax,edx
    shr   eax,16+8
    and   eax,15

;    cmp   eax,0   ; type I    - original style
    jne   nosyswI
    inc   [mouse_pause]
    call  [disable_mouse]
    call  sys_set_window
    call  [disable_mouse]
    call  drawwindow_I
    ;dec   [mouse_pause]
    ;call   [draw_pointer]
    ;ret
    jmp   draw_window_caption.2
  nosyswI:

    cmp   al,1	  ; type II   - only reserve area, no draw
    jne   nosyswII
    inc   [mouse_pause]
    call  [disable_mouse]
    call  sys_set_window
    call  [disable_mouse]
    call  sys_window_mouse
    dec   [mouse_pause]
    call   [draw_pointer]
    ret
  nosyswII:

    cmp   al,2	  ; type III  - new style
    jne   nosyswIII
    inc   [mouse_pause]
    call  [disable_mouse]
    call  sys_set_window
    call  [disable_mouse]
    call  drawwindow_III
    ;dec   [mouse_pause]
    ;call   [draw_pointer]
    ;ret
    jmp   draw_window_caption.2
  nosyswIII:

    cmp   al,3	  ; type IV - skinned window
    je	  draw_skin_window
    cmp   al,4	  ; type V - skinned window not sized! {not_sized_skin_window}
    jne   nosyswV
  draw_skin_window:

    inc   [mouse_pause]
    call  [disable_mouse]
    call  sys_set_window
    call  [disable_mouse]
    mov   eax, [TASK_COUNT]
    movzx eax, word [WIN_POS + eax*2]
    cmp   eax, [CURRENT_TASK]
    setz  al
    movzx eax, al
    push  eax
    call  drawwindow_IV
    ;dec   [mouse_pause]
    ;call   [draw_pointer]
    ;ret
    jmp   draw_window_caption.2
  nosyswV:

    ret


draw_window_caption:
	inc	[mouse_pause]
	call	[disable_mouse]

	xor	eax,eax
	mov	edx,[TASK_COUNT]
	movzx	edx,word[WIN_POS+edx*2]
	cmp	edx,[CURRENT_TASK]
	jne	@f
	inc	eax
    @@: mov	edx,[CURRENT_TASK]
	shl	edx,5
	add	edx,window_data
	movzx	ebx,[edx+WDATA.fl_wstyle]
	and	bl,0x0F
	cmp	bl,3
	je	.draw_caption_style_3		;{for 3 and 4 style write caption}
	cmp	bl,4
	je	.draw_caption_style_3

	jmp	.not_style_3
  .draw_caption_style_3:

	push	edx
	call	drawwindow_IV_caption
	add	esp,4
	jmp	.2

  .not_style_3:
	cmp	bl,2
	jne	.not_style_2

	call	drawwindow_III_caption
	jmp	.2

  .not_style_2:
	cmp	bl,0
	jne	.2

	call	drawwindow_I_caption

;--------------------------------------------------------------
  .2:	;jmp     @f
	mov	edi,[CURRENT_TASK]
	shl	edi,5
	test	[edi+window_data+WDATA.fl_wstyle],WSTYLE_HASCAPTION
	jz	@f
	mov	edx,[edi*8+SLOT_BASE+APPDATA.wnd_caption]
	or	edx,edx
	jz	@f

	movzx	eax,[edi+window_data+WDATA.fl_wstyle]
	and	al,0x0F
	cmp	al,3
	je	.skinned
	cmp	al,4
	je	.skinned

	jmp	.not_skinned
  .skinned:
	mov	ebp,[edi+window_data+WDATA.box.left-2]
	mov	bp,word[edi+window_data+WDATA.box.top]
	movzx	eax,word[edi+window_data+WDATA.box.width]
	sub	ax,[_skinmargins.left]
	sub	ax,[_skinmargins.right]
	push	edx
	cwde
	cdq
	mov	ebx,6
	idiv	ebx
	pop	edx
	or	eax,eax
	js	@f
	mov	esi,eax
	mov	ebx,dword[_skinmargins.left-2]
	mov	bx,word[_skinh]
	sub	bx,[_skinmargins.bottom]
	sub	bx,[_skinmargins.top]
	sar	bx,1
	adc	bx,0
	add	bx,[_skinmargins.top]
	add	bx,-3
	add	ebx,ebp
	jmp	.dodraw

  .not_skinned:
	cmp	al,1
	je	@f

	mov	ebp,[edi+window_data+WDATA.box.left-2]
	mov	bp,word[edi+window_data+WDATA.box.top]
	movzx	eax,word[edi+window_data+WDATA.box.width]
	sub	eax,16
	push	edx
	cwde
	cdq
	mov	ebx,6
	idiv	ebx
	pop	edx
	or	eax,eax
	js	@f
	mov	esi,eax
	mov	ebx,0x00080007
	add	ebx,ebp
.dodraw:
	mov	ecx,[common_colours+16];0x00FFFFFF
	or	ecx, 0x80000000
	xor	edi,edi
; // Alver 22.06.2008 // {
;	call	dtext
        call dtext_asciiz_esi
; } \\ Alver \\

    @@:
;--------------------------------------------------------------
	dec	[mouse_pause]
	call	[draw_pointer]
	ret

iglobal
align 4
window_topleft dd \
  1, 21,\		;type 0
  0,  0,\	;type 1
  5, 20,\	;type 2
  5,  ?,\	;type 3 {set by skin}
  5,  ? 	;type 4 {set by skin}
endg

set_window_clientbox:
	push	eax ecx edi

	mov	eax,[_skinh]
	mov	[window_topleft+4*7],eax
	mov	[window_topleft+4*9],eax

	mov	ecx,edi
	sub	edi,window_data
	shl	edi,3
	test	[ecx+WDATA.fl_wstyle],WSTYLE_CLIENTRELATIVE
	jz	@f

	movzx	eax,[ecx+WDATA.fl_wstyle]
	and	eax,0x0F
	mov	eax,[eax*8+window_topleft+0]
	mov	[edi+SLOT_BASE+APPDATA.wnd_clientbox.left],eax
	shl	eax,1
	neg	eax
	add	eax,[ecx+WDATA.box.width]
	mov	[edi+SLOT_BASE+APPDATA.wnd_clientbox.width],eax

	movzx	eax,[ecx+WDATA.fl_wstyle]
	and	eax,0x0F
	push	[eax*8+window_topleft+0]
	mov	eax,[eax*8+window_topleft+4]
	mov	[edi+SLOT_BASE+APPDATA.wnd_clientbox.top],eax
	neg	eax
	sub	eax,[esp]
	add	eax,[ecx+WDATA.box.height]
	mov	[edi+SLOT_BASE+APPDATA.wnd_clientbox.height],eax
	add	esp,4

	pop	edi ecx eax
	ret
    @@:
	xor	eax,eax
	mov	[edi+SLOT_BASE+APPDATA.wnd_clientbox.left],eax
	mov	[edi+SLOT_BASE+APPDATA.wnd_clientbox.top],eax
	mov	eax,[ecx+WDATA.box.width]
	mov	[edi+SLOT_BASE+APPDATA.wnd_clientbox.width],eax
	mov	eax,[ecx+WDATA.box.height]
	mov	[edi+SLOT_BASE+APPDATA.wnd_clientbox.height],eax

	pop	edi ecx eax
	ret

sys_set_window:

    mov   eax,[CURRENT_TASK]
    shl   eax,5
    add   eax,window_data

    ; colors
    mov   [eax+WDATA.cl_workarea],edx
    mov   [eax+WDATA.cl_titlebar],esi
    mov   [eax+WDATA.cl_frames],edi

    mov   edi, eax

    ; check flag (?)
    test  [edi+WDATA.fl_wdrawn],1
    jnz   newd

    mov   eax,[timer_ticks] ;[0xfdf0]
    add   eax,100
    mov   [new_window_starting],eax

    movsx eax,bx
    mov   [edi+WDATA.box.width],eax
    movsx eax,cx
    mov   [edi+WDATA.box.height],eax
    sar   ebx,16
    sar   ecx,16
    mov   [edi+WDATA.box.left],ebx
    mov   [edi+WDATA.box.top],ecx

    call  check_window_position

    call  set_window_clientbox

    push  ecx esi edi		    ; save for window fullscreen/resize
    ;mov   esi,edi

	mov	cl, [edi+WDATA.fl_wstyle]
	mov	eax, [edi+WDATA.cl_frames]

    sub   edi,window_data
    shl   edi,3
    add   edi,SLOT_BASE

	and	cl,0x0F
	mov	[edi+APPDATA.wnd_caption],0
	cmp	cl,3
	je	set_APPDATA_wnd_caption
	cmp	cl,4								; {SPraid.simba}
	je	set_APPDATA_wnd_caption

	jmp	@f
    set_APPDATA_wnd_caption:
	mov	[edi+APPDATA.wnd_caption],eax
    @@: mov	esi,[esp+0]

    add   edi, APPDATA.saved_box
	movsd
	movsd
	movsd
	movsd
    pop   edi esi ecx

	mov	esi, [CURRENT_TASK]
	movzx	esi, word [WIN_STACK+esi*2]
	lea	esi, [WIN_POS+esi*2]
	call	waredraw

;;;    mov   ebx, 1
;;;    call  delay_hs
    mov   eax, [edi+WDATA.box.left]
    mov   ebx, [edi+WDATA.box.top]
    mov   ecx, [edi+WDATA.box.width]
    mov   edx, [edi+WDATA.box.height]
    add   ecx, eax
    add   edx, ebx
    call  calculatescreen

    mov   [KEY_COUNT],byte 0	       ; empty keyboard buffer
    mov   [BTN_COUNT],byte 0	       ; empty button buffer

  newd:
    mov   [edi+WDATA.fl_redraw],byte 0	 ; no redraw
    mov   edx,edi

    ret

syscall_windowsettings:

  .set_window_caption:
	dec	eax	; subfunction #1 - set window caption
	jnz	.get_window_caption

	; NOTE: only window owner thread can set its caption,
	;       so there's no parameter for PID/TID

	mov	edi,[CURRENT_TASK]
	shl	edi,5

	; have to check if caption is within application memory limit
	; check is trivial, and if application resizes its memory,
	;   caption still can become over bounds
; diamond, 31.10.2006: check removed because with new memory manager
; there can be valid data after APPDATA.mem_size bound
;        mov     ecx,[edi*8+SLOT_BASE+APPDATA.mem_size]
;        add     ecx,255 ; max caption length
;        cmp     ebx,ecx
;        ja      .exit_fail

	mov	[edi*8+SLOT_BASE+APPDATA.wnd_caption],ebx
	or	[edi+window_data+WDATA.fl_wstyle],WSTYLE_HASCAPTION

	call	draw_window_caption

	xor	eax,eax ; eax = 0 (success)
	ret

  .get_window_caption:
	dec	eax	; subfunction #2 - get window caption
	jnz	.exit_fail

	; not implemented yet

  .exit_fail:
	xor	eax,eax
	inc	eax	; eax = 1 (fail)
	ret


sys_window_move:

	mov	edi,[CURRENT_TASK]
	shl	edi,5
	add	edi,window_data

	test	[edi+WDATA.fl_wstate],WSTATE_MAXIMIZED
	jnz	.window_move_return

	push	dword [edi + WDATA.box.left]  ; save old coordinates
	push	dword [edi + WDATA.box.top]
	push	dword [edi + WDATA.box.width]
	push	dword [edi + WDATA.box.height]

	cmp   eax,-1		      ; set new position and size
	je    .no_x_reposition
	mov	[edi + WDATA.box.left], eax
      .no_x_reposition:
	cmp   ebx,-1
	je    .no_y_reposition
	mov	[edi + WDATA.box.top], ebx
      .no_y_reposition:

	test	[edi+WDATA.fl_wstate],WSTATE_ROLLEDUP
	jnz	.no_y_resizing

	cmp   ecx,-1
	je    .no_x_resizing
	mov	[edi + WDATA.box.width], ecx
      .no_x_resizing:
	cmp   edx,-1
	je    .no_y_resizing
	mov	[edi + WDATA.box.height], edx
      .no_y_resizing:

	call  check_window_position
	call  set_window_clientbox

	pushad			     ; save for window fullscreen/resize
	mov   esi,edi
	sub   edi,window_data
	shr   edi,5
	shl   edi,8
	add   edi, SLOT_BASE + APPDATA.saved_box
	mov   ecx,4
	cld
	rep   movsd
	popad

	pushad			     ; calculcate screen at new position
	mov   eax, [edi + WDATA.box.left]
	mov   ebx, [edi + WDATA.box.top]
	mov   ecx, [edi + WDATA.box.width]
	mov   edx, [edi + WDATA.box.height]
	add   ecx,eax
	add   edx,ebx

	call  calculatescreen
	popad

	pop   edx		    ; calculcate screen at old position
	pop   ecx
	pop   ebx
	pop   eax
	add   ecx,eax
	add   edx,ebx
	mov   [dlx],eax 	    ; save for drawlimits
	mov   [dly],ebx
	mov   [dlxe],ecx
	mov   [dlye],edx
	call  calculatescreen

	mov   [edi + WDATA.fl_redraw], 1 ; flag the process as redraw

	mov   eax,edi		    ; redraw screen at old position
	xor   esi,esi
	call  redrawscreen

	mov   [DONT_DRAW_MOUSE],byte 0 ; mouse pointer
	mov   [MOUSE_BACKGROUND],byte 0 ; no mouse under
	mov   [MOUSE_DOWN],byte 0 ; react to mouse up/down

	call  [draw_pointer]

	mov   [window_move_pr],0

      .window_move_return:

	ret

uglobal
  window_move_pr   dd  0x0
  window_move_eax  dd  0x0
  window_move_ebx  dd  0x0
  window_move_ecx  dd  0x0
  window_move_edx  dd  0x0
endg

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
sys_sheduler: ;noname & halyavin
    cmp eax,0
    je shed_counter
    cmp eax,2
    je perf_control
    cmp eax,3
    je rdmsr_instr
    cmp eax,4
    je wrmsr_instr
    cmp eax,1
    jne not_supported
    call change_task ;delay,0
ret
shed_counter:
    mov eax,[context_counter]
    mov [esp+36],eax
not_supported:
ret
perf_control:
    inc eax ;now eax=3
    cmp ebx,eax
    je cache_disable
    dec eax
    cmp ebx,eax
    je cache_enable
    dec eax
    cmp ebx,eax
    je is_cache_enabled
    dec eax
    cmp ebx,eax
    je modify_pce
ret

rdmsr_instr:
;now counter in ecx
;(edx:eax) esi:edi => edx:esi
mov eax,esi
rdmsr
mov [esp+36],eax
mov [esp+24],edx ;ret in ebx?
ret

wrmsr_instr:
;now counter in ecx
;(edx:eax) esi:edi => edx:esi
	; Fast Call MSR can't be destroy
	; Ќо MSR_AMD_EFER можно измен€ть, т.к. в этом регистре лиш
	; включаютс€/выключаютс€ расширенные возможности
	cmp	ecx, MSR_SYSENTER_CS
	je	@f
	cmp	ecx, MSR_SYSENTER_ESP
	je	@f
	cmp	ecx, MSR_SYSENTER_EIP
	je	@f
	cmp	ecx, MSR_AMD_STAR
	je	@f

	mov	eax, esi
	wrmsr
	; mov   [esp + 36], eax
	; mov   [esp + 24], edx ;ret in ebx?
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
       mov [esp+36],ebx
cache_disabled:
       mov dword [esp+36],eax ;0
ret

modify_pce:
       mov eax,cr4
;       mov ebx,0
;       or  bx,100000000b ;pce
;       xor eax,ebx ;invert pce
       bts eax,8 ;pce=cr4[8]
       mov cr4,eax
       mov [esp+36],eax
ret
;---------------------------------------------------------------------------------------------


; check if pixel is allowed to be drawn

checkpixel:
	push eax edx

	mov  edx,[Screen_Max_X]     ; screen x size
	inc  edx
	imul edx, ebx
	mov  dl, [eax+edx+display_data] ; lea eax, [...]

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
    cmp   [REDRAW_BACKGROUND],byte 0		   ; background update ?
    jz	  nobackgr
    cmp    [background_defined], 0
    jz	  nobackgr
    cmp   [REDRAW_BACKGROUND], byte 2
    jnz   no_set_bgr_event
    xor   edi, edi
    mov   ecx,	[TASK_COUNT]
set_bgr_event:
    add   edi, 256
    or	  [edi+SLOT_BASE+APPDATA.event_mask], 16
    loop  set_bgr_event
no_set_bgr_event:
;    mov   [draw_data+32 + RECT.left],dword 0
;    mov   [draw_data+32 + RECT.top],dword 0
;    mov   eax,[Screen_Max_X]
;    mov   ebx,[Screen_Max_Y]
;    mov   [draw_data+32 + RECT.right],eax
;    mov   [draw_data+32 + RECT.bottom],ebx
    call  drawbackground
    mov   [REDRAW_BACKGROUND],byte 0
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

    call [disable_mouse]

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

	 mov   ecx,[dlye]   ; ecx = area y end     ebx = window y start
	 cmp   ecx,ebx
	 jb    ricino

	 mov   ecx,[dlxe]   ; ecx = area x end     eax = window x start
	 cmp   ecx,eax
	 jb    ricino

	 mov   eax, [edi + WDATA.box.left]
	 mov   ebx, [edi + WDATA.box.top]
	 mov   ecx, [edi + WDATA.box.width]
	 mov   edx, [edi + WDATA.box.height]
	 add   ecx, eax
	 add   edx, ebx

	 mov   eax,[dly]    ; eax = area y start     edx = window y end
	 cmp   edx,eax
	 jb    ricino

	 mov   eax,[dlx]    ; eax = area x start     ecx = window x end
	 cmp   ecx,eax
	 jb    ricino

	bgli:

	 cmp   ecx,1
	 jnz   .az
	 mov   al,[REDRAW_BACKGROUND]
	 cmp   al,2
	 jz    newdw8
	 test  al,al
	 jz    .az
	 lea   eax,[edi+draw_data-window_data]
	 mov   ebx,[dlx]
	 cmp   ebx,[eax+RECT.left]
	 jae   @f
	 mov   [eax+RECT.left],ebx
	@@:
	 mov   ebx,[dly]
	 cmp   ebx,[eax+RECT.top]
	 jae   @f
	 mov   [eax+RECT.top],ebx
	@@:
	 mov   ebx,[dlxe]
	 cmp   ebx,[eax+RECT.right]
	 jbe   @f
	 mov   [eax+RECT.right],ebx
	@@:
	 mov   ebx,[dlye]
	 cmp   ebx,[eax+RECT.bottom]
	 jbe   @f
	 mov   [eax+RECT.bottom],ebx
	@@:
	 jmp   newdw8
	.az:

	 mov   eax,edi
	 add   eax,draw_data-window_data

	 mov   ebx,[dlx]	  ; set limits
	 mov   [eax + RECT.left], ebx
	 mov   ebx,[dly]
	 mov   [eax + RECT.top], ebx
	 mov   ebx,[dlxe]
	 mov   [eax + RECT.right], ebx
	 mov   ebx,[dlye]
	 mov   [eax + RECT.bottom], ebx

	 sub   eax,draw_data-window_data

	 cmp   dword [esp],1
	 jne   nobgrd
	 mov   byte [REDRAW_BACKGROUND], 1

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

	; all black

	mov   edi, [img_background]  ;IMG_BACKGROUND                 ; set background to black
	xor   eax, eax
	mov   ecx, 1023    ;0x0fff00 / 4
	cld
	rep   stosd

	mov   edi,display_data		    ; set os to use all pixels
	mov   eax,0x01010101
	mov   ecx,1280*1024 / 4
	rep   stosd

	mov   byte [REDRAW_BACKGROUND], 0	       ; do not draw background!

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
	mov	[edi + TASKDATA.event_mask], ebx
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

    mov   eax, [TASK_BASE]
    add   ebx, [eax + TASKDATA.mem_start]

    cmp   ecx, 16
    jae   .not_owner
    mov   edi, [eax + TASKDATA.pid]
    cmp   edi, [irq_owner + 4 * ecx]
    je	  .spril1
.not_owner:
    xor   ecx, ecx
    inc   ecx
    jmp   .end
  .spril1:

    shl   ecx, 6
    mov   esi, ebx
    lea   edi, [irq00read + ecx]
    push  16
    pop   ecx

    cld
    rep   movsd
  .end:
    mov   [esp+32], ecx
    ret


align 4

get_irq_data:
     movzx esi, bh			 ; save number of subfunction, if bh = 1, return data size, otherwise, read data
     xor   bh, bh
     cmp   ebx, 16
     jae   .not_owner
     mov   edx, [4 * ebx + irq_owner]	 ; check for irq owner

     mov   eax,[TASK_BASE]

     cmp   edx,[eax+TASKDATA.pid]
     je    gidril1
.not_owner:
     xor   edx, edx
     dec   edx
     jmp   gid1

  gidril1:

     shl   ebx, 12
     lea   eax, [ebx + IRQ_SAVE]	 ; calculate address of the beginning of buffer + 0x0 - data size
     mov   edx, [eax]			 ;                                              + 0x4 - data offset
     dec   esi
     jz    gid1
     test  edx, edx			 ; check if buffer is empty
     jz    gid1

     mov   ebx, [eax + 0x4]
     mov   edi, ecx

     mov   ecx, 4000			 ; buffer size, used frequently

     cmp   ebx, ecx			 ; check for the end of buffer, if end of buffer, begin cycle again
     jb    @f

     xor   ebx, ebx

   @@:

     lea   esi, [ebx + edx]		 ; calculate data size and offset
     cld
     cmp   esi, ecx			 ; if greater than the buffer size, begin cycle again
     jbe   @f

     sub   ecx, ebx
     sub   edx, ecx

     lea   esi, [eax + ebx + 0x10]
     rep   movsb

     xor   ebx, ebx
   @@:
     lea   esi, [eax + ebx + 0x10]
     mov   ecx, edx
     add   ebx, edx

     rep   movsb
     mov   edx, [eax]
     mov   [eax], ecx			 ; set data size to zero
     mov   [eax + 0x4], ebx		 ; set data offset

   gid1:
     mov   [esp+32], edx		 ; eax
     ret


set_io_access_rights:

     pushad

     mov edi, tss._io_map_0

;     mov   ecx,eax
;     and   ecx,7    ; offset in byte

;     shr   eax,3    ; number of byte
;     add   edi,eax

;     mov   ebx,1
;     shl   ebx,cl

     cmp   ebp,0		; enable access - ebp = 0
     jne   siar1

;     not   ebx
;     and   [edi],byte bl
     btr [edi], eax

     popad

     ret

siar1:

     bts [edi], eax
  ;  or    [edi],byte bl        ; disable access - ebp = 1

     popad

     ret

r_f_port_area:

     test  eax, eax
     jnz   free_port_area
;     je    r_port_area
;     jmp   free_port_area

;   r_port_area:

     pushad

     cmp   ebx,ecx	      ; beginning > end ?
     ja    rpal1
     cmp   ecx,65536
     jae   rpal1
     mov   esi,[RESERVED_PORTS]
     test  esi,esi	      ; no reserved areas ?
     je    rpal2
     cmp   esi,255	      ; max reserved
     jae   rpal1
 rpal3:
     mov   edi,esi
     shl   edi,4
     add   edi,RESERVED_PORTS
     cmp   ebx,[edi+8]
     ja    rpal4
     cmp   ecx,[edi+4]
     jae   rpal1
;     jb    rpal4
;     jmp   rpal1
 rpal4:

     dec   esi
     jnz   rpal3
     jmp   rpal2
   rpal1:
     popad
     mov   eax,1
     ret

   rpal2:
     popad


     ; enable port access at port IO map
     cli
     pushad			   ; start enable io map

     cmp   ecx,65536 ;16384
     jae   no_unmask_io ; jge

     mov   eax,ebx

   new_port_access:

     pushad

     xor   ebp,ebp		  ; enable - eax = port
     call  set_io_access_rights

     popad

     inc   eax
     cmp   eax,ecx
     jbe   new_port_access

   no_unmask_io:

     popad			   ; end enable io map
     sti

     mov   edi,[RESERVED_PORTS]
     add   edi,1
     mov   [RESERVED_PORTS],edi
     shl   edi,4
     add   edi,RESERVED_PORTS
     mov   esi,[TASK_BASE]
     mov   esi,[esi+TASKDATA.pid]
     mov   [edi],esi
     mov   [edi+4],ebx
     mov   [edi+8],ecx

     xor   eax, eax
     ret

free_port_area:

     pushad

     mov   esi,[RESERVED_PORTS]     ; no reserved areas ?
     test  esi,esi
     je    frpal2
     mov   edx,[TASK_BASE]
     mov   edx,[edx+TASKDATA.pid]
   frpal3:
     mov   edi,esi
     shl   edi,4
     add   edi,RESERVED_PORTS
     cmp   edx,[edi]
     jne   frpal4
     cmp   ebx,[edi+4]
     jne   frpal4
     cmp   ecx,[edi+8]
     jne   frpal4
     jmp   frpal1
   frpal4:
     dec   esi
     jnz   frpal3
   frpal2:
     popad
     mov   eax,1
     ret
   frpal1:
     mov   ecx,256
     sub   ecx,esi
     shl   ecx,4
     mov   esi,edi
     add   esi,16
     cld
     rep   movsb

     dec   dword [RESERVED_PORTS]

     popad


     ; disable port access at port IO map

     pushad			   ; start disable io map

     cmp   ecx,65536 ;16384
     jge   no_mask_io

     mov   eax,ebx

   new_port_access_disable:

     pushad

     mov   ebp,1		  ; disable - eax = port
     call  set_io_access_rights

     popad

     inc   eax
     cmp   eax,ecx
     jbe   new_port_access_disable

   no_mask_io:

     popad			   ; end disable io map

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
       cmp  [SCR_MODE],word 0x13
       je   dbrv20
       call  vesa12_drawbackground
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
;        call    [disable_mouse] ; this will be done in xxx_putimage
;        mov     eax, vga_putimage
	cmp	[SCR_MODE], word 0x12
	jz	@f   ;.doit
	mov	eax, vesa12_putimage
	cmp	[SCR_MODE], word 0100000000000000b
	jae	@f
	cmp	[SCR_MODE], word 0x13
	jnz	.doit
@@:
	mov	eax, vesa20_putimage
.doit:
	inc	[mouse_pause]
	call	eax
	dec	[mouse_pause]
	pop	ebp esi ebp
	jmp	[draw_pointer]

syscall_putimage_palette:
	mov	edi, esi
	mov	esi, edx
	mov	edx, ecx
	mov	ecx, ebx
	mov	ebx, eax
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
@@:
        push    ebp esi ebp
        cmp     esi, 8
        jnz     @f
        mov     ebp, putimage_get8bpp
        mov     esi, putimage_init8bpp
        jmp     sys_putimage_bpp
@@:
        cmp     esi, 15
        jnz     @f
        mov     ebp, putimage_get15bpp
        mov     esi, putimage_init15bpp
        jmp     sys_putimage_bpp
@@:
        cmp     esi, 16
        jnz     @f
        mov     ebp, putimage_get16bpp
        mov     esi, putimage_init16bpp
        jmp     sys_putimage_bpp
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
        push    ebp esi ebp
        mov     ebp, putimage_get1bpp
        mov     esi, putimage_init1bpp
        jmp     sys_putimage_bpp
put_2bit_image:
        push    ebp esi ebp
        mov     ebp, putimage_get2bpp
        mov     esi, putimage_init2bpp
        jmp     sys_putimage_bpp
put_4bit_image:
        push    ebp esi ebp
        mov     ebp, putimage_get4bpp
        mov     esi, putimage_init4bpp
        jmp     sys_putimage_bpp

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
        add     eax, ecx
        push    ecx
        add     ecx, 3
        add     eax, 3
        shr     ecx, 2
        shr     eax, 2
        sub     eax, ecx
        pop     ecx
        ret
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
        add     eax, eax
        ret
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
    cmp  [SCR_MODE],word 0100000000000000b
    jge  dbv20
    cmp  [SCR_MODE],word 0x13
    je	 dbv20
    call vesa12_drawbar
    dec   [mouse_pause]
    call   [draw_pointer]
    ret
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


_rdtsc:
     bt [cpu_caps], CAPS_TSC
     jnc ret_rdtsc
     rdtsc
     ret
   ret_rdtsc:
     mov   edx,0xffffffff
     mov   eax,0xffffffff
     ret

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



sys_process_def:
	mov	edi, [CURRENT_TASK]

	dec	eax		; 1 = set keyboard mode
     jne   no_set_keyboard_setup

     shl   edi,8
     mov   [edi+SLOT_BASE + APPDATA.keyboard_mode],bl

     ret

   no_set_keyboard_setup:

	dec	eax		; 2 = get keyboard mode
     jne   no_get_keyboard_setup

     shl   edi,8
     movzx eax, byte [SLOT_BASE+edi + APPDATA.keyboard_mode]

     mov   [esp+36],eax

     ret

   no_get_keyboard_setup:

	dec	eax		; 3 = get keyboard ctrl, alt, shift
     jne   no_get_keyboard_cas

;     xor   eax,eax
;     movzx eax,byte [shift]
;     movzx ebx,byte [ctrl]
;     shl   ebx,2
;     add   eax,ebx
;     movzx ebx,byte [alt]
;     shl   ebx,3
;     add   eax,ebx

 ;// mike.dld [
     mov   eax, [kb_state]
 ;// mike.dld ]

     mov   [esp+36],eax

     ret

   no_get_keyboard_cas:

	dec	eax
	jnz	no_add_keyboard_hotkey

	mov	eax, hotkey_list
@@:
	cmp	dword [eax+8], 0
	jz	.found_free
	add	eax, 16
	cmp	eax, hotkey_list+16*256
	jb	@b
	mov	dword [esp+36], 1
	ret
.found_free:
	mov	[eax+8], edi
	mov	[eax+4], ecx
	movzx	ebx, bl
	lea	ebx, [hotkey_scancodes+ebx*4]
	mov	ecx, [ebx]
	mov	[eax], ecx
	mov	[ebx], eax
	mov	[eax+12], ebx
	jecxz	@f
	mov	[ecx+12], eax
@@:
	and	dword [esp+36], 0
	ret

no_add_keyboard_hotkey:

	dec	eax
	jnz	no_del_keyboard_hotkey

	movzx	ebx, bl
	lea	ebx, [hotkey_scancodes+ebx*4]
	mov	eax, [ebx]
.scan:
	test	eax, eax
	jz	.notfound
	cmp	[eax+8], edi
	jnz	.next
	cmp	[eax+4], ecx
	jz	.found
.next:
	mov	eax, [eax]
	jmp	.scan
.notfound:
	mov	dword [esp+36], 1
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
	mov	[esp+36], edx
	ret

no_del_keyboard_hotkey:
     ret


align 4

sys_gs: 			; direct screen access

     cmp  eax,1 		; resolution
     jne  no_gs1
     mov  eax,[Screen_Max_X]
     shl  eax,16
     mov  ax,[Screen_Max_Y]
     add  eax,0x00010001
     mov  [esp+36],eax
     ret
   no_gs1:

     cmp   eax,2		; bits per pixel
     jne   no_gs2
     movzx eax,byte [ScreenBPP]
     mov   [esp+36],eax
     ret
   no_gs2:

     cmp   eax,3		; bytes per scanline
     jne   no_gs3
     mov   eax,[BytesPerScanLine]
     mov   [esp+36],eax
     ret
   no_gs3:

     mov  [esp+36],dword -1
     ret


align 4 ; PCI functions

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
	call	[disable_mouse]
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

syscall_cdaudio:			; CD

	cmp	eax, 4
	jb	.audio
	jz	.eject
	cmp	eax, 5
	jnz	.ret
.load:
	call	.reserve
	call	LoadMedium
	call	.free
	ret
.eject:
	call	.reserve
	call	clear_CD_cache
	call	allow_medium_removal
	call	EjectMedium
	call	.free
	ret
.audio:
     call  sys_cd_audio
     mov   [esp+36],eax
.ret:
     ret

.reserve:
	call	reserve_cd
	mov	eax, ebx
	shr	eax, 1
	and	eax, 1
	inc	eax
	mov	[ChannelNumber], ax
	mov	eax, ebx
	and	eax, 1
	mov	[DiskNumber], al
	call	reserve_cd_channel
	and	ebx, 3
	inc	ebx
	mov	[cdpos], ebx
	add	ebx, ebx
	mov	cl, 8
	sub	cl, bl
	mov	al, [DRIVE_DATA+1]
	shr	al, cl
	test	al, 2
	jz	.err
	ret
.free:
	call	free_cd_channel
	and	[cd_status], 0
	ret
.err:
	call	.free
	pop	eax
	ret

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
      cmp  [disable_mouse],__sys_disable_mouse
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
	   dec   ebx
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
      cmp  [disable_mouse],__sys_disable_mouse
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
     mov   [esp+36],eax
     ret

align 4

syscall_threads:			; CreateThreads

     call  sys_threads
     mov   [esp+36],eax
     ret

align 4

stack_driver_stat:

     call  app_stack_handler		; Stack status

;     mov   [check_idle_semaphore],5    ; enable these for zero delay
;     call  change_task                 ; between sent packet

     mov   [esp+36],eax
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

	mov [screen_workarea.right],eax
	mov [screen_workarea.bottom], edx
	inc eax
	shl eax, 2			;32 bpp
	mov [BytesPerScanLine], eax
	push ebx
	push esi
	push edi
	call	repos_windows
	mov	eax, 0
	mov	ebx, 0
	mov	ecx, [Screen_Max_X]
	mov	edx, [Screen_Max_Y]
	call	calculatescreen
	pop edi
	pop esi
	pop ebx

	popfd
	ret

; --------------- APM ---------------------
apm_entry    dp    0
apm_vf	      dd    0
align 4
sys_apm:
    cmp    word [apm_vf], 0    ; Check APM BIOS enable
    jne    @f
    or	  [esp + 56], byte 1	; error
    mov    [esp + 36], dword 8	  ; 32-bit protected-mode interface not supported
    ret

@@:
    xchg    eax, ecx
    xchg    ebx, ecx

    cmp    al, 3
    ja	  @f
    and    [esp + 56], byte 0xfe    ; emulate func 0..3 as func 0
    mov    eax, [apm_vf]
    mov    [esp + 36], eax
    shr    eax, 16
    mov    [esp + 32], eax
    ret

@@:

    mov esi, [master_tab+(OS_BASE shr 20)]
    xchg [master_tab], esi
    push esi
    mov edi, cr3
    mov cr3, edi		 ;flush TLB

    call    pword [apm_entry]	 ; call APM BIOS

    xchg eax, [esp]
    mov [master_tab], eax
    mov eax, cr3
    mov cr3, eax
    pop eax

    mov    [esp + 8 ], edi
    mov    [esp + 12], esi
    mov    [esp + 24], ebx
    mov    [esp + 28], edx
    mov    [esp + 32], ecx
    mov    [esp + 36], eax
    setc    al
    and    [esp + 56], byte 0xfe
    or	  [esp + 56], al


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
	   push 3		 ; stop playing cd
	   pop	eax
	   call sys_cd_audio

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

if 1
	   mov	word [OS_BASE+0x467+0],pr_mode_exit
	   mov	word [OS_BASE+0x467+2],0x1000

	   mov	al,0x0F
	   out	0x70,al
	   mov	al,0x05
	   out	0x71,al

	   mov	al,0xFE
	   out	0x64,al

	   hlt

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
	mov	edx, [ebx+64]
	in	ax, dx
	and	ax, 203h
	or	ax, 3C00h
	out	dx, ax
	mov	edx, [ebx+68]
	test	edx, edx
	jz	@f
	in	ax, dx
	and	ax, 203h
	or	ax, 3C00h
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

include "data32.inc"

__REV__ = __REV

uglobals_size = $ - endofcode
diff16 "end of kernel code",0,$

