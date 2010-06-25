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
;           mov eax, sysenter_stack ; Check it
           xor     eax, eax
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


; BUILD SCHEDULER

        call   build_scheduler ; sys32.inc

; LOAD IDT
        lidt   [cs:idtreg]
        cli

;!!!!!!!!!!!!!!!!!!!!!!!!!!
include 'detect/disks.inc'
;!!!!!!!!!!!!!!!!!!!!!!!!!!

; READ RAMDISK IMAGE FROM HD

;!!!!!!!!!!!!!!!!!!!!!!!
;include 'rdload.inc'
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
	push	eax
	mov		eax,char  - std_application_base_address
	call	file_system_lfn
	mov		eax,char2  - std_application_base_address
	call	file_system_lfn
	pop	eax
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

;        mov eax, fpu_data
;        mov  dword [SLOT_BASE+APPDATA.fpu_state], eax
;        mov  dword [SLOT_BASE+APPDATA.fpu_handler], 0
;        mov  dword [SLOT_BASE+APPDATA.sse_handler], 0

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


