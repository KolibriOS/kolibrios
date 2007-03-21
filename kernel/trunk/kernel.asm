;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Kolibri OS - based on source code Menuet OS, but not 100% compatible.
;;
;; See file COPYING or GNU.TXT for details with these additional details:
;;     - All code written in 32 bit x86 assembly language
;;     - No external code (eg. bios) at process execution time
;;
;;
;;   Compile with last version FASM
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
include "proc32.inc"
include "kglobals.inc"
include "lang.inc"

include "const.inc"

;WinMapAddress      equ     0x460000
;display_data       = 0x460000

max_processes      equ   255

;window_data        equ   0x0000
;tss_data           equ   0xD20000
tss_step           equ   (128+8192) ; tss & i/o - 65535 ports, * 256=557056*4
;draw_data          equ   0xC00000
;sysint_stack_data  equ   0xC03000

;twdw               equ   (0x3000-window_data)

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
                  jmp   start_of_code

; mike.dld {
        org $+0x10000
db 0
dd servetable-0x10000
draw_line       dd __sys_draw_line
disable_mouse   dd __sys_disable_mouse
draw_pointer    dd __sys_draw_pointer
;//mike.dld, 2006-08-02 [
;drawbar         dd __sys_drawbar
drawbar         dd __sys_drawbar.forced
;//mike.dld, 2006-08-02 ]
putpixel        dd __sys_putpixel
; } mike.dld

version           db    'Kolibri OS  version 0.6.5.0      ',13,10,13,10,0
                  ;dd    endofcode-0x10000

                  ;db   'Boot02'
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
include "boot/preboot.inc"
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

preboot_lfb       db    0
preboot_bootlog   db    0


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                      ;;
;;                      16 BIT INCLUDED FILES                           ;;
;;                                                                      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

include "kernel16.inc"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                      ;;
;;                  SWITCH TO 32 BIT PROTECTED MODE                     ;;
;;                                                                      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

os_data        =  os_data_l-gdts    ; GDTs
os_code        =  os_code_l-gdts
int_code       equ  int_code_l-gdts
int_data       equ  int_data_l-gdts
tss0sys        equ  tss0sys_l-gdts
graph_data     equ  3+graph_data_l-gdts
tss0           equ  tss0_l-gdts
app_code       equ  3+app_code_l-gdts
app_data       equ  3+app_data_l-gdts



; CR0 Flags - Protected mode and Paging

        mov ecx, CR0_PE

; Enabling 32 bit protected mode

        sidt    [cs:old_ints_h-0x10000]

        cli                             ; disable all irqs
        cld
        mov     al,255                  ; mask all irqs
        out     0xa1,al
        out     0x21,al
   l.5: in      al, 0x64                ; Enable A20
        test    al, 2
        jnz     l.5
        mov     al, 0xD1
        out     0x64, al
   l.6: in      al, 0x64
        test    al, 2
        jnz     l.6
        mov     al, 0xDF
        out     0x60, al
   l.7: in      al, 0x64
        test    al, 2
        jnz     l.7
        mov     al, 0xFF
        out     0x64, al
        lgdt    [cs:gdts-0x10000]       ; Load GDT
        mov     eax, cr0                ; Turn on paging // protected mode
        or      eax, ecx
        and     eax, 10011111b *65536*256 + 0xffffff ; caching enabled
        mov     cr0, eax
        jmp     $+2
org $+0x10000
        mov     ax,os_data              ; Selector for os
        mov     ds,ax
        mov     es,ax
        mov     fs,ax
        mov     gs,ax
        mov     ss,ax
        mov     esp,0x3ec00             ; Set stack
        jmp     pword os_code:B32       ; jmp to enable 32 bit mode

if gdte >= $
error 'GDT overlaps with used code!'
end if

use32

include 'unpacker.inc'

__DEBUG__ fix 1
__DEBUG_LEVEL__ fix 1
include 'fdo.inc'

iglobal
  boot_memdetect    db   'Determining amount of memory',0
  boot_fonts        db   'Fonts loaded',0
  boot_tss          db   'Setting TSSs',0
  boot_cpuid        db   'Reading CPUIDs',0
  boot_devices      db   'Detecting devices',0
  boot_timer        db   'Setting timer',0
  boot_irqs         db   'Reprogramming IRQs',0
  boot_setmouse     db   'Setting mouse',0
  boot_windefs      db   'Setting window defaults',0
  boot_bgr          db   'Calculating background',0
  boot_resirqports  db   'Reserving IRQs & ports',0
  boot_setrports    db   'Setting addresses for IRQs',0
  boot_setostask    db   'Setting OS task',0
  boot_allirqs      db   'Unmasking all IRQs',0
  boot_tsc          db   'Reading TSC',0
  boot_pal_ega      db   'Setting EGA/CGA 320x200 palette',0
  boot_pal_vga      db   'Setting VGA 640x480 palette',0
  boot_mtrr         db   'Setting MTRR',0
  boot_tasking      db   'All set - press ESC to start',0
endg

iglobal
  boot_y dd 10
endg

boot_log:
         pushad

         mov   eax,10*65536
         mov   ax,word [boot_y]
         add   [boot_y],dword 10
         mov   ebx,0x80ffffff   ; ASCIIZ string with white color
         mov   ecx,esi
         mov   edi,1
         call  dtext

         mov   [novesachecksum],1000
         call  checkVga_N13

         cmp   [preboot_blogesc],byte 1
         je    .bll2

         cmp   esi,boot_tasking
         jne   .bll2
         ; begin ealex 04.08.05
;         in    al,0x61
;         and   al,01111111b
;         out   0x61,al
         ; end ealex 04.08.05
.bll1:   in    al,0x60    ; wait for ESC key press
         cmp   al,129
         jne   .bll1

.bll2:   popad

         ret

iglobal
  firstapp   db  '/rd/1/LAUNCHER',0
  vrr_m      db  '/rd/1/VRR_M',0
  
  char		dd 0,0,0
  			dd 2560
  			dd  0x3F600 - std_application_base_address
  			db '/RD/1/FONTS/CHAR.MT',0
  char2		dd 0,0,0
  			dd 2560
  			dd  0x3EC00 - std_application_base_address
  			db '/RD/1/FONTS/CHAR2.MT',0

  ;char       db  'FONTS/CHAR.MT',0
  ;char2      db  'FONTS/CHAR2.MT',0
  bootpath   db  '/KOLIBRI    '
  bootpath2  db  0
;  vmode      db  'drivers/VMODE.MDR',0
  vmode		dd 0,0,0
  			dd 0x8000
  			dd 0x760000 - std_application_base_address
  			db '/RD/1/drivers/VMODE.MDR',0
endg


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                      ;;
;;                          32 BIT ENTRY                                ;;
;;                                                                      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4

B32:
; CLEAR 0x280000-0xF00000

        xor   eax,eax
        mov   edi,0x280000
        mov   ecx,(0x100000*0xF-0x280000) / 4
        cld
        rep   stosd
; CLEAR 0x80000-0x90000
;       xor   eax,eax

        mov   edi,0x80000
        mov   ecx,(0x90000-0x80000)/4
;       cld
        rep   stosd

; CLEAR KERNEL UNDEFINED GLOBALS
        mov   edi, endofcode
        mov   ecx, (uglobals_size/4)+4
        rep   stosd

; SAVE & CLEAR 0-0xffff

        mov   esi,0x0000
        mov   edi,0x2F0000
        mov   ecx,0x10000 / 4
        cld
        rep   movsd
        xor   eax,eax
        mov   edi,0
        mov   ecx,0x10000 / 4
        cld
        rep   stosd

; SAVE REAL MODE VARIABLES
        mov     ax, [0x2f0000 + 0x9031]
        mov     [IDEContrRegsBaseAddr], ax
; --------------- APM ---------------------
    mov    eax, [0x2f0000 + 0x9040]    ; entry point
    mov    dword[apm_entry], eax
    mov    word [apm_entry + 4], apm_code_32 - gdts

    mov    eax, [0x2f0000 + 0x9044]    ; version & flags
    mov    [apm_vf], eax
; -----------------------------------------
;        movzx eax,byte [0x2f0000+0x9010]  ; mouse port
;        mov   [0xF604],byte 1  ;al
        mov     al, [0x2F0000+0x901F]   ; DMA writing
        mov     [allow_dma_write], al
        mov   al,[0x2f0000+0x9000]        ; bpp
        mov   [ScreenBPP],al
        movzx eax,word [0x2f0000+0x900A]  ; X max
        dec   eax
        mov   [ScreenWidth],eax
        mov   [screen_workarea.right],eax
        movzx eax,word [0x2f0000+0x900C]  ; Y max
        dec   eax
        mov   [ScreenHeight],eax
        mov   [screen_workarea.bottom],eax
        movzx eax,word [0x2f0000+0x9008]  ; screen mode
        mov   [SCR_MODE],eax
        mov   eax,[0x2f0000+0x9014]       ; Vesa 1.2 bnk sw add
        mov   [BANK_SWITCH],eax
        mov   [BytesPerScanLine],word 640*4         ; Bytes PerScanLine
        cmp   [SCR_MODE],word 0x13          ; 320x200
        je    @f
        cmp   [SCR_MODE],word 0x12          ; VGA 640x480
        je    @f
        mov   ax,[0x2f0000+0x9001]        ; for other modes
        mov   [BytesPerScanLine],ax
      @@:

; GRAPHICS ADDRESSES

        ;mov     eax,0x100000*8                    ; LFB address
        ;cmp     [0xfe0c],word 0x13
        ;je      no_d_lfb
        ;cmp     [0xfe0c],word 0x12
        ;je      no_d_lfb
        ;cmp     [0x2f0000+0x901e],byte 1
        ;jne     no_d_lfb
        mov     byte [0x2f0000+0x901e],0x0
        mov     eax,[0x2f0000+0x9018]
      ;no_d_lfb:
        mov     [LFBAddress],eax

        cmp     [SCR_MODE],word 0100000000000000b
        jge     setvesa20
        cmp     [SCR_MODE],word 0x13
        je      v20ga32
        mov     [PUTPIXEL],dword Vesa12_putpixel24  ; Vesa 1.2
        mov     [0xe024],dword Vesa12_getpixel24
        cmp     [ScreenBPP],byte 24
        jz      ga24
        mov     [PUTPIXEL],dword Vesa12_putpixel32
        mov     [0xe024],dword Vesa12_getpixel32
      ga24:
        jmp     v20ga24
      setvesa20:
        mov     [PUTPIXEL],dword Vesa20_putpixel24  ; Vesa 2.0
        mov     [0xe024],dword Vesa20_getpixel24
        cmp     [ScreenBPP],byte 24
        jz      v20ga24
      v20ga32:
        mov     [PUTPIXEL],dword Vesa20_putpixel32
        mov     [0xe024],dword Vesa20_getpixel32
      v20ga24:
        cmp     [SCR_MODE],word 0x12                ; 16 C VGA 640x480
        jne     no_mode_0x12
        mov     [PUTPIXEL],dword VGA_putpixel
        mov     [0xe024],dword Vesa20_getpixel32
      no_mode_0x12:

           call test_cpu
;           btr [cpu_caps], CAPS_SSE    ;test: dont't use sse code
;           btr [cpu_caps], CAPS_SSE2   ;test: don't use sse2

;           btr [cpu_caps], CAPS_FXSR   ;test: disable sse support
                                        ;all sse commands rise #UD exption
;           btr [cpu_caps], CAPS_PSE    ;test: don't use large pages
;           btr [cpu_caps], CAPS_PGE    ;test: don't use global pages
;           btr [cpu_caps], CAPS_MTRR   ;test: don't use MTRR
           bts [cpu_caps], CAPS_TSC     ;force use rdtsc

; -------- Fast System Call init ----------
; Intel SYSENTER/SYSEXIT (AMD CPU support it too)
           bt [cpu_caps], CAPS_SEP
           jnc .SEnP   ; SysEnter not Present
           xor edx, edx
           mov ecx, MSR_SYSENTER_CS
           mov eax, os_code
           wrmsr
           mov ecx, MSR_SYSENTER_ESP
           mov eax, sysenter_stack ; Check it
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
	; Bit 31–16 During the SYSRET instruction, this field is copied into the CS register
	;  and the contents of this field, plus 8, are copied into the SS register.
	; Bit 15–0 During the SYSCALL instruction, this field is copied into the CS register
	;  and the contents of this field, plus 8, are copied into the SS register.

	; mov	edx, (os_code + 16) * 65536 + os_code
           mov edx, 0x1B0013

           mov eax, syscall_entry
           mov ecx, MSR_AMD_STAR
           wrmsr
.noSYSCALL:
; -----------------------------------------



; MEMORY MODEL
           call mem_test
           call init_mtrr
           call init_mem
           call init_page_map

; ENABLE PAGING
           mov eax, sys_pgdir
           mov cr3, eax

           mov eax,cr0
           or eax,CR0_PG
           mov cr0,eax

           call init_kernel_heap
           stdcall kernel_alloc, RING0_STACK_SIZE+512
           mov [os_stack], eax

           call init_LFB
           call init_fpu

           call init_malloc

           stdcall alloc_kernel_space, 0x4F000
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

           call init_events

           mov eax, srv.fd-SRV_FD_OFFSET
           mov [srv.fd], eax
           mov [srv.bk], eax

           mov edi, irq_tab
           xor eax, eax
           mov ecx, 16
           rep stosd

;Set base of graphic segment to linear address of LFB
        mov     eax,[LFBAddress]          ; set for gs
        mov     [graph_data_l+2],ax
        shr     eax,16
        mov     [graph_data_l+4],al
        mov     [graph_data_l+7],ah

;!!!!!!!!!!!!!!!!!!!!!!!!!!
include 'detect/disks.inc'
;!!!!!!!!!!!!!!!!!!!!!!!!!!

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

; LOAD FONTS I and II

        mov   [CURRENT_TASK],dword 1
        mov   [TASK_COUNT],dword 1
        mov   [TASK_BASE],dword TASK_DATA

        pushad
        push    eax
        mov		eax,char  - std_application_base_address
        call    file_system_lfn
        mov		eax,char2  - std_application_base_address
        call    file_system_lfn
        pop     eax
        popad

;        mov   esi,char
;        xor   ebx,ebx
;        mov   ecx,2560;26000
;        mov   edx,FONT_I
;        call  fs_RamdiskRead

;        mov   esi,char2
;        xor   ebx,ebx
;        mov   ecx,2560;26000
;        mov   edx,FONT_II
;        call  fs_RamdiskRead

        mov   esi,boot_fonts
        call  boot_log

; PRINT AMOUNT OF MEMORY
        mov     esi, boot_memdetect
        call    boot_log

        movzx   ecx, word [boot_y]
        or      ecx, (10+29*6) shl 16 ; "Determining amount of memory"
        sub     ecx, 10
        mov     edx, 0xFFFFFF
        mov     ebx, [MEM_AMOUNT]
        shr     ebx, 20
        mov     edi, 1
        mov     eax, 0x00040000
        call    display_number_force

; REDIRECT ALL IRQ'S TO INT'S 0x20-0x2f

        mov   esi,boot_irqs
        call  boot_log
        call  rerouteirqs

        mov    esi,boot_tss
        call   boot_log

; BUILD SCHEDULER

        call   build_scheduler ; sys32.inc

; LOAD IDT
        lidt   [cs:idtreg]
        cli

        mov    esi,boot_devices
        call   boot_log
        call   detect_devices

 ; TIMER SET TO 1/100 S

        mov   esi,boot_timer
        call  boot_log
        mov   al,0x34              ; set to 100Hz
        out   0x43,al
        mov   al,0x9b              ; lsb    1193180 / 1193
        out   0x40,al
        mov   al,0x2e              ; msb
        out   0x40,al

; SET MOUSE

        mov   esi,boot_setmouse
        call  boot_log
        call  setmouse

        mov  [pci_access_enabled],1

; SET PRELIMINARY WINDOW STACK AND POSITIONS

        mov   esi,boot_windefs
        call  boot_log
        call  setwindowdefaults

; SET BACKGROUND DEFAULTS

        mov   esi,boot_bgr
        call  boot_log
        call  calculatebackground

; RESERVE SYSTEM IRQ'S JA PORT'S

        mov   esi,boot_resirqports
        call  boot_log
        call  reserve_irqs_ports

; SET PORTS FOR IRQ HANDLERS

        mov  esi,boot_setrports
        call boot_log
        call setirqreadports

; SET UP OS TASK

        mov  esi,boot_setostask
        call boot_log

        mov eax, fpu_data
        mov  dword [SLOT_BASE+APPDATA.fpu_state], eax
        mov  dword [SLOT_BASE+APPDATA.fpu_handler], 0
        mov  dword [SLOT_BASE+APPDATA.sse_handler], 0

        ; name for OS/IDLE process

        mov dword [SLOT_BASE+256+APPDATA.app_name],   dword 'OS/I'
        mov dword [SLOT_BASE+256+APPDATA.app_name+4], dword 'DLE '
        mov edi, [os_stack]
        mov dword [SLOT_BASE+256+APPDATA.pl0_stack], edi
        add edi, RING0_STACK_SIZE
        mov dword [SLOT_BASE+256+APPDATA.fpu_state], edi

        mov esi, fpu_data
        mov ecx, 512/4
        cld
        rep movsd

        mov dword [SLOT_BASE+256+APPDATA.fpu_handler], 0
        mov dword [SLOT_BASE+256+APPDATA.sse_handler], 0

        mov ebx, [def_cursor]
        mov dword [SLOT_BASE+256+APPDATA.cursor], ebx

        mov ebx, SLOT_BASE+256+APP_OBJ_OFFSET
        mov  dword [SLOT_BASE+256+APPDATA.fd_obj], ebx
        mov  dword [SLOT_BASE+256+APPDATA.bk_obj], ebx

        ; task list
        mov  [TASK_DATA+TASKDATA.wnd_number], 1 ; on screen number
        mov  [TASK_DATA+TASKDATA.pid], 1        ; process id number
        mov  [TASK_DATA+TASKDATA.mem_start], 0  ; process base address

        mov  edi,tss_data+tss_step
        mov ecx, (tss_step)/4
        xor eax, eax
        cld
        rep stosd

        mov  edi,tss_data+tss_step
        mov  [edi+TSS._ss0], os_data
        mov  eax,cr3
        mov  [edi+TSS._cr3],eax
        mov  [edi+TSS._eip],osloop
        mov  [edi+TSS._eflags],dword 0x11202 ; sti and resume
        mov eax, [os_stack]
        add eax, RING0_STACK_SIZE
        mov  [edi+TSS._esp], eax
        mov  [edi+TSS._cs],os_code
        mov  [edi+TSS._ss],os_data
        mov  [edi+TSS._ds],os_data
        mov  [edi+TSS._es],os_data
        mov  [edi+TSS._fs],os_data
        mov  [edi+TSS._gs],os_data

        mov  ax,tss0
        ltr  ax

        call init_cursors


; READ TSC / SECOND

        mov   esi,boot_tsc
        call  boot_log
        call  _rdtsc
        mov   ecx,eax
        mov   esi,250               ; wait 1/4 a second
        call  delay_ms
        call  _rdtsc
        sub   eax,ecx
        shl   eax,2
        mov   [CPU_FREQ],eax          ; save tsc / sec
        mov ebx, 1000000
        div ebx
        mov [stall_mcs], eax

; SET VARIABLES

        call  set_variables

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

        mov     esi,_skin_file_default
        mov     edi,_skin_file
        movsd
        movsd
        movsd
        call    load_skin

; LOAD FIRST APPLICATION
        mov   [CURRENT_TASK],dword 1
        mov   [TASK_COUNT],dword 1
        cli
        cmp   byte [0x2f0000+0x9030],1
        jne   no_load_vrr_m

        mov ebp, vrr_m
        xor ebx, ebx
        xor edx, edx
        call fs_execute
        cmp   eax,2                  ; if vrr_m app found (PID=2)
        je    first_app_found

no_load_vrr_m:
        mov ebp, firstapp
        xor ebx, ebx
        xor edx, edx
        call fs_execute
        cmp   eax,2                  ; continue if a process has been loaded
        je    first_app_found
        mov   eax, 0xDEADBEEF        ; otherwise halt
        hlt
first_app_found:
        cli

        ;mov   [TASK_COUNT],dword 2
        mov   [CURRENT_TASK],dword 1       ; set OS task fisrt


; SET KEYBOARD PARAMETERS
        mov   al, 0xf6         ; reset keyboard, scan enabled
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

        mov   al, 0xF3       ; set repeat rate & delay
        call  kb_write
;        call  kb_read
        mov   al, 0 ; 30 250 ;00100010b ; 24 500  ;00100100b  ; 20 500
        call  kb_write
;        call  kb_read
     ;// mike.dld [
        call  set_lights
     ;// mike.dld ]

; START MULTITASKING

        mov   esi,boot_tasking
        call  boot_log

 ;      mov   [ENABLE_TASKSWITCH],byte 1        ; multitasking enabled

; UNMASK ALL IRQ'S

        mov   esi,boot_allirqs
        call  boot_log

        cli                          ;guarantee forbidance of interrupts.
        mov   al,0                   ; unmask all irq's
        out   0xA1,al
        out   0x21,al

        mov   ecx,32

     ready_for_irqs:

        mov   al,0x20                ; ready for irqs
        out   0x20,al
        out   0xa0,al

        loop  ready_for_irqs         ; flush the queue

        stdcall attach_int_handler, dword 1, irq1

;        mov    [dma_hdd],1
        cmp     [IDEContrRegsBaseAddr], 0
        setnz   [dma_hdd]

        sti
        jmp   $                      ; wait here for timer to take control

        ; Fly :)

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
  idlemem               dd   0x0
  idleuse               dd   0x0
  idleusesec            dd   0x0
  check_idle_semaphore  dd   0x0
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

        mov  [irq_owner+4*0],byte 1    ; timer
        mov  [irq_owner+4*1],byte 1    ; keyboard
        mov  [irq_owner+4*5],byte 1    ; sound blaster
        mov  [irq_owner+4*6],byte 1    ; floppy diskette
        mov  [irq_owner+4*13],byte 1   ; math co-pros
        mov  [irq_owner+4*14],byte 1   ; ide I
        mov  [irq_owner+4*15],byte 1   ; ide II
;        movzx eax,byte [0xf604]        ; mouse irq
;        dec   eax
;        add   eax,mouseirqtable
;        movzx eax,byte [eax]
;        shl   eax,2
;        mov   [irq_owner+eax],byte 1


                                       ; RESERVE PORTS
        mov   edi,1                    ; 0x00-0x2d
        mov   [RESERVED_PORTS],edi
        shl   edi,4
        mov   [RESERVED_PORTS+edi+0],dword 1
        mov   [RESERVED_PORTS+edi+4],dword 0x0
        mov   [RESERVED_PORTS+edi+8],dword 0x2d

        inc   dword [RESERVED_PORTS]          ; 0x30-0x4d
        mov   edi,[RESERVED_PORTS]
        shl   edi,4
        mov   [RESERVED_PORTS+edi+0],dword 1
        mov   [RESERVED_PORTS+edi+4],dword 0x30
        mov   [RESERVED_PORTS+edi+8],dword 0x4d

        inc   dword [RESERVED_PORTS]          ; 0x50-0xdf
        mov   edi,[RESERVED_PORTS]
        shl   edi,4
        mov   [RESERVED_PORTS+edi+0],dword 1
        mov   [RESERVED_PORTS+edi+4],dword 0x50
        mov   [RESERVED_PORTS+edi+8],dword 0xdf

        inc   dword [RESERVED_PORTS]          ; 0xe5-0xff
        mov   edi,[RESERVED_PORTS]
        shl   edi,4
        mov   [RESERVED_PORTS+edi+0],dword 1
        mov   [RESERVED_PORTS+edi+4],dword 0xe5
        mov   [RESERVED_PORTS+edi+8],dword 0xff


;        cmp   [0xf604],byte 2          ; com1 mouse -> 0x3f0-0x3ff
;        jne   ripl1
;        inc   dword [0x2d0000]
;        mov   edi,[0x2d0000]
;        shl   edi,4
;        mov   [0x2d0000+edi+0],dword 1
;        mov   [0x2d0000+edi+4],dword 0x3f0
;        mov   [0x2d0000+edi+8],dword 0x3ff
;      ripl1:
;        cmp   [0xf604],byte 3          ; com2 mouse -> 0x2f0-0x2ff
;        jne   ripl2
;        inc   dword [0x2d0000]
;        mov   edi,[0x2d0000]
;        shl   edi,4
;        mov   [0x2d0000+edi+0],dword 1
;        mov   [0x2d0000+edi+4],dword 0x2f0
;        mov   [0x2d0000+edi+8],dword 0x2ff
;      ripl2:

        popad
        ret

iglobal
mouseirqtable   db  12    ; ps2
                db  4     ; com1
                db  3     ; com2
endg

setirqreadports:

        mov   [irq12read+0],dword 0x60 + 0x01000000  ; read port 0x60 , byte
        mov   [irq12read+4],dword 0                  ; end of port list
        mov   [irq04read+0],dword 0x3f8 + 0x01000000 ; read port 0x3f8 , byte
        mov   [irq04read+4],dword 0                  ; end of port list
        mov   [irq03read+0],dword 0x2f8 + 0x01000000 ; read port 0x2f8 , byte
        mov   [irq03read+4],dword 0                  ; end of port list

        ret

iglobal
  process_number dd 0x1
endg

set_variables:

        mov   ecx,0x100                       ; flush port 0x60
.fl60:  in    al,0x60
        loop  .fl60
        mov   [MOUSE_BUFF_COUNT],byte 0                 ; mouse buffer
        mov   [KEY_COUNT],byte 0                 ; keyboard buffer
        mov   [BTN_COUNT],byte 0                 ; button buffer
;        mov   [MOUSE_X],dword 100*65536+100    ; mouse x/y

        push  eax
        mov   ax,[0x2f0000+0x900c]
        shr   ax,1
        shl   eax,16
        mov   ax,[0x2f0000+0x900A]
        shr   ax,1
        mov   [MOUSE_X],eax
        pop   eax

        mov   byte [SB16_Status],0            ; Minazzi Paolo
        mov   [display_data-12],dword 1       ; tiled background
        mov   [BTN_ADDR],dword BUTTON_INFO    ; address of button list

     ;!! IP 04.02.2005:
        mov   [next_usage_update], 100
        mov   byte [0xFFFF], 0 ; change task if possible

        ret

;* mouse centered - start code- Mario79
mouse_centered:
        push  eax
        mov   eax,[ScreenWidth]
        shr   eax,1
        mov   [MOUSE_X],ax
        mov   eax,[ScreenHeight]
        shr   eax,1
        mov   [MOUSE_Y],ax
        pop   eax
        ret
;* mouse centered - end code- Mario79

align 4

sys_outport:

    mov   edi,ebx          ; separate flag for read / write
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
    jb    sopl2
    cmp   ebx,[esi+8]
    jg    sopl2
    jmp   sopl3

  sopl2:

    dec   ecx
    jnz   sopl1
    mov   [esp+36],dword 1
    ret

  sopl3:

    test  edi,0x80000000 ; read ?
    jnz   sopl4

    mov   dx,bx          ; write
    out   dx,al
    mov   [esp+36],dword 0
    ret

  sopl4:

    mov   dx,bx          ; read
    in    al,dx
    and   eax,0xff
    mov   [esp+36],dword 0
    mov   [esp+24],eax
    ret



align 4
sys_sb16:

     cmp  word [sb16],word 0
     jnz  sb16l1
     mov  [esp+36],dword 1
     ret
   sb16l1:
     mov  [esp+36],dword 0
     cmp  eax,1    ; set volume - main
     jnz  sb16l2
     mov  dx,word [sb16]
     add  dx,4
     mov  al,0x22
     out  dx,al
     mov  esi,1
     call delay_ms
     mov  eax,ebx
     inc  edx
     out  dx,al
     ret
   sb16l2:

     cmp  eax,2    ; set volume - cd
     jnz  sb16l3
     mov  dx,word [sb16]
     add  dx,4
     mov  al,0x28
     out  dx,al
     mov  esi,1
     call delay_ms
     mov  eax,ebx
     add  edx,1
     out  dx,al
     ret
   sb16l3:
      mov  [esp+36],dword 2
      ret


align 4

sys_sb16II:

     cmp  word [sb16],word 0
     jnz  IIsb16l1
     mov  [esp+36],dword 1
     ret
   IIsb16l1:

     cmp  eax,1    ; set volume - main
     jnz  IIsb16l2
     ; L
     mov  dx,word [sb16]
     add  dx,4
     mov  al,0x30
     out  dx,al
     mov  eax,ebx
     inc  edx
     out  dx,al
     ; R
     mov  dx,word [sb16]
     add  dx,4
     mov  al,0x31
     out  dx,al
     mov  eax,ebx
     inc  edx
     out  dx,al
     mov  [esp+36],dword 0
     ret
   IIsb16l2:

     cmp  eax,2    ; set volume - cd
     jnz  IIsb16l3
     ; L
     mov  dx,word [sb16]
     add  dx,4
     mov  al,0x36
     out  dx,al
     mov  eax,ebx
     inc  edx
     out  dx,al
     ; R
     mov  dx,word [sb16]
     add  dx,4
     mov  al,0x37
     out  dx,al
     mov  eax,ebx
     inc  edx
     out  dx,al
     mov  [esp+36],dword 0
     ret
   IIsb16l3:

     mov  [esp+36],dword 2
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
        xor     edi, edi
display_number_force:

     cmp   eax,0xffff            ; length > 0 ?
     jge   cont_displ
     ret
   cont_displ:

     cmp   eax,61*0x10000        ; length <= 60 ?
     jb    cont_displ2
     ret
   cont_displ2:

     pushad

     cmp   al,1                  ; ecx is a pointer ?
     jne   displnl1
     mov   ebx,[ebx+std_application_base_address]
   displnl1:
     sub   esp,64

     cmp   ah,0                  ; DECIMAL
     jne   no_display_desnum
     shr   eax,16
     and   eax,0x3f
     push  eax
     mov   edi,esp
     add   edi,4+64-1
     mov   ecx,eax
     mov   eax,ebx
     mov   ebx,10
   d_desnum:
     xor   edx,edx
     div   ebx
     add   dl,48
     mov   [edi],dl
     dec   edi
     loop  d_desnum
     pop   eax
     call  draw_num_text
     add   esp,64
     popad
     ret
   no_display_desnum:

     cmp   ah,0x01               ; HEXADECIMAL
     jne   no_display_hexnum
     shr   eax,16
     and   eax,0x3f
     push  eax
     mov   edi,esp
     add   edi,4+64-1
     mov   ecx,eax
     mov   eax,ebx
     mov   ebx,16
   d_hexnum:
     xor   edx,edx
     div   ebx
     add   edx,hexletters
     mov   dl,[edx]
     mov   [edi],dl
     dec   edi
     loop  d_hexnum
     pop   eax
     call  draw_num_text
     add   esp,64
     popad
     ret
   no_display_hexnum:

     cmp   ah,0x02               ; BINARY
     jne   no_display_binnum
     shr   eax,16
     and   eax,0x3f
     push  eax
     mov   edi,esp
     add   edi,4+64-1
     mov   ecx,eax
     mov   eax,ebx
     mov   ebx,2
   d_binnum:
     xor   edx,edx
     div   ebx
     add   dl,48
     mov   [edi],dl
     dec   edi
     loop  d_binnum
     pop   eax
     call  draw_num_text
     add   esp,64
     popad
     ret
   no_display_binnum:

     add   esp,64
     popad
     ret


draw_num_text:

     ; dtext
     ;
     ; eax x & y
     ; ebx color
     ; ecx start of text
     ; edx length
     ; edi 1 force

;        mov     edi,[CURRENT_TASK]
;        shl     edi,8
;        add     ax,word[edi+SLOT_BASE+APPDATA.wnd_clientbox.top]
;        rol     eax,16
;        add     ax,word[edi+SLOT_BASE+APPDATA.wnd_clientbox.left]
;        rol     eax,16

     mov   edx,eax
     mov   ecx,64+4
     sub   ecx,eax
     add   ecx,esp
     mov   eax,[esp+64+32-8+4]
     push  edx                       ; add window start x & y
     mov   edx,[TASK_BASE]
     mov   ebx,[edx-twdw+WDATA.box.left]
     add   ebx, [(edx-CURRENT_TASK)*8+SLOT_BASE+APPDATA.wnd_clientbox.left]
     shl   ebx,16
     add   ebx,[edx-twdw+WDATA.box.top]
     add   ebx, [(edx-CURRENT_TASK)*8+SLOT_BASE+APPDATA.wnd_clientbox.top]
     add   eax,ebx
     pop   edx
     mov   ebx,[esp+64+32-12+4]
        and     ebx, not 0x80000000     ; force counted string
        mov     esi, [esp+64+4+4]
        mov     edi, [esp+64+4]
     jmp   dtext

read_string:

    ; eax  read_area
    ; ebx  color of letter
    ; ecx  color of background
    ; edx  number of letters to read
    ; esi  [x start]*65536 + [y_start]

    ret


align 4

sys_setup:

; 1=roland mpu midi base , base io address
; 2=keyboard   1, base kaybap 2, shift keymap, 9 country 1eng 2fi 3ger 4rus
; 3=cd base    1, pri.master 2, pri slave 3 sec master, 4 sec slave
; 4=sb16 base , base io address
; 5=system language, 1eng 2fi 3ger 4rus
; 7=hd base    1, pri.master 2, pri slave 3 sec master, 4 sec slave
; 8=fat32 partition in hd
; 9
; 10 = sound dma channel
; 11 = enable lba read
; 12 = enable pci access


     mov  [esp+36],dword 0
     cmp  eax,1                      ; MIDI
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

     cmp  eax,2                      ; KEYBOARD
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
     cmp  eax,3                      ; CD
     jnz  nsyse3
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

   nsyse3:

     cmp  eax,4                      ; SB
     jnz  nsyse4
     cmp  ebx,0x100
     jb   nsyse4
     mov  edx,65535
     cmp  edx,ebx
     jb   nsyse4
     mov  word [sb16],bx
     ret
   nsyse4:

     cmp  eax,5                      ; SYSTEM LANGUAGE
     jnz  nsyse5
     mov  [syslang],ebx
     ret
   nsyse5:

     cmp  eax,7                      ; HD BASE
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
    mov   [hd1_status],0        ; free
   nosethd:
     ret

iglobal
hd_base db 0
endg

   nsyse7:

     cmp  eax,8                      ; HD PARTITION
     jne  nsyse8
     mov  [fat32part],ebx
;     call set_FAT32_variables
    call  reserve_hd1
    call  reserve_hd_channel
    call  free_hd_channel
     pusha
     call  choice_necessity_partition_1
     popa
    mov   [hd1_status],0        ; free
     ret
   nsyse8:

     cmp  eax,10                     ; SOUND DMA CHANNEL
     jne  no_set_sound_dma
     cmp  ebx,3
     ja   sys_setup_err
     mov  [sound_dma],ebx
     ret
   no_set_sound_dma:

     cmp  eax,11                     ; ENABLE LBA READ
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
; 4=sb16 base , base io address
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

     cmp  eax,4
     jne  ngsyse4
     mov  eax,[sb16]
     mov  [esp+36],eax
     ret
   ngsyse4:

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
     cmp  eax,10
     jnz  ngsyse10
     mov eax,[sound_dma]
     mov  [esp+36],eax
     ret
   ngsyse10:
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

iglobal
align 4
mousefn dd msscreen, mswin, msbutton, msset
        dd app_load_cursor
        dd app_set_cursor
        dd app_delete_cursor
endg

readmousepos:

; eax=0 screen relative
; eax=1 window relative
; eax=2 buttons pressed
; eax=3 set mouse pos   ; reserved
; eax=4 load cursor
; eax=5 set cursor
; eax=6 delete cursor   ; reserved

           cmp eax, 6
           ja msset
           jmp [mousefn+eax*4]
msscreen:
           mov  eax,[MOUSE_X]
           shl  eax,16
           mov  ax,[MOUSE_Y]
           mov  [esp+36],eax
           ret
mswin:
           mov  eax,[MOUSE_X]
           shl  eax,16
           mov  ax,[MOUSE_Y]
           mov  esi,[TASK_BASE]
           mov  bx, word [esi-twdw+WDATA.box.left]
           shl  ebx,16
           mov  bx, word [esi-twdw+WDATA.box.top]
           sub  eax,ebx

           mov  edi,[CURRENT_TASK]
           shl  edi,8
           sub  ax,word[edi+SLOT_BASE+APPDATA.wnd_clientbox.top]
           rol  eax,16
           sub  ax,word[edi+SLOT_BASE+APPDATA.wnd_clientbox.left]
           rol  eax,16
           mov  [esp+36],eax
           ret
msbutton:
           movzx eax,byte [BTN_DOWN]
           mov  [esp+36],eax
           ret
msset:
           ret

app_load_cursor:
           add ebx, new_app_base
           cmp ebx, new_app_base
           jb msset
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
   mov  dx,word [midisp]
   in   al,dx
   and  al,0x80
   pop  edx
   ret

is_output:

   push edx
   mov  dx,word [midisp]
   in   al,dx
   and  al,0x40
   pop  edx
   ret


get_mpu_in:

   push edx
   mov  dx,word [mididp]
   in   al,dx
   pop  edx
   ret


put_mpu_out:

   push edx
   mov  dx,word [mididp]
   out  dx,al
   pop  edx
   ret


setuart:

 su1:
   call is_output
   cmp  al,0
   jnz  su1
   mov  dx,word [midisp]
   mov  al,0xff
   out  dx,al
 su2:
   mov  dx,word [midisp]
   mov  al,0xff
   out  dx,al
   call is_input
   cmp  al,0
   jnz  su2
   call get_mpu_in
   cmp  al,0xfe
   jnz  su2
 su3:
   call is_output
   cmp  al,0
   jnz  su3
   mov  dx,word [midisp]
   mov  al,0x3f
   out  dx,al

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
include 'detect/commouse.inc'
include 'detect/ps2mouse.inc'
;include 'detect/dev_fd.inc'
;include 'detect/dev_hdcd.inc'
;include 'detect/sear_par.inc'
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    ret


sys_end:

     mov   eax,[TASK_BASE]
     mov   [eax+TASKDATA.state], 3  ; terminate this program

    waitterm:            ; wait here for termination
     mov   eax,5
     call  delay_hs
     jmp   waitterm

iglobal
align 4
sys_system_table:
        dd      sysfn_shutdown          ; 1 = system shutdown
        dd      sysfn_terminate         ; 2 = terminate thread
        dd      sysfn_activate          ; 3 = activate window
        dd      sysfn_getidletime       ; 4 = get idle time
        dd      sysfn_getcpuclock       ; 5 = get cpu clock
        dd      sysfn_saveramdisk       ; 6 = save ramdisk
        dd      sysfn_getactive         ; 7 = get active window
        dd      sysfn_sound_flag        ; 8 = get/set sound_flag
        dd      sysfn_shutdown_param    ; 9 = shutdown with parameter
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
sysfn_num = ($ - sys_system_table)/4
endg

sys_system:
        dec     eax
        cmp     eax, sysfn_num
        jae     @f
        jmp     dword [sys_system_table + eax*4]
@@:
        ret

sysfn_shutdown:         ; 18.1 = BOOT
     mov  [0x2f0000+0x9030],byte 0
  for_shutdown_parameter:

     mov  eax,[TASK_COUNT]
     add  eax,2
     mov  [shutdown_processes],eax
     mov  [SYS_SHUTDOWN],al
     and  dword [esp+36], 0
     ret
  uglobal
   shutdown_processes: dd 0x0
  endg

sysfn_terminate:        ; 18.2 = TERMINATE
     cmp  ebx,2
     jb   noprocessterminate
     mov  edx,[TASK_COUNT]
     cmp  ebx,edx
     ja   noprocessterminate
     mov  eax,[TASK_COUNT]
     shl  ebx,5
     mov  edx,[ebx+CURRENT_TASK+TASKDATA.pid]
     add  ebx,CURRENT_TASK+TASKDATA.state
     cmp  byte [ebx], 9
     jz   noprocessterminate

     ;call MEM_Heap_Lock      ;guarantee that process isn't working with heap
     mov  [ebx],byte 3       ; clear possible i40's
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
    je     .stf
    sti
    call   change_task
    jmp    .table_status
.stf:
    call   set_application_table_status
    mov    eax,ebx
    call   pid_to_slot
    test   eax,eax
    jz     .not_found
    mov    ebx,eax
    cli
    call   sysfn_terminate
    mov    [application_table_status],0
    sti
    and    dword [esp+36],0
    ret
.not_found:
    mov    [application_table_status],0
    or     dword [esp+36],-1
    ret

sysfn_activate:         ; 18.3 = ACTIVATE WINDOW
     cmp  ebx,2
     jb   .nowindowactivate
     cmp  ebx,[TASK_COUNT]
     ja   .nowindowactivate

     mov   [window_minimize], 2   ; restore window if minimized

     movzx esi, word [WIN_STACK + ebx*2]
     cmp   esi, [TASK_COUNT]
     je    .nowindowactivate ; already active

     mov   edi, ebx
     shl   edi, 5
     add   edi, window_data
     movzx esi, word [WIN_STACK + ebx * 2]
     lea   esi, [WIN_POS + esi * 2]
     call  waredraw
.nowindowactivate:
     ret

sysfn_getidletime:              ; 18.4 = GET IDLETIME
     mov  eax,[idleusesec]
     mov  [esp+36], eax
     ret

sysfn_getcpuclock:              ; 18.5 = GET TSC/SEC
     mov  eax,[CPU_FREQ]
     mov  [esp+36], eax
     ret

;  SAVE ramdisk to /hd/1/menuet.img
;!!!!!!!!!!!!!!!!!!!!!!!!
   include 'blkdev/rdsave.inc'
;!!!!!!!!!!!!!!!!!!!!!!!!

sysfn_getactive:        ; 18.7 = get active window
     mov  eax, [TASK_COUNT]
   movzx  eax, word [WIN_POS + eax*2]
     mov  [esp+36],eax
     ret

sysfn_sound_flag:       ; 18.8 = get/set sound_flag
     cmp  ebx,1
     jne  nogetsoundflag
     movzx  eax,byte [sound_flag] ; get sound_flag
     mov  [esp+36],eax
     ret
 nogetsoundflag:
     cmp  ebx,2
     jnz  nosoundflag
     xor  byte [sound_flag], 1
 nosoundflag:
     ret

sysfn_shutdown_param:   ; 18.9 = system shutdown with param
     cmp  ebx,1
     jl   exit_for_anyone
     cmp  ebx,4
     jg   exit_for_anyone
     mov  [0x2f0000+0x9030],bl
     jmp  for_shutdown_parameter

sysfn_minimize:         ; 18.10 = minimize window
     mov   [window_minimize],1
 exit_for_anyone:
     ret

sysfn_getdiskinfo:      ; 18.11 = get disk info table
     cmp  ebx,1
     jnz  full_table
  small_table:
     call for_all_tables
     mov ecx,10
     cld
     rep movsb
     ret
   for_all_tables:
     mov edi,[TASK_BASE]
     mov edi,[edi+TASKDATA.mem_start]
     add edi,ecx
     mov esi,DRIVE_DATA
     ret
  full_table:
     cmp  ebx,2
     jnz  exit_for_anyone
     call for_all_tables
     mov ecx,16384
     cld
     rep movsd
     ret

sysfn_lastkey:          ; 18.12 = return 0 (backward compatibility)
        and     dword [esp+36], 0
        ret

sysfn_getversion:       ; 18.13 = get kernel ID and version
     mov edi,[TASK_BASE]
     mov edi,[edi+TASKDATA.mem_start]
     add edi,ebx
     mov esi,version_inf
     mov ecx,version_end-version_inf
     cld
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
     mov [esp+36],dword 0
     ret

sysfn_centermouse:      ; 18.15 = mouse centered
     call  mouse_centered
     mov [esp+36],dword 0
     ret

sysfn_mouse_acceleration: ; 18.19 = set/get mouse features
     cmp  ebx,0  ; get mouse speed factor
     jnz  .set_mouse_acceleration
     xor  eax,eax
     mov  ax,[mouse_speed_factor]
     mov  [esp+36],eax
     ret
 .set_mouse_acceleration:
     cmp  ebx,1  ; set mouse speed factor
     jnz  .get_mouse_delay
     mov  [mouse_speed_factor],cx
     ret
 .get_mouse_delay:
     cmp  ebx,2  ; get mouse delay
     jnz  .set_mouse_delay
     mov  eax,[mouse_delay]
     mov  [esp+36],eax
     ret
 .set_mouse_delay:
     cmp  ebx,3  ; set mouse delay
     jnz  .set_pointer_position
     mov  [mouse_delay],ecx
     ret
 .set_pointer_position:
     cmp  ebx,4  ; set mouse pointer position
     jnz  .end
     mov   [MOUSE_Y],cx    ;y
     ror   ecx,16
     mov   [MOUSE_X],cx    ;x
     rol   ecx,16
 .end:
     ret

sysfn_getfreemem:
     mov eax, [pg_data.pages_free]
     shl eax, 2
     mov [esp+36],eax
     ret

sysfn_getallmem:
     mov  eax,[MEM_AMOUNT]
     shr eax, 10
     mov  [esp+36],eax
     ret

uglobal
;// mike.dld, 2006-29-01 [
screen_workarea RECT
;// mike.dld, 2006-29-01 ]
window_minimize db 0
sound_flag      db 0
endg

iglobal
version_inf:
  db 0,6,5,0  ; version 0.6.5.0
  db UID_KOLIBRI
  db 'Kolibri',0
version_end:
endg

UID_NONE=0
UID_MENUETOS=1   ;official
UID_KOLIBRI=2    ;russian

sys_cachetodiskette:
;    pushad
;    cmp  eax,1
;    jne  no_write_all_of_ramdisk
;    call fdc_writeramdisk
;    popad
;    ret
;  no_write_all_of_ramdisk:
;    cmp eax,2
;    jne no_write_part_of_ramdisk
;    call fdc_commitflush
;    popad
;    ret
;  no_write_part_of_ramdisk:
;    cmp  eax,3
;    jne  no_set_fdc
;    call fdc_set
;    popad
;    ret
;  no_set_fdc:
;    cmp  eax,4
;    jne  no_get_fdc
;    popad
;    call fdc_get
;    mov    [esp+36],ecx
;    ret
;  no_get_fdc:
;    popad
;    ret
    cmp eax,1
    jne no_floppy_a_save
    mov   [flp_number],1
    jmp save_image_on_floppy
  no_floppy_a_save:
    cmp eax,2
    jne no_floppy_b_save
    mov   [flp_number],2
  save_image_on_floppy:
    call save_image
    mov  [esp+36],dword 0
    cmp  [FDC_Status],0
    je   yes_floppy_save
  no_floppy_b_save:
    mov [esp+36],dword 1
  yes_floppy_save:
    ret

uglobal
;  bgrchanged  dd  0x0
endg

sys_background:

    cmp   eax,1                            ; BACKGROUND SIZE
    jnz   nosb1
    cmp   ebx,0
    je    sbgrr
    cmp   ecx,0
    je    sbgrr
    mov   [display_data-8],ebx
    mov   [display_data-4],ecx
;    mov   [bgrchanged],1
  sbgrr:
    ret
  nosb1:

    cmp   eax,2                            ; SET PIXEL
    jnz   nosb2
    mov   edx,0x160000-16
    cmp   edx,ebx
    jbe   nosb2
    mov   edx,[ebx]
    and   edx,0xFF000000 ;255*256*256*256
    and   ecx,0x00FFFFFF ;255*256*256+255*256+255
    add   edx,ecx
    mov   [ebx+IMG_BACKGROUND],edx
;    mov   [bgrchanged],1
    ret
  nosb2:

    cmp   eax,3                            ; DRAW BACKGROUND
    jnz   nosb3
draw_background_temp:
;    cmp   [bgrchanged],1 ;0
;    je    nosb31
;draw_background_temp:
;    mov   [bgrchanged],1 ;0
    mov   [REDRAW_BACKGROUND],byte 1
    mov    [background_defined], 1
   nosb31:
    ret
  nosb3:

    cmp   eax,4                            ; TILED / STRETCHED
    jnz   nosb4
    cmp   ebx,[display_data-12]
    je    nosb41
    mov   [display_data-12],ebx
;    mov   [bgrchanged],1
   nosb41:
    ret
  nosb4:

    cmp   eax,5                            ; BLOCK MOVE TO BGR
    jnz   nosb5
  ; bughere
    mov   edi, [TASK_BASE]
    add   ebx, [edi+TASKDATA.mem_start]
 ;   mov   esi, ebx
 ;   mov   edi, ecx
    mov   eax, ebx
    mov   ebx, ecx
    add   ecx, edx
    cmp   ecx, 0x160000-16
    ja    .fin
 ;   add   edi, 0x300000
    add   ebx, IMG_BACKGROUND
    mov   ecx, edx
    cmp   ecx, 0x160000-16
    ja    .fin
;    mov   [bgrchanged],1
  ;  cld
  ;  rep   movsb
    call  memmove
  .fin:
    ret
  nosb5:

    ret


align 4

sys_getbackground:

    cmp   eax,1                                  ; SIZE
    jnz   nogb1
    mov   eax,[display_data-8]
    shl   eax,16
    mov   ax,[display_data-4]
    mov   [esp+36],eax
    ret
  nogb1:

    cmp   eax,2                                  ; PIXEL
    jnz   nogb2
    mov   edx,0x160000-16
    cmp   edx,ebx
    jbe   nogb2
    mov   eax, [ebx+IMG_BACKGROUND]
    and   eax, 0xFFFFFF
    mov   [esp+36],eax
    ret
  nogb2:

    cmp   eax,4                                  ; TILED / STRETCHED
    jnz   nogb4
    mov   eax,[display_data-12]
  nogb4:
    mov   [esp+36],eax
    ret


align 4

sys_getkey:
    mov   [esp+36],dword 1
; test main buffer
    mov   ebx, [CURRENT_TASK]                          ; TOP OF WINDOW STACK
    movzx ecx,word [WIN_STACK + ebx * 2]
    mov   edx,[TASK_COUNT]
    cmp   ecx,edx
    jne   .finish
    cmp   [KEY_COUNT],byte 0
    je    .finish
    movzx eax,byte [KEY_BUFF]
    shl   eax,8
    push  eax
    dec   byte [KEY_COUNT]
    and   byte [KEY_COUNT],127
    movzx ecx,byte [KEY_COUNT]
    add   ecx,2
 ;   mov   esi,0xf402
 ;   mov   edi,0xf401
 ;   cld
 ;  rep   movsb
    mov   eax, KEY_BUFF+1
    mov   ebx, KEY_BUFF
    call  memmove
    pop   eax
.ret_eax:
    mov   [esp+36],eax
    ret
 .finish:
; test hotkeys buffer
        mov     ecx, hotkey_buffer
@@:
        cmp     [ecx], ebx
        jz      .found
        add     ecx, 8
        cmp     ecx, hotkey_buffer+120*8
        jb      @b
        ret
.found:
        mov     ax, [ecx+6]
        shl     eax, 16
        mov     ah, [ecx+4]
        mov     al, 2
        and     dword [ecx+4], 0
        and     dword [ecx], 0
        jmp     .ret_eax

align 4

sys_getbutton:

    mov   ebx, [CURRENT_TASK]                         ; TOP OF WINDOW STACK
    mov   [esp+36],dword 1
    movzx ecx, word [WIN_STACK + ebx * 2]
    mov   edx, [TASK_COUNT] ; less than 256 processes
    cmp   ecx,edx
    jne   .exit
    movzx eax,byte [BTN_COUNT]
    test  eax,eax
    jz    .exit
    mov   eax,[BTN_BUFF]
    shl   eax,8
    mov   [BTN_COUNT],byte 0
    mov   [esp+36],eax
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

    mov  edi,[TASK_BASE]   ; eax = return area
    add  eax,[edi + TASKDATA.mem_start]

    cmp  ebx,-1         ; who am I ?
    jne  no_who_am_i
    mov  ebx,[CURRENT_TASK]
  no_who_am_i:

    push eax            ; return area
    push ebx            ; process number

    push ebx
    push ebx
    push eax

    ; return memory usage

    xor  edx,edx
    mov  eax,0x20
    mul  ebx
    add  eax,CURRENT_TASK+TASKDATA.cpu_usage
    mov  ebx,eax
    pop  eax
    mov  ecx,[ebx]
    mov  [eax],ecx
    pop  ebx
    mov  cx, [WIN_STACK + ebx * 2]
    mov  [eax+4],cx
    mov  cx, [WIN_POS + ebx * 2]
    mov  [eax+6],cx
    push eax
    mov  eax,ebx
    shl  eax,8
    add  eax,SLOT_BASE+APPDATA.app_name
    pop  ebx
    add  ebx,10
    mov  ecx,11
    call memmove

    ; memory usage

    xor    eax,eax
    mov    edx,0x100000*16
    pop    ecx                                   ; get gdt of tss
    cmp    ecx,1
    je     os_mem
    shl    ecx,8
    mov    edx,[SLOT_BASE+ecx+APPDATA.mem_size] ;0x8c
    mov    eax,std_application_base_address
    ; eax run base -> edx used memory
  os_mem:
    dec    edx
    mov    [ebx+12],eax
    mov    [ebx+16],edx

    ; PID (+30)

    mov    eax,[esp]
    shl    eax,5
    add    eax,CURRENT_TASK+TASKDATA.pid
    mov    eax,[eax]
    mov    [ebx+20],eax

    ; window position and size

    mov    esi,[esp]
    shl    esi,5
    add    esi,window_data + WDATA.box
    mov    edi,[esp+4]
    add    edi,34
    mov    ecx,4
    cld
    rep    movsd

    ; Process state (+50)

    mov    eax,[esp]
    shl    eax,5
    add    eax,CURRENT_TASK+TASKDATA.state
    mov    eax,[eax]
    mov    [ebx+40],ax

    ; Window client area box

    mov    esi,[esp]
    shl    esi,8
    add    esi,SLOT_BASE+APPDATA.wnd_clientbox
    lea    edi,[ebx+44]
    mov    ecx,4
    rep    movsd

    ; Window state

    mov    esi,[esp]
    shl    esi,5
    add    esi,window_data + WDATA.box
    mov    al,[esi+window_data+WDATA.fl_wstate]
    mov    [edi],al

    pop    ebx
    pop    eax

    ; return number of processes

    mov    eax,[TASK_COUNT]
    mov    [esp+36],eax
    ret




align 4
sys_clock:
        cli
  ; Mikhail Lisovin  xx Jan 2005
  @@:   mov   al, 10
        out   0x70, al
        in    al, 0x71
        test  al, al
        jns   @f
        mov   esi, 1
        call  delay_ms
        jmp   @b
  @@:
  ; end Lisovin's fix

        xor   al,al           ; seconds
        out   0x70,al
        in    al,0x71
        movzx ecx,al
        mov   al,02           ; minutes
        shl   ecx,16
        out   0x70,al
        in    al,0x71
        movzx edx,al
        mov   al,04           ; hours
        shl   edx,8
        out   0x70,al
        in    al,0x71
        add   ecx,edx
        movzx edx,al
        add   ecx,edx
        sti
        mov   [esp+36],ecx
        ret


align 4

sys_date:

        cli

  @@:   mov   al, 10
        out   0x70, al
        in    al, 0x71
        test  al, al
        jns   @f
        mov   esi, 1
        call  delay_ms
        jmp   @b
  @@:

        mov     ch,0
        mov     al,7            ; date
        out     0x70,al
        in      al,0x71
        mov     cl,al
        mov     al,8            ; month
        shl     ecx,16
        out     0x70,al
        in      al,0x71
        mov     ch,al
        mov     al,9            ; year
        out     0x70,al
        in      al,0x71
        mov     cl,al
        sti
        mov     [esp+36],ecx
        ret


; redraw status

sys_redrawstat:

    cmp  eax,1
    jne  no_widgets_away

    ; buttons away

    mov   ecx,[CURRENT_TASK]

  sys_newba2:

    mov   edi,[BTN_ADDR]
    cmp   [edi],dword 0  ; empty button list ?
    je    end_of_buttons_away

    movzx ebx,word [edi]
    inc   ebx

    mov   eax,edi

  sys_newba:

    dec   ebx
    jz    end_of_buttons_away

    add   eax,0x10
    cmp   cx,[eax]
    jnz   sys_newba

    push  eax ebx ecx
    mov   ecx,ebx
    inc   ecx
    shl   ecx,4
    mov   ebx,eax
    add   eax,0x10
    call  memmove
    dec   dword [edi]
    pop   ecx ebx eax

    jmp   sys_newba2

  end_of_buttons_away:

    ret

  no_widgets_away:

    cmp   eax,2
    jnz   srl1

    mov   edx,[TASK_BASE]      ; return whole screen draw area for this app
    add   edx,draw_data-CURRENT_TASK
    mov   [edx+RECT.left], 0
    mov   [edx+RECT.top], 0
    mov   eax,[ScreenWidth]
    mov   [edx+RECT.right],eax
    mov   eax,[ScreenHeight]
    mov   [edx+RECT.bottom],eax

    mov   edi,[TASK_BASE]
    or    [edi-twdw+WDATA.fl_wdrawn], 1   ; no new position & buttons from app

    call  sys_window_mouse

    ret

  srl1:

    ret


sys_drawwindow:

    mov   edi,ecx
    shr   edi,16+8
    and   edi,15

    cmp   edi,0   ; type I    - original style
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

    cmp   edi,1   ; type II   - only reserve area, no draw
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

    cmp   edi,2   ; type III  - new style
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

    cmp   edi,3   ; type IV - skinned window
    jne   nosyswIV

    ; parameter for drawwindow_IV
    push  0
    mov   edi, [TASK_COUNT]
    movzx edi, word [WIN_POS + edi*2]
    cmp   edi, [CURRENT_TASK]
    jne   @f
    inc   dword [esp]
 @@:

    inc   [mouse_pause]
    call  [disable_mouse]
    call  sys_set_window
    call  [disable_mouse]
    call  drawwindow_IV
    ;dec   [mouse_pause]
    ;call   [draw_pointer]
    ;ret
    jmp   draw_window_caption.2
  nosyswIV:

    ret


draw_window_caption:
        inc     [mouse_pause]
        call    [disable_mouse]

        xor     eax,eax
        mov     edx,[TASK_COUNT]
        movzx   edx,word[WIN_POS+edx*2]
        cmp     edx,[CURRENT_TASK]
        jne     @f
        inc     eax
    @@: mov     edx,[CURRENT_TASK]
        shl     edx,5
        add     edx,window_data
        movzx   ebx,[edx+WDATA.fl_wstyle]
        and     bl,0x0F
        cmp     bl,3
        jne     .not_style_3

        push    edx
        call    drawwindow_IV_caption
        add     esp,4
        jmp     .2

  .not_style_3:
        cmp     bl,2
        jne     .not_style_2

        call    drawwindow_III_caption
        jmp     .2

  .not_style_2:
        cmp     bl,0
        jne     .2

        call    drawwindow_I_caption

;--------------------------------------------------------------
  .2:   ;jmp     @f
        mov     edi,[CURRENT_TASK]
        shl     edi,5
        test    [edi+window_data+WDATA.fl_wstyle],WSTYLE_HASCAPTION
        jz      @f
        mov     ecx,[edi*8+SLOT_BASE+APPDATA.wnd_caption]
        or      ecx,ecx
        jz      @f
        add     ecx,[edi+twdw+TASKDATA.mem_start]

        movzx   eax,[edi+window_data+WDATA.fl_wstyle]
        and     al,0x0F
        cmp     al,3
        jne     .not_skinned

        mov     ebp,[edi+window_data+WDATA.box.left-2]
        mov     bp,word[edi+window_data+WDATA.box.top]
        movzx   eax,word[edi+window_data+WDATA.box.width]
        sub     ax,[_skinmargins.left]
        sub     ax,[_skinmargins.right]
        cwde
        cdq
        mov     ebx,6
        idiv    ebx
        or      eax,eax
        js      @f
        mov     edx,eax
        mov     eax,dword[_skinmargins.left-2]
        mov     ax,word[_skinh]
        sub     ax,[_skinmargins.bottom]
        sub     ax,[_skinmargins.top]
        sar     ax,1
        adc     ax,0
        add     ax,[_skinmargins.top]
        add     ax,-3
        add     eax,ebp
        jmp     .dodraw

  .not_skinned:
        cmp     al,1
        je      @f

        mov     ebp,[edi+window_data+WDATA.box.left-2]
        mov     bp,word[edi+window_data+WDATA.box.top]
        movzx   eax,word[edi+window_data+WDATA.box.width]
        sub     eax,16
        cwde
        cdq
        mov     ebx,6
        idiv    ebx
        or      eax,eax
        js      @f
        mov     edx,eax
        mov     eax,0x00080007
        add     eax,ebp
.dodraw:
        mov     ebx,[common_colours+16];0x00FFFFFF
        or      ebx, 0x80000000
        xor     edi,edi
        call    dtext

    @@:
;--------------------------------------------------------------
        dec     [mouse_pause]
        call    [draw_pointer]
        ret

iglobal
align 4
window_topleft dd \
  1, 21,\
  0,  0,\
  5, 20,\
  5,  ?
endg

set_window_clientbox:
        push    eax ecx edi

        mov     eax,[_skinh]
        mov     [window_topleft+4*7],eax

        mov     ecx,edi
        sub     edi,window_data
        shl     edi,3
        test    [ecx+WDATA.fl_wstyle],WSTYLE_CLIENTRELATIVE
        jz      @f

        movzx   eax,[ecx+WDATA.fl_wstyle]
        and     eax,0x0F
        mov     eax,[eax*8+window_topleft+0]
        mov     [edi+SLOT_BASE+APPDATA.wnd_clientbox.left],eax
        shl     eax,1
        neg     eax
        add     eax,[ecx+WDATA.box.width]
        mov     [edi+SLOT_BASE+APPDATA.wnd_clientbox.width],eax

        movzx   eax,[ecx+WDATA.fl_wstyle]
        and     eax,0x0F
        push    [eax*8+window_topleft+0]
        mov     eax,[eax*8+window_topleft+4]
        mov     [edi+SLOT_BASE+APPDATA.wnd_clientbox.top],eax
        neg     eax
        sub     eax,[esp]
        add     eax,[ecx+WDATA.box.height]
        mov     [edi+SLOT_BASE+APPDATA.wnd_clientbox.height],eax
        add     esp,4

        pop     edi ecx eax
        ret
    @@:
        xor     eax,eax
        mov     [edi+SLOT_BASE+APPDATA.wnd_clientbox.left],eax
        mov     [edi+SLOT_BASE+APPDATA.wnd_clientbox.top],eax
        mov     eax,[ecx+WDATA.box.width]
        mov     [edi+SLOT_BASE+APPDATA.wnd_clientbox.width],eax
        mov     eax,[ecx+WDATA.box.height]
        mov     [edi+SLOT_BASE+APPDATA.wnd_clientbox.height],eax

        pop     edi ecx eax
        ret

sys_set_window:

    mov   edi,[CURRENT_TASK]
    shl   edi,5
    add   edi,window_data

    ; colors
    mov   [edi+WDATA.cl_workarea],ecx
    mov   [edi+WDATA.cl_titlebar],edx
    mov   [edi+WDATA.cl_frames],esi

    ; check flag (?)
    test  [edi+WDATA.fl_wdrawn],1
    jnz   newd

    push  eax
    mov   eax,[timer_ticks] ;[0xfdf0]
    add   eax,100
    mov   [new_window_starting],eax
    pop   eax

    mov   word[edi+WDATA.box.width],ax
    mov   word[edi+WDATA.box.height],bx
    sar   eax,16
    sar   ebx,16
    mov   word[edi+WDATA.box.left],ax
    mov   word[edi+WDATA.box.top],bx

    call  check_window_position

    call  set_window_clientbox

    push  ecx esi edi               ; save for window fullscreen/resize
    ;mov   esi,edi

        mov     cl,[edi+WDATA.fl_wstyle]

    sub   edi,window_data
    shl   edi,3
    add   edi,SLOT_BASE

        and     cl,0x0F
        mov     [edi+APPDATA.wnd_caption],0
        cmp     cl,3
        jne     @f
        mov     [edi+APPDATA.wnd_caption],esi
    @@: mov     esi,[esp+0]

    add   edi, APPDATA.saved_box
        movsd
        movsd
        movsd
        movsd
    pop   edi esi ecx

    push  eax ebx ecx edx
;;;    mov   eax, 1
;;;    call  delay_hs
    mov   eax, [edi+WDATA.box.left]
    mov   ebx, [edi+WDATA.box.top]
    mov   ecx, [edi+WDATA.box.width]
    mov   edx, [edi+WDATA.box.height]
    add   ecx, eax
    add   edx, ebx
    call  calculatescreen
    pop   edx ecx ebx eax

    mov   [KEY_COUNT],byte 0           ; empty keyboard buffer
    mov   [BTN_COUNT],byte 0           ; empty button buffer

  newd:
    mov   [edi+WDATA.fl_redraw],byte 0   ; no redraw
    mov   edx,edi

    ret

syscall_windowsettings:

  .set_window_caption:
        dec     eax     ; subfunction #1 - set window caption
        jnz     .get_window_caption

        ; NOTE: only window owner thread can set its caption,
        ;       so there's no parameter for PID/TID

        mov     edi,[CURRENT_TASK]
        shl     edi,5

        ; have to check if caption is within application memory limit
        ; check is trivial, and if application resizes its memory,
        ;   caption still can become over bounds
; diamond, 31.10.2006: check removed because with new memory manager
; there can be valid data after APPDATA.mem_size bound
;        mov     ecx,[edi*8+SLOT_BASE+APPDATA.mem_size]
;        add     ecx,255 ; max caption length
;        cmp     ebx,ecx
;        ja      .exit_fail

        mov     [edi*8+SLOT_BASE+APPDATA.wnd_caption],ebx
        or      [edi+window_data+WDATA.fl_wstyle],WSTYLE_HASCAPTION

        call    draw_window_caption

        xor     eax,eax ; eax = 0 (success)
        ret

  .get_window_caption:
        dec     eax     ; subfunction #2 - get window caption
        jnz     .exit_fail

        ; not implemented yet

  .exit_fail:
        xor     eax,eax
        inc     eax     ; eax = 1 (fail)
        ret


sys_window_move:

        mov     edi,[CURRENT_TASK]
        shl     edi,5
        add     edi,window_data

        test    [edi+WDATA.fl_wstate],WSTATE_MAXIMIZED
        jnz     .window_move_return

        push    dword [edi + WDATA.box.left]  ; save old coordinates
        push    dword [edi + WDATA.box.top]
        push    dword [edi + WDATA.box.width]
        push    dword [edi + WDATA.box.height]

        cmp   eax,-1                  ; set new position and size
        je    .no_x_reposition
        mov     [edi + WDATA.box.left], eax
      .no_x_reposition:
        cmp   ebx,-1
        je    .no_y_reposition
        mov     [edi + WDATA.box.top], ebx
      .no_y_reposition:

        test    [edi+WDATA.fl_wstate],WSTATE_ROLLEDUP
        jnz     .no_y_resizing

        cmp   ecx,-1
        je    .no_x_resizing
        mov     [edi + WDATA.box.width], ecx
      .no_x_resizing:
        cmp   edx,-1
        je    .no_y_resizing
        mov     [edi + WDATA.box.height], edx
      .no_y_resizing:

        call  check_window_position
        call  set_window_clientbox

        pushad                       ; save for window fullscreen/resize
        mov   esi,edi
        sub   edi,window_data
        shr   edi,5
        shl   edi,8
        add   edi, SLOT_BASE + APPDATA.saved_box
        mov   ecx,4
        cld
        rep   movsd
        popad

        pushad                       ; calculcate screen at new position
        mov   eax, [edi + WDATA.box.left]
        mov   ebx, [edi + WDATA.box.top]
        mov   ecx, [edi + WDATA.box.width]
        mov   edx, [edi + WDATA.box.height]
        add   ecx,eax
        add   edx,ebx

        call  calculatescreen
        popad

        pop   edx                   ; calculcate screen at old position
        pop   ecx
        pop   ebx
        pop   eax
        add   ecx,eax
        add   edx,ebx
        mov   [dlx],eax             ; save for drawlimits
        mov   [dly],ebx
        mov   [dlxe],ecx
        mov   [dlye],edx
        call  calculatescreen

        mov   [edi + WDATA.fl_redraw], 1 ; flag the process as redraw

        mov   eax,edi               ; redraw screen at old position
        xor   esi,esi
        call  redrawscreen

        mov   [DONT_DRAW_MOUSE],byte 0 ; mouse pointer
        mov   [MOUSE_BACKGROUND],byte 0 ; no mouse under
        mov   [MOUSE_DOWN],byte 0 ; react to mouse up/down

        mov   ecx,10          ; wait 1/10 second
      .wmrl3:
        call  [draw_pointer]
        mov   eax,1
        call  delay_hs
        loop  .wmrl3

        mov   [window_move_pr],0

      .window_move_return:

        ret

;type_background_1:
;    cmp   [0xfff0],byte 0               ; background update ?
;    jz    temp_nobackgr
;    mov   [0xfff0],byte 2
;    call  change_task
;    mov   [draw_data+32+0],dword 0
;    mov   [draw_data+32+4],dword 0
;    mov   eax,[ScreenWidth
;    mov   ebx,[0xfe04]
;    mov   [draw_data+32+8],eax
;    mov   [draw_data+32+12],ebx
;    call  drawbackground
;    mov   [0xfff0],byte 0
;    mov   [MOUSE_BACKGROUND],byte 0
;temp_nobackgr:
;    ret

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
mov eax,esi
wrmsr
mov [esp+36],eax
mov [esp+24],edx ;ret in ebx?
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

        mov  edx,[ScreenWidth]     ; screen x size
        inc  edx
        imul edx, ebx
        mov  dl, [eax+edx+display_data] ; lea eax, [...]

        xor  ecx, ecx
        mov  eax, [CURRENT_TASK]
        cmp  al, dl
        setne cl

        pop  edx eax
        ret

uglobal
  mouse_active  db  0
endg
iglobal
  cpustring db '/RD/1/CPU',0
endg

uglobal
background_defined    db    0    ; diamond, 11.04.2006
endg

align 4
; check misc

checkmisc:

    cmp   [ctrl_alt_del], 1
    jne   nocpustart
    mov   ebp, cpustring
    call   fs_execute               ; SPraid 8.03.2007
    ;lea   esi,[ebp+6]
    ;xor   ebx,ebx               ; no parameters
    ;xor   edx,edx               ; no flags
    ;call  fs_RamdiskExecute.flags
    mov   [ctrl_alt_del], 0
  nocpustart:
    cmp   [mouse_active], 1
    jne   mouse_not_active
    mov   [mouse_active], 0
    xor   edi, edi
    mov   ecx, [TASK_COUNT]
   set_mouse_event:
    add   edi, 256
    or    [edi+SLOT_BASE+APPDATA.event_mask], dword 00100000b
    loop  set_mouse_event
  mouse_not_active:


    cmp   [REDRAW_BACKGROUND],byte 0               ; background update ?
    jz    nobackgr
    cmp    [background_defined], 0
    jz    nobackgr
    mov   [REDRAW_BACKGROUND],byte 2
    call  change_task
	mov   [draw_data+32 + RECT.left],dword 0
	mov   [draw_data+32 + RECT.top],dword 0
    mov   eax,[ScreenWidth]
    mov   ebx,[ScreenHeight]
	mov   [draw_data+32 + RECT.right],eax
	mov   [draw_data+32 + RECT.bottom],ebx
    call  drawbackground
    mov   [REDRAW_BACKGROUND],byte 0
    mov   [MOUSE_BACKGROUND],byte 0

  nobackgr:


    ; system shutdown request

    cmp  [SYS_SHUTDOWN],byte 0
    je   noshutdown

    mov  edx,[shutdown_processes]
    sub  dl,2

    cmp  [SYS_SHUTDOWN],dl
    jne  no_mark_system_shutdown

    mov   edx,0x3040
    movzx ecx,byte [SYS_SHUTDOWN]
    add   ecx,5
  markz:
    mov   [edx+TASKDATA.state],byte 3
    add   edx,0x20
    loop  markz

  no_mark_system_shutdown:

    call [disable_mouse]

    dec  byte [SYS_SHUTDOWN]

    cmp  [SYS_SHUTDOWN],byte 0
    je   system_shutdown

  noshutdown:


    mov   eax,[TASK_COUNT]                  ; termination
    mov   ebx,TASK_DATA+TASKDATA.state
    mov   esi,1

  newct:
    mov   cl,[ebx]
    cmp   cl,byte 3
    jz    terminate
    cmp   cl,byte 4
    jz    terminate

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

;;;         mov   eax,2
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

         cmp   ecx,1               ; limit for background
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

         cmp   edi,esi
         jz    ricino

         mov   eax,edi
         add   eax,draw_data-window_data

         mov   ebx,[dlx]          ; set limits
         mov   [eax + RECT.left], ebx
         mov   ebx,[dly]
         mov   [eax + RECT.top], ebx
         mov   ebx,[dlxe]
         mov   [eax + RECT.right], ebx
         mov   ebx,[dlye]
         mov   [eax + RECT.bottom], ebx

         sub   eax,draw_data-window_data

         cmp   ecx,1
         jne   nobgrd
         cmp   esi,1
         je    newdw8
         call  drawbackground

       newdw8:
       nobgrd:

         mov   [eax + WDATA.fl_redraw],byte 1    ; mark as redraw

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

        mov   [display_data-8],dword 4      ; size x
        mov   [display_data-4],dword 2      ; size y

        mov   edi, IMG_BACKGROUND                 ; set background to black
        xor   eax, eax
        mov   ecx, 0x0fff00 / 4
        cld
        rep   stosd

        mov   edi,display_data              ; set os to use all pixels
        mov   eax,0x01010101
        mov   ecx,0x15ff00 / 4
        rep   stosd

        mov   byte [REDRAW_BACKGROUND], 0              ; do not draw background!

        ret

uglobal
  imax    dd 0x0
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

 cnt1:  in    al,0x61
        and   al,0x10
        cmp   al,ah
        jz    cnt1

        mov   ah,al
        loop  cnt1

        pop   ecx
        pop   eax

        ret


set_app_param:
        push edi

        mov  edi,[TASK_BASE]
        mov  [edi+TASKDATA.event_mask],eax

        pop  edi
        ret



delay_hs:     ; delay in 1/100 secs
        push  eax
        push  ecx
        push  edx

        mov   edx,[timer_ticks]
        add   edx,eax

      newtic:
        mov   ecx,[timer_ticks]
        cmp   edx,ecx
        jbe   zerodelay

        call  change_task

        jmp   newtic

      zerodelay:
        pop   edx
        pop   ecx
        pop   eax

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
    jz   @f

    push ecx
    shr  ecx, 2
    rep  movsd
    pop  ecx
    and  ecx, 11b
    jz   .finish
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

    mov   edi,[TASK_BASE]
    add   eax,[edi+TASKDATA.mem_start]

    cmp   ebx,16
    jae   .not_owner
    mov   edi,[TASK_BASE]
    mov   edi,[edi+TASKDATA.pid]
    cmp   edi,[irq_owner+ebx*4]
    je    spril1
.not_owner:
    mov   [esp+36],dword 1
    ret
  spril1:

    mov   esi,eax
    shl   ebx,6
    add   ebx,irq00read
    mov   edi,ebx
    mov   ecx,16
    cld
    rep   movsd
    mov   [esp+36],dword 0
    ret


align 4

get_irq_data:
     cmp   eax,16
     jae   .not_owner
     mov   edx,eax           ; check for correct owner
     shl   edx,2
     add   edx,irq_owner
     mov   edx,[edx]
     mov   edi,[TASK_BASE]
     mov   edi,[edi+TASKDATA.pid]
     cmp   edx,edi
     je    gidril1
.not_owner:
     mov   [esp+32],dword 2     ; ecx=2
     ret

  gidril1:

     mov   ebx,eax
     shl   ebx,12
     add   ebx,IRQ_SAVE
     mov   eax,[ebx]
     mov   ecx,1
     test  eax,eax
     jz    gid1

     dec   eax
     mov   esi,ebx
     mov   [ebx],eax
     movzx ebx,byte [ebx+0x10]
     add   esi,0x10
     mov   edi,esi
     inc   esi
     mov   ecx,4000 / 4
     cld
     rep   movsd
;     xor   ecx,ecx     ; as result of 'rep' ecx=0
   gid1:
     mov   [esp+36],eax
     mov   [esp+32],ecx
     mov   [esp+24],ebx
     ret


set_io_access_rights:

     pushad

     mov   edi,[CURRENT_TASK]
     imul  edi,tss_step
     add   edi,tss_data+128
;     add   edi,128

     mov   ecx,eax
     and   ecx,7    ; offset in byte

     shr   eax,3    ; number of byte
     add   edi,eax

     mov   ebx,1
     shl   ebx,cl

     cmp   ebp,0                ; enable access - ebp = 0
     jne   siar1

     not   ebx
     and   [edi],byte bl

     popad

     ret

   siar1:

     or    [edi],byte bl        ; disable access - ebp = 1

     popad

     ret

r_f_port_area:

     test  eax, eax
     jnz   free_port_area
;     je    r_port_area
;     jmp   free_port_area

;   r_port_area:

     pushad

     cmp   ebx,ecx            ; beginning > end ?
     ja    rpal1
     cmp   ecx,65536
     jae   rpal1
     mov   esi,[RESERVED_PORTS]
     test  esi,esi            ; no reserved areas ?
     je    rpal2
     cmp   esi,255            ; max reserved
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
     pushad                        ; start enable io map

     cmp   ecx,65536 ;16384
     jae   no_unmask_io ; jge

     mov   eax,ebx

   new_port_access:

     pushad

     xor   ebp,ebp                ; enable - eax = port
     call  set_io_access_rights

     popad

     inc   eax
     cmp   eax,ecx
     jbe   new_port_access

   no_unmask_io:

     popad                         ; end enable io map
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

     pushad                        ; start disable io map

     cmp   ecx,65536 ;16384
     jge   no_mask_io

     mov   eax,ebx

   new_port_access_disable:

     pushad

     mov   ebp,1                  ; disable - eax = port
     call  set_io_access_rights

     popad

     inc   eax
     cmp   eax,ecx
     jbe   new_port_access_disable

   no_mask_io:

     popad                         ; end disable io map

     xor   eax, eax
     ret


reserve_free_irq:

     mov   ecx, 1
     cmp   ebx, 16
     jae   fril1
     test  eax,eax
     jz    reserve_irq

     lea   edi,[irq_owner+ebx*4]
     mov   edx,[edi]
     mov   eax,[TASK_BASE]
     cmp   edx,[eax+TASKDATA.pid]
     jne   fril1
     dec   ecx
     mov   [edi],ecx
   fril1:
     mov   [esp+36],ecx ; return in eax
     ret

  reserve_irq:

     lea   edi,[irq_owner+ebx*4]
     cmp   dword [edi], 0
     jnz   ril1

     mov   edx,[TASK_BASE]
     mov   edx,[edx+TASKDATA.pid]
     mov   [edi],edx
     dec   ecx
   ril1:
     mov   [esp+36],ecx ; return in eax
     ret

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
       cmp   [display_data-12],dword 1
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

syscall_putimage:                       ; PutImage

     mov   edx,ecx
     mov   ecx,ebx
        lea     ebx, [eax+std_application_base_address]

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
        mov     edi,[CURRENT_TASK]
        shl     edi,8
        add     dx,word[edi+SLOT_BASE+APPDATA.wnd_clientbox.top]
        rol     edx,16
        add     dx,word[edi+SLOT_BASE+APPDATA.wnd_clientbox.left]
        rol     edx,16
  .forced:
        push    ebp esi 0
        mov     ebp, putimage_get24bpp
        mov     esi, putimage_init24bpp
sys_putimage_bpp:
;        call    [disable_mouse] ; this will be done in xxx_putimage
;        mov     eax, vga_putimage
        cmp     [SCR_MODE], word 0x12
        jz      @f   ;.doit
        mov     eax, vesa12_putimage
        cmp     [SCR_MODE], word 0100000000000000b
        jae     @f
        cmp     [SCR_MODE], word 0x13
        jnz     .doit
@@:
        mov     eax, vesa20_putimage
.doit:
        inc     [mouse_pause]
        call    eax
        dec     [mouse_pause]
        pop     ebp esi ebp
        jmp     [draw_pointer]

syscall_putimage_palette:
        lea     edi, [esi+std_application_base_address]
        mov     esi, edx
        mov     edx, ecx
        mov     ecx, ebx
        lea     ebx, [eax+std_application_base_address]
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
.forced:
        push    ebp esi ebp
        cmp     esi, 8
        jnz     @f
        mov     ebp, putimage_get8bpp
        mov     esi, putimage_init8bpp
        jmp     sys_putimage_bpp
@@:
        cmp     esi, 24
        jnz     @f
        mov     ebp, putimage_get24bpp
        mov     esi, putimage_init24bpp
        jmp     sys_putimage_bpp
@@:
        cmp     esi, 32
        jnz     @f
        mov     ebp, putimage_get32bpp
        mov     esi, putimage_init32bpp
        jmp     sys_putimage_bpp
@@:
        pop     ebp esi
        ret

putimage_init24bpp:
        lea     eax, [eax*3]
putimage_init8bpp:
        ret

putimage_get24bpp:
        mov     eax, [esi]
        add     esi, 3
        ret     4
putimage_get8bpp:
        movzx   eax, byte [esi]
        push    edx
        mov     edx, [esp+8]
        mov     eax, [edx+eax*4]
        pop     edx
        inc     esi
        ret     4

putimage_init32bpp:
        shl     eax, 2
        ret
putimage_get32bpp:
        lodsd
        ret     4

; eax x beginning
; ebx y beginning
; ecx x end
	; edx y end
; edi color

__sys_drawbar:
        mov     esi,[CURRENT_TASK]
        shl     esi,8
        add     eax,[esi+SLOT_BASE+APPDATA.wnd_clientbox.left]
        add     ecx,[esi+SLOT_BASE+APPDATA.wnd_clientbox.left]
        add     ebx,[esi+SLOT_BASE+APPDATA.wnd_clientbox.top]
        add     edx,[esi+SLOT_BASE+APPDATA.wnd_clientbox.top]
  .forced:
    inc   [mouse_pause]
;        call    [disable_mouse]
    cmp   [SCR_MODE],word 0x12
    je   dbv20
   sdbv20:
    cmp  [SCR_MODE],word 0100000000000000b
    jge  dbv20
    cmp  [SCR_MODE],word 0x13
    je   dbv20
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

        push    ecx edx

        mov     ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
      kr_loop:
        in      al,0x64
        test    al,1
        jnz     kr_ready
        loop    kr_loop
        mov     ah,1
        jmp     kr_exit
      kr_ready:
        push    ecx
        mov     ecx,32
      kr_delay:
        loop    kr_delay
        pop     ecx
        in      al,0x60
        xor     ah,ah
      kr_exit:

        pop     edx ecx

        ret


kb_write:

        push    ecx edx

        mov     dl,al
;        mov     ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
;      kw_loop1:
;        in      al,0x64
;        test    al,0x20
;        jz      kw_ok1
;        loop    kw_loop1
;        mov     ah,1
;        jmp     kw_exit
;      kw_ok1:
        in      al,0x60
        mov     ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
      kw_loop:
        in      al,0x64
        test    al,2
        jz      kw_ok
        loop    kw_loop
        mov     ah,1
        jmp     kw_exit
      kw_ok:
        mov     al,dl
        out     0x60,al
        mov     ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
      kw_loop3:
        in      al,0x64
        test    al,2
        jz      kw_ok3
        loop    kw_loop3
        mov     ah,1
        jmp     kw_exit
      kw_ok3:
        mov     ah,8
      kw_loop4:
        mov     ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
      kw_loop5:
        in      al,0x64
        test    al,1
        jnz     kw_ok4
        loop    kw_loop5
        dec     ah
        jnz     kw_loop4
      kw_ok4:
        xor     ah,ah
      kw_exit:

        pop     edx ecx

        ret


kb_cmd:

        mov     ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
      c_wait:
        in      al,0x64
        test    al,2
        jz      c_send
        loop    c_wait
        jmp     c_error
      c_send:
        mov     al,bl
        out     0x64,al
        mov     ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
      c_accept:
        in      al,0x64
        test    al,2
        jz      c_ok
        loop    c_accept
      c_error:
        mov     ah,1
        jmp     c_exit
      c_ok:
        xor     ah,ah
      c_exit:
        ret


setmouse:  ; set mousepicture -pointer
           ; ps2 mouse enable

     mov     [MOUSE_PICTURE],dword mousepointer

     cli
;     mov     bl,0xa8                 ; enable mouse cmd
;     call    kb_cmd
;     call    kb_read                 ; read status
;     mov     bl,0x20                 ; get command byte
;     call    kb_cmd
;     call    kb_read
;     or      al,3                    ; enable interrupt
;     mov     bl,0x60                 ; write command
;     push    eax
;     call    kb_cmd
;     pop     eax
;     call    kb_write
;     mov     bl,0xd4                 ; for mouse
;     call    kb_cmd
;     mov     al,0xf4                 ; enable mouse device
;     call    kb_write
;     call    kb_read           ; read status return

     ; com1 mouse enable

     mov   bx,0x3f8 ; combase

     mov   dx,bx
     add   dx,3
     mov   al,0x80
     out   dx,al

     mov   dx,bx
     add   dx,1
     mov   al,0
     out   dx,al

     mov   dx,bx
     add   dx,0
     mov   al,0x30*2    ; 0x30 / 4
     out   dx,al

     mov   dx,bx
     add   dx,3
     mov   al,2         ; 3
     out   dx,al

     mov   dx,bx
     add   dx,4
     mov   al,0xb
     out   dx,al

     mov   dx,bx
     add   dx,1
     mov   al,1
     out   dx,al


     ; com2 mouse enable

     mov   bx,0x2f8 ; combase

     mov   dx,bx
     add   dx,3
     mov   al,0x80
     out   dx,al

     mov   dx,bx
     add   dx,1
     mov   al,0
     out   dx,al

     mov   dx,bx
     add   dx,0
     mov   al,0x30*2
     out   dx,al

     mov   dx,bx
     add   dx,3
     mov   al,2
     out   dx,al

     mov   dx,bx
     add   dx,4
     mov   al,0xb
     out   dx,al

     mov   dx,bx
     add   dx,1
     mov   al,1
     out   dx,al

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

        mov     al,0x11         ;  icw4, edge triggered
        out     0x20,al
        call    pic_delay
        out     0xA0,al
        call    pic_delay

        mov     al,0x20         ;  generate 0x20 +
        out     0x21,al
        call    pic_delay
        mov     al,0x28         ;  generate 0x28 +
        out     0xA1,al
        call    pic_delay

        mov     al,0x04         ;  slave at irq2
        out     0x21,al
        call    pic_delay
        mov     al,0x02         ;  at irq9
        out     0xA1,al
        call    pic_delay

        mov     al,0x01         ;  8086 mode
        out     0x21,al
        call    pic_delay
        out     0xA1,al
        call    pic_delay

        mov     al,255          ; mask all irq's
        out     0xA1,al
        call    pic_delay
        out     0x21,al
        call    pic_delay

        mov     ecx,0x1000
        cld
picl1:  call    pic_delay
        loop    picl1

        mov     al,255          ; mask all irq's
        out     0xA1,al
        call    pic_delay
        out     0x21,al
        call    pic_delay

        cli

        ret


pic_delay:

        jmp     pdl1
pdl1:   ret


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

uglobal
  msg_board_data: times 4096 db 0
  msg_board_count dd 0x0
endg

sys_msg_board:

; eax=1 : write :  bl byte to write
; eax=2 :  read :  ebx=0 -> no data, ebx=1 -> data in al

     mov  ecx,[msg_board_count]
     cmp  eax, 1
     jne  smbl1


     mov  [msg_board_data+ecx],bl
     inc  ecx
     and  ecx, 4095
     mov  [msg_board_count], ecx
     mov  [check_idle_semaphore], 5
     ret
   smbl1:

     cmp   eax, 2
     jne   smbl2
     test  ecx, ecx
     jz    smbl21
;     mov   edi, msg_board_data
;     mov   esi, msg_board_data+1
;     movzx eax, byte [edi]
     mov   eax, msg_board_data+1
     mov   ebx, msg_board_data
     movzx edx, byte [ebx]
     call  memmove
;     push  ecx
;     shr   ecx, 2
;     cld
;     rep   movsd
;     pop   ecx
;     and   ecx, 3
;     rep   movsb
     dec   [msg_board_count]
     mov   [esp+36], edx ;eax
     mov   [esp+24], dword 1
     ret
   smbl21:
     mov   [esp+36], ecx
     mov   [esp+24], ecx

   smbl2:
     ret



sys_process_def:
        mov     edi, [CURRENT_TASK]

        dec     eax             ; 1 = set keyboard mode
     jne   no_set_keyboard_setup

     shl   edi,8
     mov   [edi+SLOT_BASE + APPDATA.keyboard_mode],bl

     ret

   no_set_keyboard_setup:

        dec     eax             ; 2 = get keyboard mode
     jne   no_get_keyboard_setup

     shl   edi,8
     movzx eax, byte [SLOT_BASE+edi + APPDATA.keyboard_mode]

     mov   [esp+36],eax

     ret

   no_get_keyboard_setup:

        dec     eax             ; 3 = get keyboard ctrl, alt, shift
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

        dec     eax
        jnz     no_add_keyboard_hotkey

        mov     eax, hotkey_list
@@:
        cmp     dword [eax+8], 0
        jz      .found_free
        add     eax, 16
        cmp     eax, hotkey_list+16*256
        jb      @b
        mov     dword [esp+36], 1
        ret
.found_free:
        mov     [eax+8], edi
        mov     [eax+4], ecx
        movzx   ebx, bl
        lea     ebx, [hotkey_scancodes+ebx*4]
        mov     ecx, [ebx]
        mov     [eax], ecx
        mov     [ebx], eax
        mov     [eax+12], ebx
        jecxz   @f
        mov     [ecx+12], eax
@@:
        and     dword [esp+36], 0
        ret

no_add_keyboard_hotkey:

        dec     eax
        jnz     no_del_keyboard_hotkey

        movzx   ebx, bl
        lea     ebx, [hotkey_scancodes+ebx*4]
        mov     eax, [ebx]
.scan:
        test    eax, eax
        jz      .notfound
        cmp     [eax+8], edi
        jnz     .next
        cmp     [eax+4], ecx
        jz      .found
.next:
        mov     eax, [eax]
        jmp     .scan
.notfound:
        mov     dword [esp+36], 1
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
        mov     [esp+36], edx
        ret

no_del_keyboard_hotkey:
     ret


align 4

sys_gs:                         ; direct screen access

     cmp  eax,1                 ; resolution
     jne  no_gs1
     mov  eax,[ScreenWidth]
     shl  eax,16
     mov  ax,[ScreenHeight]
     add  eax,0x00010001
     mov  [esp+36],eax
     ret
   no_gs1:

     cmp   eax,2                ; bits per pixel
     jne   no_gs2
     movzx eax,byte [ScreenBPP]
     mov   [esp+36],eax
     ret
   no_gs2:

     cmp   eax,3                ; bytes per scanline
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

syscall_setpixel:                       ; SetPixel


     mov   edx,[TASK_BASE]
     add   eax,[edx-twdw+WDATA.box.left]
     add   ebx,[edx-twdw+WDATA.box.top]
        mov     edi,[CURRENT_TASK]
        shl     edi,8
        add     eax,[edi+SLOT_BASE+APPDATA.wnd_clientbox.left]
        add     ebx,[edi+SLOT_BASE+APPDATA.wnd_clientbox.top]
     xor   edi,edi ; no force
;     mov   edi,1
     call  [disable_mouse]
     jmp   [putpixel]

align 4

syscall_writetext:                      ; WriteText

     mov   edi,[TASK_BASE]
     mov   ebp,[edi-twdw+WDATA.box.left]
        push    esi
        mov     esi,[CURRENT_TASK]
        shl     esi,8
        add     ebp,[esi+SLOT_BASE+APPDATA.wnd_clientbox.left]
     shl   ebp,16
     add   ebp,[edi-twdw+WDATA.box.top]
        add     bp,word[esi+SLOT_BASE+APPDATA.wnd_clientbox.top]
        pop     esi
     add   ecx,[edi+TASKDATA.mem_start]
     add   eax,ebp
     xor   edi,edi
     jmp   dtext

align 4

syscall_openramdiskfile:                ; OpenRamdiskFile


     mov   edi,[TASK_BASE]
     add   edi,TASKDATA.mem_start
     add   eax,[edi]
     add   edx,[edi]
     mov   esi,12
     call  fileread
     mov   [esp+36],ebx
     ret

align 4

syscall_drawrect:                       ; DrawRect

     mov   edi,ecx
     and   edi,0x80FFFFFF
     test  ax,ax
     je    drectr
     test  bx,bx
     je    drectr
     movzx ecx,ax
     shr   eax,16
     movzx edx,bx
     shr   ebx,16
        mov     esi,[CURRENT_TASK]
        shl     esi,8
        add     eax,[esi+SLOT_BASE+APPDATA.wnd_clientbox.left]
        add     ebx,[esi+SLOT_BASE+APPDATA.wnd_clientbox.top]
     add   ecx,eax
     add   edx,ebx
     jmp   [drawbar]
    drectr:
     ret

align 4

syscall_getscreensize:                  ; GetScreenSize

     movzx eax,word[ScreenWidth]
     shl   eax,16
     mov   ax,[ScreenHeight]
     mov   [esp+36],eax
     ret

align 4

syscall_cdaudio:                        ; CD

     call  sys_cd_audio
     mov   [esp+36],eax
     ret

align 4

syscall_delramdiskfile:                 ; DelRamdiskFile

     mov   edi,[TASK_BASE]
     add   edi,TASKDATA.mem_start
     add   eax,[edi]
     call  filedelete
     mov   [esp+36],eax
     ret

align 4

syscall_writeramdiskfile:               ; WriteRamdiskFile

     mov   edi,[TASK_BASE]
     add   edi,TASKDATA.mem_start
     add   eax,[edi]
     add   ebx,[edi]
     call  filesave
     mov   [esp+36],eax
     ret

align 4

syscall_getpixel:                       ; GetPixel
     mov   ecx,[ScreenWidth]
     inc   ecx
     xor   edx,edx
     div   ecx
     mov   ebx,edx
     xchg  eax,ebx
     call  dword [0xe024]
     mov   [esp+36],ecx
     ret

align 4

syscall_readstring:                     ; ReadString

     mov   edi,[TASK_BASE]
     add   edi,TASKDATA.mem_start
     add   eax,[edi]
     call  read_string
     mov   [esp+36],eax
     ret

align 4

syscall_drawline:                       ; DrawLine

     mov   edi,[TASK_BASE]
     movzx edx,word[edi-twdw+WDATA.box.left]
     mov   ebp,edx
        mov     esi,[CURRENT_TASK]
        shl     esi,8
        add     ebp,[esi+SLOT_BASE+APPDATA.wnd_clientbox.left]
        add     dx,word[esi+SLOT_BASE+APPDATA.wnd_clientbox.left]
     shl   edx,16
     add   ebp,edx
     movzx edx,word[edi-twdw+WDATA.box.top]
     add   eax,ebp
     mov   ebp,edx
        add     ebp,[esi+SLOT_BASE+APPDATA.wnd_clientbox.top]
        add     dx,word[esi+SLOT_BASE+APPDATA.wnd_clientbox.top]
     shl   edx,16
     xor   edi,edi
     add   edx,ebp
     add   ebx,edx
     jmp   [draw_line]

align 4

syscall_getirqowner:                    ; GetIrqOwner
     cmp   eax,16
     jae   .err
     shl   eax,2
     add   eax,irq_owner
     mov   eax,[eax]
     mov   [esp+36],eax
     ret
.err:
     or    dword [esp+36], -1
     ret

align 4

syscall_reserveportarea:                ; ReservePortArea and FreePortArea

     call  r_f_port_area
     mov   [esp+36],eax
     ret

align 4

syscall_threads:                        ; CreateThreads

     call  sys_threads
     mov   [esp+36],eax
     ret

align 4

stack_driver_stat:

     call  app_stack_handler            ; Stack status

;     mov   [check_idle_semaphore],5    ; enable these for zero delay
;     call  change_task                 ; between sent packet

     mov   [esp+36],eax
     ret

align 4

socket:                                 ; Socket interface
     call  app_socket_handler

;     mov   [check_idle_semaphore],5    ; enable these for zero delay
;     call  change_task                 ; between sent packet

     mov   [esp+36],eax
     mov   [esp+24],ebx
     ret

align 4

user_events:                            ; User event times

     mov   eax,0x12345678
     mov   [esp+36],eax

     ret

align 4

read_from_hd:                           ; Read from hd - fn not in use

     mov   edi,[TASK_BASE]
     add   edi,TASKDATA.mem_start
     add   eax,[edi]
     add   ecx,[edi]
     add   edx,[edi]
     call  file_read

     mov   [esp+36],eax
     mov   [esp+24],ebx

     ret

align 4
paleholder:
	ret

; --------------- APM ---------------------
apm_entry    dp    0
apm_vf        dd    0
align 4
sys_apm:
    cmp    word [apm_vf], 0    ; Check APM BIOS enable
    jne    @f
    or    [esp + 56], byte 1    ; error
    mov    [esp + 36], dword 8    ; 32-bit protected-mode interface not supported
    ret

@@:    xchg    eax, ecx
    xchg    ebx, ecx

    cmp    al, 3
    ja    @f
    and    [esp + 56], byte 0xfe    ; emulate func 0..3 as func 0
    mov    eax, [apm_vf]
    mov    [esp + 36], eax
    shr    eax, 16
    mov    [esp + 32], eax
    ret

@@:    call    pword [apm_entry]    ; call APM BIOS
    mov    [esp + 8 ], edi
    mov    [esp + 12], esi
    mov    [esp + 24], ebx
    mov    [esp + 28], edx
    mov    [esp + 32], ecx
    mov    [esp + 36], eax
    setc    al
    and    [esp + 56], byte 0xfe
    or    [esp + 56], al
    ret
; -----------------------------------------

align 4

undefined_syscall:                      ; Undefined system call

     mov   [esp+36],dword -1
     ret


;clear_busy_flag_at_caller:

;      push  edi

;      mov   edi,[CURRENT_TASK]    ; restore processes tss pointer in gdt, busyfl?
;      imul  edi,8
;      mov   [edi+gdts+ tss0 +5], word 01010000b *256 +11101001b

;      pop   edi

;      ret


keymap:

     db   '6',27
     db   '1234567890-=',8,9
     db   'qwertyuiop[]',13
     db   '~asdfghjkl;',39,96,0,'\zxcvbnm,./',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB<D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'


keymap_shift:

     db   '6',27
     db   '!@#$%^&*()_+',8,9
     db   'QWERTYUIOP{}',13
     db   '~ASDFGHJKL:"~',0,'|ZXCVBNM<>?',0,'45 '
     db   '@234567890123',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'AB>D',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'


keymap_alt:

     db   ' ',27
     db   ' @ $  {[]}\ ',8,9
     db   '            ',13
     db   '             ',0,'           ',0,'4',0,' '
     db   '             ',180,178,184,'6',176,'7'
     db   179,'8',181,177,183,185,182
     db   'ABCD',255,'FGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
     db   'ABCDEFGHIJKLMNOPQRSTUVWXYZ'


; device irq owners
uglobal
irq_owner:       ; process id

     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
     dd   0x0
endg


; on irq read ports
uglobal
  irq00read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq01read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq02read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq03read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq04read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq05read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq06read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq07read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq08read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq09read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq10read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq11read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq12read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq13read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq14read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  irq15read  dd  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
endg

; status
uglobal
  hd1_status                  dd 0x0  ; 0 - free : other - pid
  application_table_status    dd 0x0  ; 0 - free : other - pid
endg

; device addresses
uglobal
  mididp     dd 0x0
  midisp     dd 0x0

  cdbase     dd 0x0
  cdid       dd 0x0

  hdbase              dd   0x0  ; for boot 0x1f0
  hdid                dd   0x0
  hdpos               dd   0x0  ; for boot 0x1
  fat32part           dd   0x0  ; for boot 0x1

  ;part2_ld            dd   0x0

;* start code - Mario79
mouse_pause         dd   0
MouseTickCounter    dd   0
ps2_mouse_detected  db   0
com1_mouse_detected db   0
com2_mouse_detected db   0
;* end code - Mario79

wraw_bacground_select db 0
  lba_read_enabled    dd   0x0  ; 0 = disabled , 1 = enabled
  pci_access_enabled  dd   0x0  ; 0 = disabled , 1 = enabled

  sb16       dd 0x0

  buttontype         dd 0x0
  windowtypechanged  dd 0x0

align 4
  cpu_caps    dd 4 dup(0)
  pg_data  PG_DATA
  heap_test   dd ?
  hd_entries  rd 1     ;unused ? 0xfe10
endg

iglobal
  keyboard   dd 0x1
  sound_dma  dd 0x1
  syslang    dd 0x1
endg

if __DEBUG__ eq 1
  include_debug_strings
end if

IncludeIGlobals
endofcode:
IncludeUGlobals
uglobals_size = $ - endofcode
diff16 "end of kernel code",0,$

