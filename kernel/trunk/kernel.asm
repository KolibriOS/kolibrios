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
include "KGLOBALS.INC"
include "lang.inc"

WinMapAddress           equ     0x460000
display_data       = 0x460000

max_processes      equ   255

window_data        equ   0x0000
tss_data           equ   0xD20000
;tss_step           equ   (128+2048) ; tss & i/o - 16384 ports, * 256=557056
tss_step           equ   (128+8192) ; tss & i/o - 65535 ports, * 256=557056*4
draw_data          equ   0xC00000
sysint_stack_data  equ   0xC03000


twdw               equ   (0x3000-window_data)

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
                  org   0x10000
macro diff16 title,l2
 {
  local s,d,l1
  s = l2
  display title,': 0x'
  repeat 8
   d = 48 + s shr ((8-%) shl 2) and $0F
   if d > 57
    d = d + 65-57-1
   end if
   display d
  end repeat
  display 13,10
 }      
                  jmp   start_of_code

; mike.dld {
db 0
dd servetable-0x10000
draw_line       dd __sys_draw_line
disable_mouse   dd __sys_disable_mouse
draw_pointer    dd __sys_draw_pointer
drawbar         dd __sys_drawbar
putpixel        dd __sys_putpixel
; } mike.dld

version           db    'Kolibri OS  version 0.5.1.0      ',13,10,13,10,0
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

include "KERNEL16.INC"

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
tss0i          equ  tss0i_l-gdts
app_code       equ  3+app_code_l-gdts
app_data       equ  3+app_data_l-gdts



; CR0 Flags - Protected mode and Paging

        mov     ecx,0x00000001
        ;and     ebx,65535
        ;cmp     ebx,00100000000000000b ; lfb -> paging
        ;jb      no_paging
        ;mov     ax,0x0000
        ;mov     es,ax
        ;mov     al,[es:0x901E]
        ;cmp     al,1
        ;je      no_paging
        ;or      ecx, 0x80000000
       ;no_paging:

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
        lgdt    [cs:gdts-0x10000]       ; Load GDT
        mov     eax, cr0                ; Turn on paging // protected mode
        or      eax, ecx
        and     eax, 10011111b *65536*256 + 0xffffff ; caching enabled
        mov     cr0, eax
        jmp     byte $+2
        mov     ax,os_data              ; Selector for os
        mov     ds,ax
        mov     es,ax
        mov     fs,ax
        mov     gs,ax
        mov     ss,ax
        mov     esp,0x30000             ; Set stack
        jmp     pword os_code:B32       ; jmp to enable 32 bit mode

use32

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

         mov   edx,esi
.bll3:   inc   edx
         cmp   [edx],byte 0
         jne   .bll3
         sub   edx,esi
         mov   eax,10*65536
         mov   ax,word [boot_y]
         add   [boot_y],dword 10
         mov   ebx,0xffffff
         mov   ecx,esi
         mov   edi,1
         call  dtext

         mov   [novesachecksum],1000
         call  checkEgaCga

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

uglobal
  cpuid_0    dd  0,0,0,0
  cpuid_1    dd  0,0,0,0
  cpuid_2    dd  0,0,0,0
  cpuid_3    dd  0,0,0,0
endg

iglobal
  firstapp   db  'LAUNCHER   '
  char       db  'CHAR    MT '
  char2      db  'CHAR2   MT '
  bootpath   db  '/KOLIBRI    '
  bootpath2  db  0
  vmode      db  'VMODE   MDR'
  vrr_m      db  'VRR_M      '
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
;        movzx eax,byte [0x2f0000+0x9010]  ; mouse port
;        mov   [0xF604],byte 1  ;al
        mov   al,[0x2f0000+0x9000]        ; bpp
        mov   [0xFBF1],al
        movzx eax,word [0x2f0000+0x900A]  ; X max
        dec   eax
        mov   [0xfe00],eax
        movzx eax,word [0x2f0000+0x900C]  ; Y max
        dec   eax
        mov   [0xfe04],eax
        movzx eax,word [0x2f0000+0x9008]  ; screen mode
        mov   [0xFE0C],eax
        mov   eax,[0x2f0000+0x9014]       ; Vesa 1.2 bnk sw add
        mov   [0xE030],eax
        mov   [0xfe08],word 640*4         ; Bytes PerScanLine
        cmp   [0xFE0C],word 0x13          ; 320x200
        je    @f
        cmp   [0xFE0C],word 0x12          ; VGA 640x480
        je    @f
        mov   ax,[0x2f0000+0x9001]        ; for other modes
        mov   [0xfe08],ax
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
        mov     [0xfe80],eax

        cmp     [0xfe0c],word 0100000000000000b
        jge     setvesa20
        cmp     [0xfe0c],word 0x13
        je      v20ga32
        mov     [0xe020],dword Vesa12_putpixel24  ; Vesa 1.2
        mov     [0xe024],dword Vesa12_getpixel24
        cmp     [0xfbf1],byte 24
        jz      ga24
        mov     [0xe020],dword Vesa12_putpixel32
        mov     [0xe024],dword Vesa12_getpixel32
      ga24:
        jmp     v20ga24
      setvesa20:
        mov     [0xe020],dword Vesa20_putpixel24  ; Vesa 2.0
        mov     [0xe024],dword Vesa20_getpixel24
        cmp     [0xfbf1],byte 24
        jz      v20ga24
      v20ga32:
        mov     [0xe020],dword Vesa20_putpixel32
        mov     [0xe024],dword Vesa20_getpixel32
      v20ga24:
        cmp     [0xfe0c],word 0x12                ; 16 C VGA 640x480
        jne     no_mode_0x12
        mov     [0xe020],dword VGA_putpixel
        mov     [0xe024],dword Vesa20_getpixel32
      no_mode_0x12:

; MEMORY MODEL

;        mov     [0xfe84],dword 0x100000*16        ; apps mem base address
;        movzx   ecx,byte [0x2f0000+0x9030]
;        dec     ecx
;        mov     eax,16*0x100000 ; memory-16
;        shl     eax,cl
;        mov     [0xfe8c],eax      ; memory for use
;        cmp     eax,16*0x100000
;        jne     no16mb
;        mov     [0xfe84],dword 0xD80000 ; !!! 10 !!!
;      no16mb:

; init:
;  1) 0xFE84 - applications base
;  2) 0xFE8C - total amount of memory

        xor     edi, edi
  m_GMS_loop:
        add     edi, 0x400000
        mov     eax, dword [edi]
        mov     dword [edi], 'TEST'
        wbinvd
        cmp     dword [edi], 'TEST'
        jne     m_GMS_exit
        cmp     dword [0], 'TEST'
        je      m_GMS_exit
        mov     dword [es:edi], eax
        jmp     m_GMS_loop
  m_GMS_exit:
        mov     [edi], eax
        ; now edi contains the EXACT amount of memory

        mov     eax, 0x100000*16
        cmp     edi, eax ;0x100000*16
        jb      $                 ; less than 16 Mb

        mov     dword [0xFE84], eax ;0x100000*16
        cmp     edi, eax ;0x100000*16
        jne     @f
        mov     dword [0xFE84], 0xD80000 ; =0x100000*13.5
      @@:
        mov     dword [0xFE8C], edi
        
;!!!!!!!!!!!!!!!!!!!!!!!!!!
include 'detect/disks.inc'
;!!!!!!!!!!!!!!!!!!!!!!!!!!
        
; CHECK EXTRA REGION
; ENABLE PAGING
        mov     eax,cr0
        or      eax,0x80000000
        mov     cr0,eax
        jmp     $+2
        mov     dword [0xfe80],0x800000 
        
;Set base of graphic segment to linear address of LFB        
        mov     eax,[0xfe80]                      ; set for gs
        mov     [graph_data_l+2],ax
        shr     eax,16
        mov     [graph_data_l+4],al
        mov     [graph_data_l+7],ah             

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

        mov   [0x3000],dword 1
        mov   [0x3004],dword 1
        mov   [0x3010],dword 0x3020

        mov   eax,char
        mov   esi,12
        xor   ebx,ebx
        mov   ecx,26000
        mov   edx,0x37000
        call  fileread

        mov   eax,char2
        mov   esi,12
        xor   ebx,ebx
        mov   ecx,26000
        mov   edx,0x30000
        call  fileread

        mov   esi,boot_fonts
        call  boot_log

; PRINT AMOUNT OF MEMORY
        mov     esi, boot_memdetect
        call    boot_log

        movzx   ecx, word [boot_y]
        or      ecx, (10+29*6) shl 16 ; "Determining amount of memory"
        sub     ecx, 10
        mov     edx, 0xFFFFFF
        mov     ebx, [0xFE8C]
        shr     ebx, 20
        mov     edi, 1
        mov     eax, 0x00040000
        call    display_number
        
; CHECK EXTENDED REGION
;        mov     dword [0x80000000],0x12345678
;        cmp     dword [0x80000000],0x12345678
;        jz      extended_region_found
;        mov     esi,boot_ext_region
;        call    boot_log
;        jmp     $
;extended_region_found:
        
        call    MEM_Init
;add 0x800000-0xc00000 area        
        cmp     word [0xfe0c],0x13
        jle     .less_memory
        mov     eax,0x80000000      ;linear address
        mov     ebx,0x400000 shr 12 ;size in pages (4Mb)
        mov     ecx,0x800000        ;physical address
        jmp     .end_first_block
.less_memory:
        mov     eax,0x80180000      ;linear address
        mov     ebx,0x280000 shr 12 ;size in pages (2.5Mb)
        mov     ecx,0x980000        ;physical address
.end_first_block:                
        call    MEM_Add_Heap        ;nobody can lock mutex yet

        call    create_general_page_table
;add 0x1000000(0xd80000)-end_of_memory area
        mov     eax,second_base_address
        mov     ebx,[0xfe8c]
        mov     ecx,[0xfe84]
        sub     ebx,ecx
        shr     ebx,12
        add     eax,ecx
        call    MEM_Add_Heap
;init physical memory manager.
        call    Init_Physical_Memory_Manager
       
; REDIRECT ALL IRQ'S TO INT'S 0x20-0x2f

        mov   esi,boot_irqs
        call  boot_log
        call  rerouteirqs

        mov    esi,boot_tss
        call   boot_log

; BUILD SCHEDULER

        call   build_scheduler ; sys32.inc

; LOAD IDT
        ; <IP 05.02.2005>
        lidt   [cs:idtreg] ;[cs:idts]
        ; </IP>

; READ CPUID RESULT

        mov     esi,boot_cpuid
        call    boot_log
        pushfd                  ; get current flags
        pop     eax
        mov     ecx,eax
        xor     eax,0x00200000  ; attempt to toggle ID bit
        push    eax
        popfd
        pushfd                  ; get new EFLAGS
        pop     eax
        push    ecx             ; restore original flags
        popfd
        and     eax,0x00200000  ; if we couldn't toggle ID,
        and     ecx,0x00200000  ; then this is i486
        cmp     eax,ecx
        jz      nopentium
        ; It's Pentium or later. Use CPUID
        mov     edi,cpuid_0
        mov     esi,0
      cpuid_new_read:
        mov     eax,esi
        cpuid
        call    cpuid_save
        add     edi,4*4
        cmp     esi,3
        jge     cpuid_done
        cmp     esi,[cpuid_0]
        jge     cpuid_done
        inc     esi
        jmp     cpuid_new_read
      cpuid_save:
        mov     [edi+00],eax
        mov     [edi+04],ebx
        mov     [edi+8],ecx
        mov     [edi+12],edx
        ret
      cpuid_done:
      nopentium:

; CR4 flags - enable fxsave / fxrstore
;
;        finit
;        mov     eax,1
;        cpuid
;        test    edx,1000000h
;        jz      fail_fpu
;        mov     eax,cr4
;        or      eax,200h        ; Enable fxsave/fxstor
;        mov     cr4,eax
;     fail_fpu:

; DETECT DEVICES

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

;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;include 'detect/commouse.inc'
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
; SET MOUSE

        mov   esi,boot_setmouse
        call  boot_log
        call  setmouse

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
        ; name for OS/IDLE process
        mov  [0x80000+256+0],dword 'OS/I'
        mov  [0x80000+256+4],dword 'DLE '
        ; task list
        mov  [0x3004],dword 2         ; number of processes
        mov  [0x3000],dword 0         ; process count - start with os task
        mov  [0x3020+0xE],byte  1     ; on screen number
        mov  [0x3020+0x4],dword 1     ; process id number

        ; set default flags & stacks
        mov  [l.eflags],dword 0x11202 ; sti and resume
        mov  [l.ss0], os_data
        ;mov  [l.ss1], ring1_data
        ;mov  [l.ss2], ring2_data
        mov  [l.esp0], 0x52000
        mov  [l.esp1], 0x53000
        mov  [l.esp2], 0x54000
        ; osloop - TSS
        mov  eax,cr3
        mov  [l.cr3],eax
        mov  [l.eip],osloop
; <Ivan Poddubny 14/03/2004>
        mov  [l.esp],0x30000 ;0x2ffff
; </Ivan Poddubny 14/03/2004>
        mov  [l.cs],os_code
        mov  [l.ss],os_data
        mov  [l.ds],os_data
        mov  [l.es],os_data
        mov  [l.fs],os_data
        mov  [l.gs],os_data
        ; move tss to tss_data+tss_step
        mov  esi,tss_sceleton
        mov  edi,tss_data+tss_step
        mov  ecx,120/4
        cld
        rep  movsd

        mov  ax,tss0
        ltr  ax


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
        mov   [0xf600],eax          ; save tsc / sec

; SET VARIABLES

        call  set_variables

; STACK AND FDC

        call  stack_init
        call  fdc_init

; PALETTE FOR 320x200 and 640x480 16 col

        cmp   [0xfe0c],word 0x12
        jne   no_pal_vga
        mov   esi,boot_pal_vga
        call  boot_log
        call  paletteVGA
      no_pal_vga:

        cmp   [0xfe0c],word 0x13
        jne   no_pal_ega
        mov   esi,boot_pal_ega
        call  boot_log
        call  palette320x200
      no_pal_ega:

; LOAD DEFAULT SKIN

        call  load_default_skin

; MTRR'S

        call  enable_mtrr


; LOAD FIRST APPLICATION
        mov   [0x3000],dword 1 ;1
        mov   [0x3004],dword 1 ;1
        cli
        mov   al,[0x2f0000+0x9030]
        cmp   al,1
        jne   no_load_vrr_m
        mov   eax,vrr_m
        call  start_application_fl
        cmp   eax,2                  ; if no vrr_m app found
        je    first_app_found
        
    no_load_vrr_m:     
        mov   eax,firstapp
        call  start_application_fl

        cmp   eax,2                  ; if no first app found - halt
        je    first_app_found
        mov   eax, 0xDEADBEEF
        hlt    ;jmp   $
      first_app_found:
        cli

        mov   [0x3004],dword 2
        mov   [0x3000],dword 1


; START MULTITASKING

        mov   esi,boot_tasking
        call  boot_log

        mov   [0xe000],byte 1        ; multitasking enabled

    mov   al, 0xf6         ; —брос клавиатуры, разрешить сканирование
        call  kb_write

        mov     ecx,0
wait_loop_1:       ; variant 2
; читаем порт состо€ни€ процессора 8042
        in      al,64h 
    and     al,00000010b  ; флаг готовности
; ожидаем готовность процессора 8042
    loopnz  wait_loop_1

; SET KEYBOARD PARAMETERS
       ; mov   al, 0xED       ; svetodiody - only for testing!
       ; call  kb_write
       ; call  kb_read
       ; mov   al, 111b
       ; call  kb_write
       ; call  kb_read
       
        mov   al, 0xF3       ; set repeat rate & delay
        call  kb_write
        call  kb_read
        mov   al, 00100010b ; 24 500  ;00100100b  ; 20 500
        call  kb_write
        call  kb_read
     ;// mike.dld [
        call  set_lights
     ;// mike.dld ]


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

;        mov    [dma_hdd],1

        sti
        jmp   $                      ; wait here for timer to take control

        ; Fly :)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                                    ;
;                         MAIN OS LOOP                               ;
;                                                                    ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 32
osloop:

        call   check_mouse_data
        call   [draw_pointer]

        call   checkbuttons
        call   main_loop_sys_getkey
        call   checkwindows
        call   check_window_move_request

        call   checkmisc
        call   checkEgaCga

        call   stack_handler

        call   checkidle
        call   check_fdd_motor_status
        jmp    osloop
;temp_pointers:
; rd 32
; seed dd 0x12345678

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


include "KERNEL32.INC"


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                                      ;
;                       KERNEL FUNCTIONS                               ;
;                                                                      ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

enable_mtrr:

        pushad

        cmp    [0x2f0000+0x901c],byte 2
        je     no_mtrr
        mov    eax,[0xFE0C]                ; if no LFB then no MTRR
        test   eax,0100000000000000b
        jz     no_mtrr
        mov    edx,[cpuid_1+3*4]           ; edx - MTRR's supported ?
        test   edx,1000000000000b
        jz     no_mtrr
        call   find_empty_mtrr
        cmp    ecx,0
        jz     no_mtrr
        mov    esi,boot_mtrr               ; 'setting mtrr'
        call   boot_log
        mov    edx,0x0                     ; LFB , +8 M , write combine
        mov    eax,[0x2f9018]
        or     eax,1
        wrmsr
        inc    ecx
        mov    edx,0xf
        mov    eax,0xff800800
        wrmsr
        mov    ecx,0x2ff                   ; enable mtrr's
        rdmsr
        or     eax,100000000000b           ; set
        wrmsr
     no_mtrr:

        popad
        ret


find_empty_mtrr:  ; 8 pairs checked

        mov    ecx,0x201-2
      mtrr_find:
        add    ecx,2
        cmp    ecx,0x200+8*2
        jge    no_free_mtrr
        rdmsr
        test   eax,0x0800
        jnz    mtrr_find
        dec    ecx
        ret
      no_free_mtrr:
        mov    ecx,0
        ret

reserve_irqs_ports:

        pushad

        mov  [irq_owner+4*0],byte 1    ; timer
        mov  [irq_owner+4*1],byte 1    ; keyboard
        mov  [irq_owner+4*5],byte 1    ; sound blaster
        mov  [irq_owner+4*6],byte 1    ; floppy diskette
        mov  [irq_owner+4*13],byte 1   ; math co-pros
        mov  [irq_owner+4*14],byte 1   ; ide I
        mov  [irq_owner+4*15],byte 1   ; ide II
        movzx eax,byte [0xf604]        ; mouse irq
        dec   eax
        add   eax,mouseirqtable
        movzx eax,byte [eax]
        shl   eax,2
        mov   [irq_owner+eax],byte 1


                                       ; RESERVE PORTS
        mov   edi,1                    ; 0x00-0xff
        mov   [0x2d0000],edi
        shl   edi,4
        mov   [0x2d0000+edi+0],dword 1
        mov   [0x2d0000+edi+4],dword 0x0
        mov   [0x2d0000+edi+8],dword 0xff
        cmp   [0xf604],byte 2          ; com1 mouse -> 0x3f0-0x3ff
        jne   ripl1
        inc   dword [0x2d0000]
        mov   edi,[0x2d0000]
        shl   edi,4
        mov   [0x2d0000+edi+0],dword 1
        mov   [0x2d0000+edi+4],dword 0x3f0
        mov   [0x2d0000+edi+8],dword 0x3ff
      ripl1:
        cmp   [0xf604],byte 3          ; com2 mouse -> 0x2f0-0x2ff
        jne   ripl2
        inc   dword [0x2d0000]
        mov   edi,[0x2d0000]
        shl   edi,4
        mov   [0x2d0000+edi+0],dword 1
        mov   [0x2d0000+edi+4],dword 0x2f0
        mov   [0x2d0000+edi+8],dword 0x2ff
      ripl2:

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

uglobal
  novesachecksum dd 0x0
endg

checkEgaCga:

        cmp    [0xfe0c],dword 0x13
        je     cnvl
        ret
      cnvl:

        pushad
        mov    ecx,[0xfb0a]
        cmp    ecx,[novesachecksum]
        jne    novesal
        popad
        ret

      novesal:
        mov    [novesachecksum],ecx
        mov    ecx,0
        movzx  eax,word [0xfb0c]
        cmp    eax,100
        jge    m13l3
        mov    eax,100
      m13l3:
        cmp    eax,480-100
        jbe    m13l4
        mov    eax,480-100
      m13l4:
        sub    eax,100
        imul   eax,640*4
        add    ecx,eax
        movzx  eax,word [0xfb0a]
        cmp    eax,160
        jge    m13l1
        mov    eax,160
      m13l1:
        cmp    eax,640-160
        jbe    m13l2
        mov    eax,640-160
      m13l2:
        sub    eax,160
        shl    eax,2
        add    ecx,eax
        mov    esi,[0xfe80]
        add    esi,ecx
        mov    edi,0xa0000
        mov    edx,200
        mov    ecx,320
        cld
     m13pix:
        lodsd
        push   eax
        mov    ebx,eax
        and    eax,(128+64+32)      ; blue
        shr    eax,5
        and    ebx,(128+64+32)*256  ; green
        shr    ebx,8+2
        add    eax,ebx
        pop    ebx
        and    ebx,(128+64)*256*256 ; red
        shr    ebx,8+8
        add    eax,ebx
        stosb
        loop   m13pix
        mov    ecx,320
        add    esi,4*(640-320)
        dec    edx
        jnz    m13pix

        popad
        ret


palette320x200:

       mov   edx,0x3c8
       xor   eax, eax
       out   dx,al
       mov   ecx,256
       mov   edx,0x3c9
       xor   eax,eax

     palnew:
       mov   al,0
       test  ah,64
       jz    pallbl1
       add   al,21
     pallbl1:
       test  ah,128
       jz    pallbl2
       add   al,42
     pallbl2:
       out   dx,al
       mov   al,0
       test  ah,8
       jz    pallbl3
       add   al,8
     pallbl3:
       test  ah,16
       jz    pallbl4
       add   al,15
     pallbl4:
       test  ah,32
       jz    pallbl5
       add   al,40
     pallbl5:
       out   dx,al
       mov   al,0
       test  ah,1
       jz    pallbl6
       add   al,8
     pallbl6:
       test  ah,2
       jz    pallbl7
       add   al,15
     pallbl7:
       test  ah,4
       jz    pallbl8
       add   al,40
     pallbl8:
       out   dx,al
       add   ah,1
       loop  palnew

       ret

set_variables:

        mov   ecx,0x100                       ; flush port 0x60
.fl60:  in    al,0x60
        loop  .fl60
        mov   [0xfcff],byte 0                 ; mouse buffer
        mov   [0xf400],byte 0                 ; keyboard buffer
        mov   [0xf500],byte 0                 ; button buffer
;        mov   [0xfb0a],dword 100*65536+100    ; mouse x/y

        push  eax
        mov   ax,[0x2f0000+0x900c]
        shr   ax,1
        shl   eax,16
        mov   ax,[0x2f0000+0x900A]
        shr   ax,1
        mov   [0xfb0a],eax
        pop   eax
        
        mov   byte [SB16_Status],0            ; Minazzi Paolo
        mov   [display_data-12],dword 1       ; tiled background
        mov   [0xfe88],dword 0x2C0000         ; address of button list

     ;!! IP 04.02.2005:
        mov   [next_usage_update], 100
        mov   byte [0xFFFF], 0 ; change task if possible

        ret

;* mouse centered - start code- Mario79
mouse_centered:
        push  eax
        mov   eax,[0xFE00]
        shr   eax,1
        mov   [0xFB0A],ax
        mov   eax,[0xFE04]
        shr   eax,1
        mov   [0xFB0C],ax
        pop   eax
        ret
;* mouse centered - end code- Mario79

align 4

sys_outport:

    mov   edi,ebx          ; separate flag for read / write
    and   ebx,65535

    mov   ecx,[0x2d0000]
    test  ecx,ecx
    jne   sopl8
    mov   [esp+36],dword 1
    ret

  sopl8:
    mov   edx,[0x3010]
    mov   edx,[edx+0x4]
    and   ebx,65535
    cld
  sopl1:

    mov   esi,ecx
    shl   esi,4
    add   esi,0x2d0000
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


align 4

sys_wss:

     cmp  word [wss],word 0
     jnz  wssl1
     mov  [esp+36],dword 1
     ret
   wssl1:

     cmp  eax,1    ; set volume - main
     jnz  wssl2
     mov  [esp+36],dword 0
     ret
   wssl2:

     cmp  eax,2    ; set volume - cd
     jnz  wssl3
     ; L
     mov  dx,word [wss]
     add  dx,4
     mov  al,0x2
     out  dx,al
     mov  esi,1
     call delay_ms
     mov  eax,ebx
     inc  edx
     out  dx,al
     ; R
     mov  dx,word [wss]
     add  dx,4
     mov  al,0x3
     out  dx,al
     mov  esi,1
     call delay_ms
     mov  eax,ebx
     inc  edx
     out  dx,al
     mov  [esp+36],dword 0
     ret
   wssl3:
     mov   [esp+36],dword 2
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

     cmp   eax,0xffff            ; length > 0 ?
     jge   cont_displ
     ret
   cont_displ:

     cmp   eax,60*0x10000        ; length <= 60 ?
     jbe   cont_displ2
     ret
   cont_displ2:

     pushad

     cmp   al,1                  ; ecx is a pointer ?
     jne   displnl1
     mov   edi,[0x3010]
     mov   edi,[edi+0x10]
     mov   ebx,[edi+ebx]
   displnl1:
     sub   esp,64

     cmp   ah,0                  ; DECIMAL
     jne   no_display_desnum
     shr   eax,16
     and   eax,0x2f
     push  eax
     ;mov   edi,[0x3010]
     ;mov   edi,[edi+0x10]
     mov   edi,esp
     add   edi,4+64
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
     and   eax,0x2f
     push  eax
     ;mov   edi,[0x3010]
     ;mov   edi,[edi+0x10]
     mov   edi,esp
     add   edi,4+64
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
     and   eax,0x2f
     push  eax
     ;mov   edi,[0x3010]
     ;mov   edi,[edi+0x10]
     mov   edi,esp
     add   edi,4+64
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

     mov   edx,eax
     mov   ecx,65
     sub   ecx,eax
     add   ecx,esp
     add   ecx,4
     mov   eax,[esp+64+32-8+4]
     mov   ebx,[esp+64+32-12+4]
     push  edx                       ; add window start x & y
     push  ebx
     mov   edx,[0x3010]
     mov   ebx,[edx-twdw]
     shl   ebx,16
     add   ebx,[edx-twdw+4]
     add   eax,ebx
     pop   ebx
     pop   edx
     mov   edi,0
     call  dtext

     ret


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
; 6=wss base , base io address
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

midi_base dw 0

   nsyse1:

     cmp  eax,2                      ; KEYBOARD
     jnz  nsyse2
     cmp  ebx,1
     jnz  kbnobase
     mov  edi,[0x3010]
     add  ecx,[edi+0x10]
     mov  eax,ecx
     mov  ebx,keymap
     mov  ecx,128
     call memmove
     ret
   kbnobase:
     cmp  ebx,2
     jnz  kbnoshift
     mov  edi,[0x3010]
     add  ecx,[edi+0x10]
     mov  eax,ecx
     mov  ebx,keymap_shift
     mov  ecx,128
     call memmove
     ret
   kbnoshift:
     cmp  ebx,3
     jne  kbnoalt
     mov  edi,[0x3010]
     add  ecx,[edi+0x10]
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

     cmp  eax,6                      ; WSS
     jnz  nsyse6
     cmp  ebx,0x100
     jb   nsyse6
     mov  [wss],ebx
     ret

wss_temp dd 0

   nsyse6:

     cmp  eax,7                      ; HD BASE
     jne  nsyse7
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
     mov   [0xfe10],dword 0
    call  reserve_hd1
    call  clear_hd_cache
    mov   [hd1_status],0        ; free
     ret

hd_base db 0

   nsyse7:

     cmp  eax,8                      ; HD PARTITION
     jne  nsyse8
     mov  [fat32part],ebx
;     call set_FAT32_variables
    call  reserve_hd1
    call  clear_hd_cache
     pusha
     call  choice_necessity_partition_1
     popa
    mov   [hd1_status],0        ; free
     ret
   nsyse8:

     cmp  eax,10                     ; SOUND DMA CHANNEL
     jne  no_set_sound_dma
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

     mov  [esp+36],dword -1
     ret


align 4

sys_getsetup:

; 1=roland mpu midi base , base io address
; 2=keyboard   1, base kaybap 2, shift keymap, 9 country 1eng 2fi 3ger 4rus
; 3=cd base    1, pri.master 2, pri slave 3 sec master, 4 sec slave
; 4=sb16 base , base io address
; 5=system language, 1eng 2fi 3ger 4rus
; 6=wss base
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
     mov  edi,[0x3010]
     add  ecx,[edi+0x10]
     mov  ebx,ecx
     mov  eax,keymap
     mov  ecx,128
     call memmove
     ret
   kbnobaseret:
     cmp  ebx,2
     jnz  kbnoshiftret
     mov  edi,[0x3010]
     add  ecx,[edi+0x10]
     mov  ebx,ecx
     mov  eax,keymap_shift
     mov  ecx,128
     call memmove
     ret
   kbnoshiftret:
     cmp  ebx,3
     jne  kbnoaltret
     mov  edi,[0x3010]
     add  ecx,[edi+0x10]
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
     cmp  eax,6
     jnz  ngsyse6
     mov  eax,[wss]
     mov  [esp+36],eax
     ret
   ngsyse6:
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


align 4

readmousepos:

; eax=0 screen relative
; eax=1 window relative
; eax=2 buttons pressed

    test eax,eax
    jnz  nosr
    mov  eax,[0xfb0a]
    shl  eax,16
    mov  ax,[0xfb0c]
    mov  [esp+36],eax
    ret
  nosr:

    cmp  eax,1
    jnz  nowr
    mov  eax,[0xfb0a]
    shl  eax,16
    mov  ax,[0xfb0c]
    mov  esi,[0x3010]
    sub  esi,twdw
    mov  bx,[esi]
    shl  ebx,16
    mov  bx,[esi+4]
    sub  eax,ebx
    mov  [esp+36],eax
    ret
  nowr:

    cmp   eax,2
    jnz   nomb
    movzx eax,byte [0xfb40]
  nomb:
    mov   [esp+36],eax

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
;include 'detect/dev_fd.inc'
;include 'detect/dev_hdcd.inc'
;include 'detect/sear_par.inc'
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    ret


sys_end:

     mov   eax,[0x3010]
     add   eax,0xa
     mov   [eax],byte 3  ; terminate this program
     
    waitterm:            ; wait here for termination
     mov   eax,5
     call  delay_hs
     jmp   waitterm

sys_system:

     cmp  eax,1                              ; BOOT
     jnz  nosystemboot
     mov  [0x2f0000+0x9030],byte 0
  for_shutdown_parameter:     
     mov  eax,[0x3004]
     add  eax,2
     mov  [shutdown_processes],eax
     mov  [0xFF00],al
     xor  eax, eax
     ret
  uglobal
   shutdown_processes: dd 0x0
  endg
   nosystemboot:

     cmp  eax,2                              ; TERMINATE
     jnz  noprocessterminate
     cmp  ebx,2
     jb   noprocessterminate
     mov  edx,[0x3004]
     cmp  ebx,edx
     jg   noprocessterminate
     mov  eax,[0x3004]
     shl  ebx,5
     mov  edx,[ebx+0x3000+4]
     add  ebx,0x3000+0xa
     
     ;call MEM_Heap_Lock      ;guarantee that process isn't working with heap
     mov  [ebx],byte 3       ; clear possible i40's
     ;call MEM_Heap_UnLock

     cmp  edx,[application_table_status]    ; clear app table stat
     jne  noatsc
     mov  [application_table_status],0
   noatsc:
     ret
   noprocessterminate:

     cmp  eax,3                              ; ACTIVATE WINDOW
     jnz  nowindowactivate
     cmp  ebx,2
     jb   nowindowactivate
     cmp  ebx,[0x3004]
     ja   nowindowactivate
     ; edi = position at window_data+
     mov  edi, ebx          ; edi = process number
     ;shl  ebx, 1
     ;add  ebx, 0xc000
     ;mov  esi, [ebx]        ; esi = window stack value
     ;and  esi, 0xffff       ;       word
    movzx esi, word [0xC000 + ebx*2]
     mov  edx, [0x3004] ; edx = number of processes
     cmp  esi, edx
     jz   nowindowactivate ; continue if window_stack_value != number_of_processes
                           ;     i.e. if window is not already active

;* start code - get active process (1) - Mario79
     mov  [window_minimize],2
     mov  [active_process],edi
;* end code - get active process (1) - Mario79

     mov  [0xff01],edi     ; activate
     xor  eax, eax
     ret
     
   nowindowactivate:

     cmp  eax,4                              ; GET IDLETIME
     jnz  nogetidletime
     mov  eax,[idleusesec]
     ret
   nogetidletime:

     cmp  eax,5                              ; GET TSC/SEC
     jnz  nogettscsec
     mov  eax,[0xf600]
     ret
   nogettscsec:

;  SAVE ramdisk to /hd/1/menuet.img
;!!!!!!!!!!!!!!!!!!!!!!!!
   include 'blkdev/rdsave.inc'
;!!!!!!!!!!!!!!!!!!!!!!!!
;* start code - get active process (2) - Mario79
     cmp  eax,7
     jnz  nogetactiveprocess
     mov  eax,[active_process]
     ret
 nogetactiveprocess:  
     cmp  eax,8
     jnz  nosoundflag
     cmp  ebx,1
     jne  nogetsoundflag
     movzx  eax,byte [sound_flag] ; get sound_flag
     ret
 nogetsoundflag:
     cmp  ebx,2
     jnz  nosoundflag
     inc  byte [sound_flag]       ; set sound_flag
     and byte [sound_flag],1      ;
     ret     
nosoundflag:
     cmp  eax,9                   ; system shutdown with param
     jnz  noshutdownsystem
     cmp  ebx,1
     jl  exit_for_anyone
     cmp  ebx,4
     jg   exit_for_anyone
     mov  [0x2f0000+0x9030],bl
     jmp  for_shutdown_parameter
noshutdownsystem:
     cmp  eax,10                   ; minimize window
     jnz  nominimizewindow
     mov   [window_minimize],1
 exit_for_anyone:
     ret
nominimizewindow:
     cmp  eax,11           ; get disk info table
     jnz  nogetdiskinfo
     cmp  ebx,1
     jnz  full_table
  small_table:
     call for_all_tables
     mov cx,10
     cld
     rep movsb
     ret
   for_all_tables:
     mov edi,[3010h]
     mov edi,[edi+10h]
     add edi,ecx
     mov esi,0x40000
     xor ecx,ecx
     ret
  full_table:
     cmp  ebx,2
     jnz  exit_for_anyone
     call for_all_tables
     mov cx,16384
     cld
     rep movsd
     ret
nogetdiskinfo:
     cmp  eax,12      ; get all key pressed with ALT
     jnz  nogetkey
     mov   eax,[last_key_press]
     mov   al,[keyboard_mode_sys]
     mov   [esp+36],eax
     mov   [last_key_press],0
 .finish:
     ret
nogetkey:
     cmp  eax,13      ; get kernel ID and version
     jnz  nogetkernel_id
     mov edi,[3010h]
     mov edi,[edi+10h]
     add edi,ebx
     mov esi,version_inf
     mov ecx,version_end-version_inf
     cld
     rep movsb
     ret
nogetkernel_id:
     cmp  eax,14      ; sys wait retrace
     jnz  nosys_wait_retrace
     ;wait retrace functions 
 sys_wait_retrace: 
     mov edx,0x3da 
 WaitRetrace_loop: 
     in al,dx 
     test al,1000b 
     jz WaitRetrace_loop 
     mov [esp+36],dword 0 
     ret
nosys_wait_retrace:
     cmp  eax,15      ; mouse centered
     jnz  no_mouse_centered
     call  mouse_centered
     mov [esp+36],dword 0
     ret
no_mouse_centered:
;* end  code - get active process (2) - Mario79
     ret
window_minimize db 0
sound_flag      db 0
last_key_press  dd 0
keyboard_mode_sys db 0

iglobal 
version_inf: 
  db 0,5,1,0  ; version 0.5.1.0 
  db UID_KOLIBRI 
  db 'Kolibri',0 
version_end: 
endg 

UID_NONE=0 
UID_MENUETOS=1   ;official
UID_KOLIBRI=2    ;russian

main_loop_sys_getkey:
    cmp   [0xf400],byte 0
    je    .finish
    movzx eax,byte [0xf401]
    shl   eax,8
    mov   [last_key_press],eax
 .finish:
    ret

sys_cachetodiskette:
    pushad
    cmp  eax,1
    jne  no_write_all_of_ramdisk

    call fdc_writeramdisk
    popad
    ret
  no_write_all_of_ramdisk:
    cmp eax,2
    jne no_write_part_of_ramdisk
    call fdc_commitflush
    popad
    ret
  no_write_part_of_ramdisk:
    cmp  eax,3
    jne  no_set_fdc
    call fdc_set
    popad
    ret
  no_set_fdc:
    cmp  eax,4
    jne  no_get_fdc
    popad
    call fdc_get
    mov    [esp+36],ecx
    ret
  no_get_fdc:
    popad
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
    mov   [ebx+0x300000],edx
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
    mov   [0xfff0],byte 1
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
    mov   edi, [0x3010]
    add   ebx, [edi+0x10]
 ;   mov   esi, ebx
 ;   mov   edi, ecx
    mov   eax, ebx
    mov   ebx, ecx
    add   ecx, edx
    cmp   ecx, 0x160000-16
    ja    .fin
 ;   add   edi, 0x300000
    add   ebx, 0x300000
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
    mov   eax, [ebx+0x300000]
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
    mov   ebx, [0x3000]                          ; TOP OF WINDOW STACK
    movzx ecx,word [0xC000 + ebx * 2]
    mov   edx,[0x3004]
    cmp   ecx,edx
    jne   .finish
    cmp   [0xf400],byte 0
    je    .finish
    movzx eax,byte [0xf401]
    shl   eax,8
    push  eax
    dec   byte [0xf400]
    and   byte [0xf400],127
    movzx ecx,byte [0xf400]
    add   ecx,2
 ;   mov   esi,0xf402
 ;   mov   edi,0xf401
 ;   cld
 ;  rep   movsb
    mov   eax, 0xF402
    mov   ebx, 0xF401
    call  memmove
    pop   eax
    mov   [last_key_press],eax

    mov   eax,[kb_state]
    and   al,110000b
    cmp   al,100000b
    je    .yes_win_key
    cmp   al,10000b
    je    .yes_win_key
    mov   eax,[last_key_press]
    jmp   .no_win_key
;    cmp   ah,232
;    je    .yes_win_key
;    cmp   ah,233
;    jne   .no_win_key
 .yes_win_key:
    mov   eax,1
 .no_win_key:
    mov   [esp+36],eax
 .finish:
    ret


align 4

sys_getbutton:

    mov   ebx, [0x3000]                         ; TOP OF WINDOW STACK
    mov   [esp+36],dword 1
    movzx ecx, word [0xC000 + ebx * 2]
    mov   edx, [0x3004] ; less than 256 processes
    cmp   ecx,edx
    jne   .exit
    movzx eax,byte [0xf500]
    test  eax,eax
    jz    .exit
    mov   eax,[0xf501]
    shl   eax,8
    mov   [0xf500],byte 0
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

    mov  edi,[0x3010]   ; eax = return area
    add  edi,0x10
    add  eax,[edi]

    cmp  ebx,-1         ; who am I ?
    jne  no_who_am_i
    mov  ebx,[0x3000]
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
    add  eax,0x3000+0x1c
    mov  ebx,eax
    pop  eax
    mov  ecx,[ebx]
    mov  [eax],ecx
    pop  ebx
;    mov  ebx,[esp]
;    shl  ebx,1
;    add  ebx,0xc000
    mov  cx, [0xC000 + ebx * 2]
    mov  [eax+4],cx
;    mov  ebx,[esp]
;    shl  ebx,1
;    add  ebx,0xc400
    mov  cx, [0xC400 + ebx * 2]
    mov  [eax+6],cx
;    pop  ebx
    push eax
    mov  eax,ebx
    shl  eax,8
    add  eax,0x80000
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
    mov    edx,[0x80000+ecx+0x8c]
    mov    eax,std_application_base_address
    ;add    ecx,0x80000+0x88
    ;mov    ecx,[ecx]
;    shl    ecx,3
    ; eax run base -> edx used memory
;    mov    al,[ecx+gdts+ app_code-3 +4]        ;  base  23:16
;    mov    ah,[ecx+gdts+ app_code-3 +7]        ;  base  31:24
;    shl    eax,16
;    mov    ax,[ecx+gdts+ app_code-3 +2]        ;  base  0:15
;    movzx  edx,word [ecx+gdts+ app_code-3 +0]
;    shl    edx,12

  os_mem:
    dec    edx
    mov    [ebx+12],eax
    mov    [ebx+16],edx

    ; PID (+30)

    mov    eax,[esp]
    shl    eax,5
    add    eax,0x3000+0x4
    mov    eax,[eax]
    mov    [ebx+20],eax

    ; window position and size

    mov    esi,[esp]
    shl    esi,5
    add    esi,window_data
    mov    edi,[esp+4]
    add    edi,34
    mov    ecx,4*4
    cld
    rep    movsb

    ; Process state (+50)

    mov    eax,[esp]
    shl    eax,5
    add    eax,0x3000+0xa
    mov    eax,[eax]
    mov    [ebx+40],ax


    pop    ebx
    pop    eax

    ; return number of processes

    mov    eax,[0x3004]
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
        mov     al,6            ; day of week
        out     0x70,al
        in      al,0x71
        mov     ch,al
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

    mov   ecx,[0x3000]

  sys_newba2:

    mov   edi,[0xfe88]
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

    mov   edx,[0x3010]      ; return whole screen draw area for this app
    add   edx,draw_data-0x3000
    mov   [edx+0],dword 0
    mov   [edx+4],dword 0
    mov   eax,[0xfe00]
    mov   [edx+8],eax
    mov   eax,[0xfe04]
    mov   [edx+12],eax

    mov   edi,[0x3010]
    sub   edi,twdw
    mov   [edi+30],byte 1   ; no new position & buttons from app

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
    call  sys_set_window
    call  drawwindow_I
    ret
  nosyswI:

    cmp   edi,1   ; type II   - only reserve area, no draw
    jne   nosyswII
    call  sys_set_window
    call  sys_window_mouse
    ret
  nosyswII:

    cmp   edi,2   ; type III  - new style
    jne   nosyswIII
    call  sys_set_window
    call  drawwindow_III
    ret
  nosyswIII:

    cmp   edi,3   ; type IV - skinned window
    jne   nosyswIV
    call  sys_set_window
    call  drawwindow_IV
    ret
  nosyswIV:

    ret


sys_set_window:

    mov   edi,[0x3000]
    shl   edi,5
    add   edi,window_data

    ; colors
    mov   [edi+16],ecx
    mov   [edi+20],edx
    mov   [edi+24],esi

    ; check flag (?)
    cmp   [edi+30],byte 1
    jz    newd

    push  eax
    mov   eax,[timer_ticks] ;[0xfdf0]
    add   eax,100
    mov   [new_window_starting],eax
    pop   eax

    mov   [edi+8],ax
    mov   [edi+12],bx
    shr   eax,16
    shr   ebx,16
    mov   [edi+00],ax
    mov   [edi+04],bx


    call  check_window_position


    push  ecx esi edi               ; save for window fullscreen/resize
    mov   esi,edi
    sub   edi,window_data
    shr   edi,5
    shl   edi,8
    add   edi,0x80000+0x90
    mov   ecx,4
    cld
    rep   movsd
    pop   edi esi ecx

    push  eax ebx ecx edx
;;;    mov   eax, 1
;;;    call  delay_hs
    movzx eax, word [edi+00]
    movzx ebx, word [edi+04]
    movzx ecx, word [edi+8]
    movzx edx, word [edi+12]
    add   ecx, eax
    add   edx, ebx
    call  calculatescreen
    pop   edx ecx ebx eax

    mov   [0xf400],byte 0           ; empty keyboard buffer
    mov   [0xf500],byte 0           ; empty button buffer

  newd:
    mov   [edi+31],byte 0   ; no redraw
    mov   edx,edi

    ret


sys_window_move:

        cmp  [window_move_pr],0
        je   mwrl1

        mov  [esp+36],dword 1         ; return queue error

        ret

     mwrl1:

        mov   edi,[0x3000]            ; requestor process base
        mov   [window_move_pr],edi

        mov   [window_move_eax],eax
        mov   [window_move_ebx],ebx
        mov   [window_move_ecx],ecx
        mov   [window_move_edx],edx

        mov   [esp+36],dword 0        ; return success

        ret

type_background_1:
    cmp   [0xfff0],byte 0               ; background update ?
    jz    temp_nobackgr
    mov   [0xfff0],byte 2
    call  change_task
    mov   [draw_data+32+0],dword 0
    mov   [draw_data+32+4],dword 0
    mov   eax,[0xfe00]
    mov   ebx,[0xfe04]
    mov   [draw_data+32+8],eax
    mov   [draw_data+32+12],ebx
    call  drawbackground
    mov   [0xfff0],byte 0
    mov   [0xfff4],byte 0
temp_nobackgr:
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
       btr eax,8 ;pce=cr4[8]
       mov cr4,eax
       mov [esp+36],eax
ret
;---------------------------------------------------------------------------------------------

; check pixel limits

;cplimit:
;        push    edi

;        cmp     byte [0xe000], 1      ; Multitasking enabled?
;        jnz     .ret0
;        mov     edi,[0x3010]
;        add     edi, draw_data-0x3000
;        mov     ecx, 1
;        cmp     [edi+0], eax  ; xs
;        ja      .ret1
;        cmp     [edi+4], ebx  ; ys
;        ja      .ret1
;        cmp     eax, [edi+8]   ; xe
;        ja      .ret1
;        cmp     ebx, [edi+12] ; ye
;        ja      .ret1

;.ret0:
;        xor     ecx, ecx
;.ret1:
;        pop     edi
;        ret


; check if pixel is allowed to be drawn

checkpixel:

        push eax
        push ebx
        push edx

;        mov   ecx,[0x3000]  ; process count
;        shl   ecx, 6        ; *64
;        add   ecx,0xc000    ; +window_stack
;        mov   dx,word [ecx] ; window_stack_value

;        cmp   dx, word [0x3004] ; is this window active right now?!
;        jz    .ret0

;        call  cplimit
;        test  ecx, ecx
;        jnz   .ret1

        mov  edx,[0xfe00]     ; screen x size
        inc  edx
        imul  edx, ebx
        mov  dl, [eax+edx+display_data] ; lea eax, [...]
;;;        mov  dl,[eax]

        mov  eax,[0x3000]
        shl  eax,5
        add  eax,0x3000+0xe

        mov  ecx, 1
        cmp  byte [eax], dl
        jnz  .ret1

.ret0:
        xor  ecx, ecx
.ret1:
        pop  edx
        pop  ebx
        pop  eax
        ret

uglobal
  mouse_active  db  0
endg
iglobal
  cpustring db 'CPU        '
endg


align 4
; check misc

checkmisc:

    cmp   [ctrl_alt_del], 1
    jne   nocpustart
    mov   eax, cpustring
    call  start_application_fl
    mov   [ctrl_alt_del], 0
  nocpustart:
    cmp   [mouse_active], 1
    jne   mouse_not_active
    mov   [mouse_active], 0
    xor   edi, edi
    mov   ecx, [0x3004]
   set_mouse_event:
    add   edi, 256
    or    [edi+0x80000+0xA8], dword 00100000b
    loop  set_mouse_event
  mouse_not_active:


    cmp   [0xfff0],byte 0               ; background update ?
    jz    nobackgr
    mov   [0xfff0],byte 2
    call  change_task
    mov   [draw_data+32+0],dword 0
    mov   [draw_data+32+4],dword 0
    mov   eax,[0xfe00]
    mov   ebx,[0xfe04]
    mov   [draw_data+32+8],eax
    mov   [draw_data+32+12],ebx
    call  drawbackground
    mov   [0xfff0],byte 0
    mov   [0xfff4],byte 0

  nobackgr:


    ; system shutdown request

    cmp  [0xFF00],byte 0
    je   noshutdown

    mov  edx,[shutdown_processes]
    sub  dl,2

    cmp  [0xff00],dl
    jne  no_mark_system_shutdown

    mov   edx,0x3040
    movzx ecx,byte [0xff00]
    add   ecx,5
  markz:
    mov   [edx+0xa],byte 3
    add   edx,0x20
    loop  markz

  no_mark_system_shutdown:

    call [disable_mouse]

    dec  byte [0xff00]

    cmp  [0xff00],byte 0
    je   system_shutdown

  noshutdown:


    mov   eax,[0x3004]                  ; termination
    mov   ebx,0x3020+0xa
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

         mov   eax,[edi+0]
         mov   ebx,[edi+4]
         mov   ecx,[edi+8]
         mov   edx,[edi+12]
         add   ecx,eax
         add   edx,ebx

         mov   ecx,[dlye]   ; ecx = area y end     ebx = window y start
         cmp   ecx,ebx
         jb    ricino

         mov   ecx,[dlxe]   ; ecx = area x end     eax = window x start
         cmp   ecx,eax
         jb    ricino

         mov   eax,[edi+0]
         mov   ebx,[edi+4]
         mov   ecx,[edi+8]
         mov   edx,[edi+12]
         add   ecx,eax
         add   edx,ebx

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
         mov   [eax+0],ebx
         mov   ebx,[dly]
         mov   [eax+4],ebx
         mov   ebx,[dlxe]
         mov   [eax+8],ebx
         mov   ebx,[dlye]
         mov   [eax+12],ebx

         sub   eax,draw_data-window_data

         cmp   ecx,1
         jne   nobgrd
         cmp   esi,1
         je    newdw8
         call  drawbackground

       newdw8:
       nobgrd:

         mov   [eax+31],byte 1    ; mark as redraw

       ricino:

       not_this_task:

         pop   ecx

         cmp   ecx,[0x3004]
         jle   newdw2
;         jg    newdw3
;         jmp   newdw2

;       newdw3:

         pop  eax
         popad

         ret

;   check mouse
;
;
;   FB00  ->   FB0F   mouse memory 00 chunk count - FB0A-B x - FB0C-D y
;   FB10  ->   FB17   mouse color mem
;   FB21              x move
;   FB22              y move
;   FB30              color temp
;   FB28              high bits temp
;   FB4A  ->   FB4D   FB4A-B x-under - FB4C-D y-under
;   FC00  ->   FCFE   com1/ps2 buffer
;   FCFF              com1/ps2 buffer count starting from FC00

uglobal
  mousecount  dd  0x0
  mousedata   dd  0x0
endg


check_mouse_data:

        pushad

        cmp    [0xF604],byte 1
        jne    no_ps2_mouse
        mov    [mousecount],dword 0x2e0000+12*4096
        mov    [mousedata],dword 0x2e0000+12*4096+0x10
        jmp    uusicheckmouse
no_ps2_mouse:        
        cmp    [0xF604],byte 2
        jne    no_com1_mouse
        mov    [mousecount],dword 0x2e0000+4*4096
        mov    [mousedata],dword 0x2e0000+4*4096+0x10
        jmp    uusicheckmouse
no_com1_mouse:
        mov    [mousecount],dword 0x2e0000+3*4096
        mov    [mousedata],dword 0x2e0000+3*4096+0x10

      uusicheckmouse:

        mov    ebx,[mousecount]       ; anything at buffer for mouse
        cmp    dword [ebx], 0         ; !!!
        jz     checkmouseret

        ; first byte of comX or ps2 ?

        cmp    [0xF604],byte 1
        je     ps2mousefirst

        ; ******************************************        
        ; *********** COMX mouse driver ************
        ; ******************************************

       com1mousefirst:

        mov    edi,[mousedata]
        mov    dl,byte [edi] ; first com1 ?
        test   dl,64
        jz     @f
        mov    [0xfb00],byte 0  ; zero mouse block count
       @@:
        xor    ebx,ebx

        mov    bl,[0xfb00]
        inc    bl
        mov    [0xfb00],bl
        mov    eax,0xfb00
        add    eax,ebx
        mov    edi,[mousedata]
        mov    dl,byte [edi]
        mov    [eax],byte dl
        cmp    bl,3             ; three ?
        jnz    decm

        ; buttons

;* change right and left button by places - start code - Mario79
        mov al,[0xfb01]
        mov ah,al
        shr al,3
        and al,2
        shr ah,5
        and ah,1
        add al,ah
;* change right and left button by places - end code - Mario79

        mov    [0xfb40],al

        ; com1 mouse
        ; x

        mov    dl,[0xfb01]        ; x high bits
        movzx  eax,dl
        and    al,3
        shl    al,6
        mov    dl,byte[0xfb02]    ; x low bits
        add    al,dl
        mov    [0xfb21],byte al
        movzx  ebx,word[0xfb0a]

        mov    al,byte [0xfb01]   ; + or - ?
        test   al,2
        jz     x_add

       x_sub:                      ; x-
        sub    bx,255
        sub    bx,255
       x_add:                      ; x+
        movzx  eax,byte [0xfb21]
        add    bx,ax
        add    bx,ax
        push   ebx
        mov    [0xfb00],byte 0

        ; y


      my_event:

        mov    dl,[0xfb01]       ; y high bits
        movzx  eax,dl
        and    al,12
        shl    al,4
        mov    dl,byte[0xfb03]   ; y low bits
        add    al,dl
        mov    [0xfb22],byte al
        movzx  ebx,word[0xfb0c]

        mov    al,byte [0xfb01]  ; + or - ?
        test   al,8
        je     y_add

      y_sub:                      ; y-
        sub    bx,255
        sub    bx,255
      y_add:                      ; y+
        movzx  eax,byte [0xfb22]
        add    bx,ax
        add    bx,ax
        push   ebx
        mov    [0xfb00],byte 0
        jmp    mdraw

        ; end of com1 mouse


        ; ******************************************
        ; ********  PS2 MOUSE DRIVER  **************
        ; ******************************************

      ps2mousefirst:
      
        movzx  edx,byte [0x2E0000+4096*12+0x10]   ; first ps2 ?
        cmp    edx,40
        jne    @f
        mov    [0xfb00],byte 0  ; zero mouse block count
      @@:

        movzx  ebx,byte [0xfb00]
        add    ebx,1
        mov    [0xfb00],bl
        mov    eax,0xfb00
        add    eax,ebx
        mov    dl,byte [0x2E0000+4096*12+0x10]
        mov    [eax],byte dl

        cmp    bl,3             ; full packet of three bytes ?
        jnz    decm
;        jz     ps2mouse
;        jmp    decm


;      ps2mouse:

        mov    [0xfb00],byte 0  ; zero mouse block count

        ; buttons

        movzx  eax,byte [0xfb01]
        and    eax,3
        mov    [0xfb40],al

        ; x

        movzx  eax,word [0xfb0a]
        movzx  edx,byte [0xfb02]
        cmp    edx,128
        jb     ps2xp
        shl    edx,1
        add    eax,edx
        cmp    eax,512
        jge    ps2xsok
        xor    eax, eax
        jmp    ps2xready
       ps2xsok:
        sub    eax,512
        jmp    ps2xready
       ps2xp:
        shl    edx,1
        add    eax,edx
        jmp    ps2xready
       ps2xready:
        push   eax

        ; y

        movzx  eax,word [0xfb0c]
        movzx  edx,byte [0xfb03]
        cmp    edx,128
        jb     ps2yp
        add    eax,512
        shl    edx,1
        sub    eax,edx
        jmp    ps2yready
       ps2yp:
        shl    edx,1
        cmp    edx,eax
        jb     ps201
        mov    edx,eax
       ps201:
        sub    eax,edx
        jmp    ps2yready
       ps2yready:
        push   eax

        ;jmp    mdraw

        ; end of ps2 mouse


        ; ****************************
        ; ***** CHECK FOR LIMITS *****
        ; ****************************

      mdraw:

        cmp    [0xfb44],byte 0
        jne    mousedraw4
        cmp    [0xfb40],byte 0
        je     mousedraw4
        mov    [0xfff5],byte 1

      mousedraw4:

        pop    ebx
        pop    eax

        mov    [mouse_active],1

;        mov    dx,0                   ; smaller than zero
        xor    dx,dx
        cmp    bx,dx
        jge    mnb11
;        mov    bx,0
        xor    bx,bx
      mnb11:
        mov    [0xfb0c],word bx

;        mov    dx,0
        xor    dx,dx
        cmp    ax,dx
        jge    mnb22
;        mov    ax,0
        xor    ax,ax
      mnb22:
        mov    [0xfb0a],word ax

        mov    edx,[0xfe04]           ; bigger than maximum
        cmp    ebx,edx
        jb     mnb1
        mov    bx,[0xfe04]
      mnb1:
        mov    [0xfb0c],word bx

        mov    edx,[0xfe00]
        cmp    eax,edx
        jb     mnb2
        mov    ax,[0xfe00]
      mnb2:
        mov    [0xfb0a],word ax


        ; ****   NEXT DATA BYTE FROM MOUSE BUFFER   ****

      decm:

        mov    edi,[mousecount]         ; decrease counter
        dec    dword [edi]

        mov    esi,[mousedata]
        mov    edi,esi
        inc    esi
;        mov    ecx,250
        mov    ecx,[mousecount]
        mov    ecx,[ecx]
        cld
        rep    movsb

        jmp    uusicheckmouse

      checkmouseret:

        cmp    [0xfb44],byte 0
        jne    cmret
        
        cmp    [0xfb40],byte 0
        je     cmret
        
        mov    [0xfff4],byte 0
        mov    [0xfff5],byte 0
        
      cmret:

        popad

        ret


draw_mouse_under:

        ; return old picture
;        cli

        pushad

        xor    ecx,ecx
        xor    edx,edx

        ;cli    ; !!!****
        align  4
      mres:

        movzx  eax,word [0xfb4a]
        movzx  ebx,word [0xfb4c]

        add    eax,ecx
        add    ebx,edx

        push   ecx
        push   edx
        push   eax
        push   ebx

        mov    eax,edx
        shl    eax,6
        shl    ecx,2
        add    eax,ecx
        add    eax,mouseunder
        mov    ecx,[eax]

        pop    ebx
        pop    eax

        ;;;push   edi
        mov    edi,1 ;force
        call   [putpixel]
        ;;;pop    edi

        pop    edx
        pop    ecx

        inc    ecx
        cmp    ecx, 16
        jnz    mres
        xor    ecx, ecx
        inc    edx
        cmp    edx, 24
        jnz    mres
        ;sti    ; !!!****

        popad
        
;        sti
        
        ret


save_draw_mouse:

        ; save & draw
;        cli

        mov    [0xfb4a],ax
        mov    [0xfb4c],bx
        push   eax
        push   ebx
        mov    ecx,0
        mov    edx,0

        ;cli ; !!!****

      drm:

        push   eax
        push   ebx
        push   ecx
        push   edx

        ; helloworld
        push   eax ebx ecx
        add    eax,ecx  ; save picture under mouse
        add    ebx,edx
        push   ecx
        call   getpixel
        mov    [0xfb30],ecx
        pop    ecx
        mov    eax,edx
        shl    eax,6
        shl    ecx,2
        add    eax,ecx
        add    eax,mouseunder
        mov    ebx,[0xfb30]
        mov    [eax],ebx
        pop    ecx ebx eax

        mov    edi,edx       ; y cycle
        shl    edi,4       ; *16 bytes per row
        add    edi,ecx       ; x cycle
        mov    esi, edi
        add    edi, esi
        add    edi, esi       ; *3
        add    edi,[0xf200]      ; we have our str address
        mov    esi, edi
        add    esi, 16*24*3
        push   ecx
        mov    ecx, [0xfb30]
        call   combine_colors
        mov    [0xfb10], ecx
        pop    ecx


        pop    edx
        pop    ecx
        pop    ebx
        pop    eax

        add    eax,ecx       ; we have x coord+cycle
        add    ebx,edx       ; and y coord+cycle

        push   ecx edi
        mov    ecx, [0xfb10]
        mov    edi, 1
        call   [putpixel]
        pop    edi ecx

      mnext:

        mov    ebx,[esp+0]      ; pure y coord again
        mov    eax,[esp+4]      ; and x

        inc    ecx          ; +1 cycle
        cmp    ecx,16       ; if more than 16
        jnz    drm
        xor    ecx, ecx
        inc    edx
        cmp    edx,24
        jnz    drm

        pop    ebx
        pop    eax

;        sti ; !!!****

        ret


combine_colors:

      ; in
      ; ecx - color ( 00 RR GG BB )
      ; edi - ref to new color byte
      ; esi - ref to alpha byte
      ;
      ; out
      ; ecx - new color ( roughly (ecx*[esi]>>8)+([edi]*[esi]>>8) )

      push eax
      push ebx
      push edx
      push ecx
      xor ecx, ecx
         ; byte 2
      mov eax, 0xff
      sub al, [esi+0]
      mov ebx, [esp]
      shr ebx, 16
      and ebx, 0xff
      mul ebx
      shr eax, 8
      add ecx, eax
;      xor eax, eax
;      xor ebx, ebx
;      mov al, [edi+0]
;      mov bl, [esi+0]
    movzx eax, byte [edi+0]
    movzx ebx, byte [esi+0]
      mul ebx
      shr eax, 8
      add ecx, eax
      shl ecx, 8
         ; byte 1
      mov eax, 0xff
      sub al, [esi+1]
      mov ebx, [esp]
      shr ebx, 8
      and ebx, 0xff
      mul ebx
      shr eax, 8
      add ecx, eax
;      xor eax, eax
;      xor ebx, ebx
;      mov al, [edi+1]
;      mov bl, [esi+1]
    movzx eax, byte [edi+1]
    movzx ebx, byte [esi+1]
      mul ebx
      shr eax, 8
      add ecx, eax
      shl ecx, 8
         ; byte 2
      mov eax, 0xff
      sub al, [esi+2]
      mov ebx, [esp]
      and ebx, 0xff
      mul ebx
      shr eax, 8
      add ecx, eax
;      xor eax, eax
;      xor ebx, ebx
;      mov al, [edi+2]
;      mov bl, [esi+2]
    movzx eax, byte [edi+2]
    movzx ebx, byte [esi+2]
      mul ebx
      shr eax, 8
      add ecx, eax

      pop eax
      pop edx
      pop ebx
      pop eax
      ret


__sys_disable_mouse:

      pushad

      cmp  [0x3000],dword 1
      je   disable_m

      mov  edx,[0x3000]
      shl  edx,5
      add  edx,window_data

      movzx  eax, word [0xfb0a]
      movzx  ebx, word [0xfb0c]

      mov  ecx,[0xfe00]
      inc  ecx
      imul  ecx,ebx
      add  ecx,eax
      add  ecx, display_data

      movzx eax, byte [edx+twdw+0xe]

      movzx ebx, byte [ecx]
      cmp   eax,ebx
      je    yes_mouse_disable
      movzx ebx, byte [ecx+16]
      cmp   eax,ebx
      je    yes_mouse_disable

      mov   ebx,[0xfe00]
      inc   ebx
      imul  ebx,10
      add   ecx,ebx

      movzx ebx, byte [ecx]
      cmp   eax,ebx
      je    yes_mouse_disable

      mov   ebx,[0xfe00]
      inc   ebx
      imul  ebx,10
      add   ecx,ebx

      movzx ebx, byte [ecx]
      cmp   eax,ebx
      je    yes_mouse_disable
      movzx ebx, byte [ecx+16]
      cmp   eax,ebx
      je    yes_mouse_disable

      jmp   no_mouse_disable

    yes_mouse_disable:

      mov  edx,[0x3000]
      shl  edx,5
      add  edx,window_data

      movzx  eax, word [0xfb0a]
      movzx  ebx, word [0xfb0c]

      mov  ecx,[edx+0]   ; mouse inside the area ?
      add  eax,14
      cmp  eax,ecx
      jb   no_mouse_disable
      sub  eax,14

      add  ecx,[edx+8]
      cmp  eax,ecx
      jg   no_mouse_disable

      mov  ecx,[edx+4]
      add  ebx,20
      cmp  ebx,ecx
      jb   no_mouse_disable
      sub  ebx,20

      add  ecx,[edx+12]
      cmp  ebx,ecx
      jg   no_mouse_disable

    disable_m:

      cmp  dword [0xf204],dword 0
      jne  @f
      call draw_mouse_under
    @@:

      mov  [0xf204],dword 1

    no_mouse_disable:

      popad

      ret



__sys_draw_pointer:
        cli

        pushad

        cmp    dword [0xf204],dword 0  ; mouse visible ?
        je     chms00

        dec    dword [0xf204]

        cmp    [0xf204],dword 0
        jnz    nodmu2

        movzx  ebx,word [0xfb0c]
        movzx  eax,word [0xfb0a]
        call   save_draw_mouse

        popad
        sti
        ret

      nodmu2:

        popad
        sti
        ret

      chms00:

;        popad

;        pushad

;        cmp   [0xf204],dword 0
;        jne   nodmp

        movzx  ecx,word [0xfb4a]
        movzx  edx,word [0xfb4c]

        movzx  ebx,word [0xfb0c]
        movzx  eax,word [0xfb0a]

        cmp    eax,ecx
        jne    redrawmouse

        cmp    ebx,edx
        jne    redrawmouse

        jmp    nodmp

      redrawmouse:

        
        call   draw_mouse_under
redrawmouse_1:
        call   save_draw_mouse

     nodmp:

        popad
        sti
        ret



calculatebackground:   ; background

        ; all black

        mov   [display_data-8],dword 4      ; size x
        mov   [display_data-4],dword 2      ; size y

        mov   edi, 0x300000                 ; set background to black
        xor   eax, eax
        mov   ecx, 0x0fff00 / 4
        cld
        rep   stosd

        mov   edi,display_data              ; set os to use all pixels
        mov   eax,0x01010101
        mov   ecx,0x1fff00 / 4
        rep   stosd

        mov   byte [0xFFF0], 0              ; do not draw background!

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

        mov  edi,[0x3010]
        mov  [edi],eax

        pop  edi
        ret



delay_hs:     ; delay in 1/100 secs
        push  eax
        push  ecx
        push  edx

        mov   edx,[timer_ticks];[0xfdf0]
        add   edx,eax

      newtic:
        mov   ecx,[timer_ticks];[0xfdf0]
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


align 4

read_floppy_file:

; as input
;
; eax pointer to file
; ebx file lenght
; ecx start 512 byte block number
; edx number of blocks to read
; esi pointer to return/work area (atleast 20 000 bytes)
;
;
; on return
;
; eax = 0 command succesful
;       1 no fd base and/or partition defined
;       2 yet unsupported FS
;       3 unknown FS
;       4 partition not defined at hd
;       5 file not found
; ebx = size of file

     mov   edi,[0x3010]
     add   edi,0x10
     add   esi,[edi]
     add   eax,[edi]

     pushad
     mov  edi,esi
     add  edi,1024
     mov  esi,0x100000+19*512
     sub  ecx,1
     shl  ecx,9
     add  esi,ecx
     shl  edx,9
     mov  ecx,edx
     cld
     rep  movsb
     popad

     mov   [esp+36],eax
     mov   [esp+24],ebx
     ret



align 4

sys_programirq:

    mov   edi,[0x3010]
    add   edi,0x10
    add   eax,[edi]

    mov   edx,ebx
    shl   edx,2
    add   edx,irq_owner
    mov   edx,[edx]
    mov   edi,[0x3010]
    mov   edi,[edi+0x4]
    cmp   edx,edi
    je    spril1
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

     mov   edx,eax           ; check for correct owner
     shl   edx,2
     add   edx,irq_owner
     mov   edx,[edx]
     mov   edi,[0x3010]
     mov   edi,[edi+0x4]
     cmp   edx,edi
     je    gidril1
     mov   [esp+36],eax
     mov   [esp+32],dword 2
     mov   [esp+24],ebx
     ret

  gidril1:

     mov   ebx,eax
     shl   ebx,12
     add   ebx,0x2e0000
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
     xor   ecx,ecx
   gid1:
     mov   [esp+36],eax
     mov   [esp+32],ecx
     mov   [esp+24],ebx
     ret


set_io_access_rights:

     pushad

     mov   edi,[0x3000]
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
     jg    rpal1
     mov   esi,[0x2d0000]
     cmp   esi,0              ; no reserved areas ?
     je    rpal2
     cmp   esi,255            ; max reserved
     jge   rpal1
   rpal3:
     mov   edi,esi
     shl   edi,4
     add   edi,0x2d0000
     cmp   ebx,[edi+8]
     jg    rpal4
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

     mov   ebp,0                  ; enable - eax = port
     call  set_io_access_rights

     popad

     inc   eax
     cmp   eax,ecx
     jbe   new_port_access

   no_unmask_io:

     popad                         ; end enable io map
     sti

     mov   edi,[0x2d0000]
     add   edi,1
     mov   [0x2d0000],edi
     shl   edi,4
     add   edi,0x2d0000
     mov   esi,[0x3010]
     mov   esi,[esi+0x4]
     mov   [edi],esi
     mov   [edi+4],ebx
     mov   [edi+8],ecx

     xor   eax, eax
     ret




free_port_area:

     pushad

     mov   esi,[0x2d0000]     ; no reserved areas ?
     cmp   esi,0
     je    frpal2
     mov   edx,[0x3010]
     mov   edx,[edx+4]
   frpal3:
     mov   edi,esi
     shl   edi,4
     add   edi,0x2d0000
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

     dec   dword [0x2d0000]

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

     cmp   eax,0
     jz    reserve_irq

     mov   edi,ebx
     shl   edi,2
     add   edi,irq_owner
     mov   edx,[edi]
     mov   eax,[0x3010]
     mov   eax,[eax+0x4]
     mov   ecx,1
     cmp   edx,eax
     jne   fril1
     mov   [edi],dword 0
     mov   ecx,0
   fril1:
     mov   [esp+36],ecx ; return in eax
     ret

  reserve_irq:

     mov   edi,ebx
     shl   edi,2
     add   edi,irq_owner
     mov   edx,[edi]
     mov   ecx,1
     cmp   edx,0
     jne   ril1

     mov   edx,[0x3010]
     mov   edx,[edx+0x4]
     mov   [edi],edx
     mov   ecx,0

   ril1:

     mov   [esp+36],ecx ; return in eax

     ret



drawbackground:

       cmp   [0xfe0c],word 0x12
       jne   dbrv12
       cmp   [display_data-12],dword 1
       jne   bgrstr12
       call  vga_drawbackground_tiled
       ret
     bgrstr12:
       call  vga_drawbackground_stretch
       ret
     dbrv12:

       cmp  [0xfe0c],word 0100000000000000b
       jge  dbrv20
       cmp  [0xfe0c],word 0x13
       je   dbrv20
       call  vesa12_drawbackground
       ret
     dbrv20:
       cmp   [display_data-12],dword 1
       jne   bgrstr
       call  vesa20_drawbackground_tiled
       ret
     bgrstr:
       call  vesa20_drawbackground_stretch
       ret


sys_putimage:

     cmp   [0xfe0c],word 0x12
     jne   spiv20
     call  vga_putimage
     ret
   spiv20:

     cmp   [0xfe0c],word 0100000000000000b
     jge   piv20
     cmp   [0xfe0c],word 0x13
     je    piv20
     call  vesa12_putimage
     ret
   piv20:
     call  vesa20_putimage
     ret



; eax x beginning
; ebx y beginning
; ecx x end
; edx y end
; edi color

__sys_drawbar:

     cmp   [0xfe0c],word 0x12
     jne   sdbv20
     call  vga_drawbar
     ret
   sdbv20:

    cmp  [0xfe0c],word 0100000000000000b
    jge  dbv20
    cmp  [0xfe0c],word 0x13
    je   dbv20
    call vesa12_drawbar
    ret

  dbv20:

    call vesa20_drawbar
    ret



kb_read:

        push    ecx edx

        mov     ecx,0xffff
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
        mov     ecx,0xffff
      kw_loop1:
        in      al,0x64
        test    al,0x20
        jz      kw_ok1
        loop    kw_loop1
        mov     ah,1
        jmp     kw_exit
      kw_ok1:
        in      al,0x60
        mov     ecx,0xffff
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
        mov     ecx,0xffff
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
        mov     ecx,0xffff
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

        mov     ecx,0xffff
      c_wait:
        in      al,0x64
        test    al,2
        jz      c_send
        loop    c_wait
        jmp     c_error
      c_send:
        mov     al,bl
        out     0x64,al
        mov     ecx,0xffff
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

     mov     [0xf200],dword mousepointer

     cli
     mov     bl,0xa8                 ; enable mouse cmd
     call    kb_cmd
     call    kb_read                 ; read status

     mov     bl,0x20                 ; get command byte
     call    kb_cmd
     call    kb_read
     or      al,3                    ; enable interrupt
     mov     bl,0x60                 ; write command
     push    eax
     call    kb_cmd
     pop     eax
     call    kb_write

     mov     bl,0xd4                 ; for mouse
     call    kb_cmd
     mov     al,0xf4                 ; enable mouse device
     call    kb_write
     call    kb_read           ; read status return

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

     mov   edx,[cpuid_1+3*4]
     test  edx,00010000b
     jz    ret_rdtsc
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
  msg_board_data: times 512 db 0
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
     and  ecx, 511
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



sys_trace:

     test eax, eax                  ; get event data
     jnz  no_get_sys_events

     mov  esi,save_syscall_data     ; data
     mov  edi,[0x3010]
     mov  edi,[edi+0x10]
     add  edi,ebx
     cld
     rep  movsb

     mov  [esp+24],dword 0
     mov  eax,[save_syscall_count]  ; count
     mov  [esp+36],eax
     ret

   no_get_sys_events:

     ret


sys_process_def:

     cmp   eax,1                   ; set keyboard mode
     jne   no_set_keyboard_setup

     mov   edi,[0x3000]
     shl   edi,8
     add   edi,0x80000+0xB4
     mov   [edi],bl

     ret

   no_set_keyboard_setup:

     cmp   eax,2                   ; get keyboard mode
     jne   no_get_keyboard_setup

     mov   edi,[0x3000]
     shl   edi,8
     add   edi,0x80000+0xB4
     movzx eax, byte [edi]

     mov   [esp+36],eax

     ret

   no_get_keyboard_setup:

     cmp   eax,3                   ; get keyboard ctrl, alt, shift
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


     ret


sys_ipc:
     cmp  eax,1                      ; DEFINE IPC MEMORY
     jne  no_ipc_def
     mov  edi,[0x3000]
     shl  edi,8
     add  edi,0x80000
     mov  [edi+0xA0],ebx
     mov  [edi+0xA4],ecx
     mov  [esp+36],dword 0
     ret
   no_ipc_def:

     cmp  eax,2                      ; SEND IPC MESSAGE
     jne  no_ipc_send
     mov  esi,1
     mov  edi,0x3020
    ipcs1:
     cmp  [edi+4],ebx
     je   ipcs2
     add  edi,0x20
     inc  esi
     cmp  esi,[0x3004]
     jbe  ipcs1
     mov  [esp+36],dword 4
     ret
    ipcs2:

     cli

     push esi
     mov  eax,esi
     shl  eax,8
     mov  ebx,[eax+0x80000+0xa0]
     test ebx,ebx                  ; ipc area not defined ?
     je   ipc_err1

     add  ebx,[eax+0x80000+0xa4]
     mov  eax,esi
     shl  eax,5
     add  ebx,[eax+0x3000+0x10]    ; ebx <- max data position

     mov  eax,esi                  ; to
     shl  esi,8
     add  esi,0x80000
     mov  edi,[esi+0xa0]
     shl  eax,5
     add  eax,0x3000
     add  edi,[eax+0x10]

     cmp  [edi],byte 0             ; overrun ?
     jne  ipc_err2

     mov  ebp,edi
     add  edi,[edi+4]
     add  edi,8

     mov  esi,ecx                  ; from
     mov  eax,[0x3010]
     mov  eax,[eax+0x10]
     add  esi,eax

     mov  ecx,edx                  ; size

     mov  eax,edi
     add  eax,ecx
     cmp  eax,ebx
     jge  ipc_err3                 ; not enough room ?

     push ecx

     mov  eax,[0x3010]
     mov  eax,[eax+4]
     mov  [edi-8],eax
     mov  [edi-4],ecx
     cld
     rep  movsb

     pop  ecx
     add  ecx,8

     mov  edi,ebp                  ; increase memory position
     add  dword [edi+4],ecx

     mov  edi,[esp]
     shl  edi,8
     or   dword [edi+0x80000+0xA8],dword 01000000b ; ipc message

     cmp  [check_idle_semaphore],dword 20
     jge  ipc_no_cis
     mov  [check_idle_semaphore],5
   ipc_no_cis:

     xor  eax, eax

    ipc_err:
     add  esp,4
     mov  [esp+36],eax
     sti
     ret

    ipc_err1:
     add  esp,4
     mov  [esp+36],dword 1
     sti
     ret
    ipc_err2:
     add  esp,4
     mov  [esp+36],dword 2
     sti
     ret
    ipc_err3:
     add  esp,4
     mov  [esp+36],dword 3
     sti
     ret

   no_ipc_send:

     mov  [esp+36],dword -1
     ret


align 4

sys_gs:                         ; direct screen access

     cmp  eax,1                 ; resolution
     jne  no_gs1
     mov  eax,[0xfe00]
     shl  eax,16
     mov  ax,[0xfe04]
     add  eax,0x00010001
     mov  [esp+36],eax
     ret
   no_gs1:

     cmp   eax,2                ; bits per pixel
     jne   no_gs2
     movzx eax,byte [0xfbf1]
     mov   [esp+36],eax
     ret
   no_gs2:

     cmp   eax,3                ; bytes per scanline
     jne   no_gs3
     mov   eax,[0xfe08]
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


     mov   edx,[0x3010]
     add   eax,[edx-twdw]
     add   ebx,[edx-twdw+4]
     xor   edi,edi ; no force
     call  [disable_mouse]
     jmp   [putpixel]

align 4

syscall_writetext:                      ; WriteText

     mov   edi,[0x3010]
     mov   ebp,[edi-twdw]
     shl   ebp,16
     add   ebp,[edi-twdw+4]
     add   edi,0x10
     add   ecx,[edi]
     add   eax,ebp
     xor   edi,edi
     jmp   dtext

align 4

syscall_openramdiskfile:                ; OpenRamdiskFile


     mov   edi,[0x3010]
     add   edi,0x10
     add   eax,[edi]
     add   edx,[edi]
     mov   esi,12
     call  fileread
     mov   [esp+36],ebx
     ret

align 4

syscall_putimage:                       ; PutImage

     mov   edi,[0x3010]
     add   edi,0x10
     add   eax,[edi]
     mov   edx,ecx
     mov   ecx,ebx
     mov   ebx,eax
     call  sys_putimage
     mov   [esp+36],eax
     ret

align 4

syscall_drawrect:                       ; DrawRect

     mov   edi,ecx
     test  ax,ax
     je    drectr
     test  bx,bx
     je    drectr
     movzx ecx,ax
     shr   eax,16
     movzx edx,bx
     shr   ebx,16
     add   ecx,eax
     add   edx,ebx
     jmp   [drawbar]
    drectr:
     ret

align 4

syscall_getscreensize:                  ; GetScreenSize

     movzx eax,word[0xfe00]
     shl   eax,16
     mov   ax,[0xfe04]
     mov   [esp+36],eax
     ret

align 4

syscall_system:                         ; System

     call  sys_system
     mov   [esp+36],eax
     ret

align 4

syscall_startapp:                       ; StartApp
     mov   edi,[0x3010]
     add   edi,0x10
     add   eax,[edi]
     test  ebx,ebx
     jz    noapppar
     add   ebx,[edi]
   noapppar:
;     call  start_application_fl
     call   new_start_application_fl
     mov   [esp+36],eax
     ret
     

align 4

syscall_cdaudio:                        ; CD

     call  sys_cd_audio
     mov   [esp+36],eax
     ret

align 4

syscall_readhd:                         ; ReadHd

     mov   edi,[0x3010]
     add   edi,0x10
     add   esi,[edi]
     add   eax,[edi]
     call  read_hd_file
     mov   [esp+36],eax
     mov   [esp+24],ebx
     ret

align 4

syscall_starthdapp:                     ; StartHdApp

     mov   edi,[0x3010]
     add   edi,0x10
     add   eax,[edi]
     add   ecx,[edi]
     mov   ebp,0
     call  start_application_hd
     mov   [esp+36],eax
     ret

align 4

syscall_delramdiskfile:                 ; DelRamdiskFile

     mov   edi,[0x3010]
     add   edi,0x10
     add   eax,[edi]
     call  filedelete
     mov   [esp+36],eax
     ret

align 4

syscall_writeramdiskfile:               ; WriteRamdiskFile

     mov   edi,[0x3010]
     add   edi,0x10
     add   eax,[edi]
     add   ebx,[edi]
     call  filesave
     mov   [esp+36],eax
     ret

align 4

syscall_getpixel:                       ; GetPixel

     mov   ecx,[0xfe00]
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

     mov   edi,[0x3010]
     add   edi,0x10
     add   eax,[edi]
     call  read_string
     mov   [esp+36],eax
     ret

align 4

syscall_drawline:                       ; DrawLine

     mov   edi,[0x3010]
     movzx edx,word[edi-twdw]
     mov   ebp,edx
     shl   edx,16
     add   ebp,edx
     movzx edx,word[edi-twdw+4]
     add   eax,ebp
     mov   ebp,edx
     shl   edx,16
     xor   edi,edi
     add   edx,ebp
     add   ebx,edx
     jmp   [draw_line]

align 4

syscall_getirqowner:                    ; GetIrqOwner

     shl   eax,2
     add   eax,irq_owner
     mov   eax,[eax]
     mov   [esp+36],eax
     ret

align 4

syscall_reserveportarea:                ; ReservePortArea and FreePortArea

     call  r_f_port_area
     mov   [esp+36],eax
     ret

align 4

syscall_appints:                        ; AppInts

     test  eax,eax
     jnz   unknown_app_int_fn
     mov   edi,[0x3010]
     mov   [edi+draw_data-0x3000+0x1c],ebx
     ret
   unknown_app_int_fn:
     mov   [esp+36],dword -1
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

     mov   edi,[0x3010]
     add   edi,0x10
     add   eax,[edi]
     add   ecx,[edi]
     add   edx,[edi]
     call  file_read

     mov   [esp+36],eax
     mov   [esp+24],ebx

     ret


align 4

write_to_hd:                            ; Write a file to hd

     mov   edi,[0x3010]
     add   edi,0x10
     add   eax,[edi]
     add   ecx,[edi]
     add   edx,[edi]
     call  file_write
     ret

align 4

delete_from_hd:                         ; Delete a file from hd

     mov   edi,[0x3010]
     add   edi,0x10
     add   eax,[edi]
     add   ecx,[edi]
     call  file_delete
     ret


align 4

undefined_syscall:                      ; Undefined system call

     mov   [esp+36],dword -1
     ret


;clear_busy_flag_at_caller:

;      push  edi

;      mov   edi,[0x3000]    ; restore processes tss pointer in gdt, busyfl?
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

;* start code - get  process (3) - Mario79
active_process      dd   0
;* end code - get active process (3) - Mario79

wraw_bacground_select db 0
  lba_read_enabled    dd   0x0  ; 0 = disabled , 1 = enabled
  pci_access_enabled  dd   0x0  ; 0 = disabled , 1 = enabled

  sb16       dd 0x0
  wss        dd 0x0

  buttontype         dd 0x0
  windowtypechanged  dd 0x0
endg

iglobal
  keyboard   dd 0x1
  sound_dma  dd 0x1
  syslang    dd 0x1
endg

IncludeIGlobals
endofcode:
IncludeUGlobals
uglobals_size = $ - endofcode
diff16 "end of kernel code",$

