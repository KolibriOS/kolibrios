;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2023. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


dpl0    =  10010000b      ; data read       dpl0
drw0    =  10010010b      ; data read/write dpl0
drw3    =  11110010b      ; data read/write dpl3
cpl0    =  10011010b      ; code read dpl0
cpl3    =  11111010b      ; code read dpl3

D32     =  01000000b      ; 32bit segment
G32     =  10000000b      ; page gran

;;;;;;;;;;; task manager errors ;;;;;;;;;;

TASKMAN_ERROR_OUT_OF_MEMORY      = 30 ; 0x1E
TASKMAN_ERROR_NOT_A_EXECUTABLE   = 31 ; 0x1F
TASKMAN_ERROR_TOO_MANY_PROCESSES = 32 ; 0x20

;;;;;;;;;;;;cpu_caps flags;;;;;;;;;;;;;;;;

CPU_386        = 3
CPU_486        = 4
CPU_PENTIUM    = 5
CPU_P6         = 6
CPU_PENTIUM4   = 0x0F

CAPS_FPU       =    00 ;on-chip x87 floating point unit
CAPS_VME       =    01 ;virtual-mode enhancements
CAPS_DE        =    02 ;debugging extensions
CAPS_PSE       =    03 ;page-size extensions
CAPS_TSC       =    04 ;time stamp counter
CAPS_MSR       =    05 ;model-specific registers
CAPS_PAE       =    06 ;physical-address extensions
CAPS_MCE       =    07 ;machine check exception
CAPS_CX8       =    08 ;CMPXCHG8B instruction
CAPS_APIC      =    09 ;on-chip advanced programmable
                       ;interrupt controller
;                   10 ;unused
CAPS_SEP       =    11 ;SYSENTER and SYSEXIT instructions
CAPS_MTRR      =    12 ;memory-type range registers
CAPS_PGE       =    13 ;page global extension
CAPS_MCA       =    14 ;machine check architecture
CAPS_CMOV      =    15 ;conditional move instructions
CAPS_PAT       =    16 ;page attribute table

CAPS_PSE36     =    17 ;page-size extensions
CAPS_PSN       =    18 ;processor serial number
CAPS_CLFLUSH   =    19 ;CLFUSH instruction

CAPS_DS        =    21 ;debug store
CAPS_ACPI      =    22 ;thermal monitor and software
                       ;controlled clock supported
CAPS_MMX       =    23 ;MMX instructions
CAPS_FXSR      =    24 ;FXSAVE and FXRSTOR instructions
CAPS_SSE       =    25 ;SSE instructions
CAPS_SSE2      =    26 ;SSE2 instructions
CAPS_SS        =    27 ;self-snoop
CAPS_HTT       =    28 ;hyper-threading technology
CAPS_TM        =    29 ;thermal monitor supported
CAPS_IA64      =    30 ;IA64 capabilities
CAPS_PBE       =    31 ;pending break enable

;ecx
CAPS_SSE3      =    32 ;SSE3 instructions
;                   33
;                   34
CAPS_MONITOR   =    35 ;MONITOR/MWAIT instructions
CAPS_DS_CPL    =    36 ;
CAPS_VMX       =    37 ;virtual mode extensions
;                   38 ;
CAPS_EST       =    39 ;enhansed speed step
CAPS_TM2       =    40 ;thermal monitor2 supported
;                   41
CAPS_CID       =    42 ;
;                   43
;                   44
CAPS_CX16      =    45 ;CMPXCHG16B instruction
CAPS_xTPR      =    46 ;
CAPS_XSAVE     =    32 + 26 ; XSAVE and XRSTOR instructions
CAPS_OSXSAVE   =    32 + 27
; A value of 1 indicates that the OS has set CR4.OSXSAVE[bit 18] to enable
; XSETBV/XGETBV instructions to access XCR0 and to support processor extended
; state management using XSAVE/XRSTOR.
CAPS_AVX       =    32 + 28 ; not AVX2
;
;reserved
;
;ext edx /ecx
CAPS_SYSCAL    =    64 ;
CAPS_XD        =    65 ;execution disable
CAPS_FFXSR     =    66 ;
CAPS_RDTSCP    =    67 ;
CAPS_X64       =    68 ;
CAPS_3DNOW     =    69 ;
CAPS_3DNOWEXT  =    70 ;
CAPS_LAHF      =    71 ;
CAPS_CMP_LEG   =    72 ;
CAPS_SVM       =    73 ;secure virual machine
CAPS_ALTMOVCR8 =    74 ;

; CPU MSR names
MSR_SYSENTER_CS         =     0x174
MSR_SYSENTER_ESP        =     0x175
MSR_SYSENTER_EIP        =     0x176
MSR_CR_PAT              =     0x277
MSR_MTRR_DEF_TYPE       =     0x2FF

MSR_AMD_EFER            =     0xC0000080      ; Extended Feature Enable Register
MSR_AMD_STAR            =     0xC0000081      ; SYSCALL/SYSRET Target Address Register

CR0_PE         =    0x00000001   ;protected mode
CR0_MP         =    0x00000002   ;monitor fpu
CR0_EM         =    0x00000004   ;fpu emulation
CR0_TS         =    0x00000008   ;task switch
CR0_ET         =    0x00000010   ;extension type hardcoded to 1
CR0_NE         =    0x00000020   ;numeric error
CR0_WP         =    0x00010000   ;write protect
CR0_AM         =    0x00040000   ;alignment check
CR0_NW         =    0x20000000   ;not write-through
CR0_CD         =    0x40000000   ;cache disable
CR0_PG         =    0x80000000   ;paging


CR4_VME        =    0x000001
CR4_PVI        =    0x000002
CR4_TSD        =    0x000004
CR4_DE         =    0x000008
CR4_PSE        =    0x000010
CR4_PAE        =    0x000020
CR4_MCE        =    0x000040
CR4_PGE        =    0x000080
CR4_PCE        =    0x000100
CR4_OSFXSR     =    0x000200
CR4_OSXMMEXPT  =    0x000400
CR4_OSXSAVE    =    0x040000

XCR0_FPU_MMX   =    0x0001
XCR0_SSE       =    0x0002
XCR0_AVX       =    0x0004
XCR0_MPX       =    0x0018
XCR0_AVX512    =    0x00e0

MXCSR_IE       =    0x0001
MXCSR_DE       =    0x0002
MXCSR_ZE       =    0x0004
MXCSR_OE       =    0x0008
MXCSR_UE       =    0x0010
MXCSR_PE       =    0x0020
MXCSR_DAZ      =    0x0040
MXCSR_IM       =    0x0080
MXCSR_DM       =    0x0100
MXCSR_ZM       =    0x0200
MXCSR_OM       =    0x0400
MXCSR_UM       =    0x0800
MXCSR_PM       =    0x1000
MXCSR_FZ       =    0x8000

MXCSR_INIT     = MXCSR_IM + MXCSR_DM + MXCSR_ZM + MXCSR_OM + MXCSR_UM + MXCSR_PM

EFLAGS_CF      =    0x000001  ; carry flag
EFLAGS_PF      =    0x000004  ; parity flag
EFLAGS_AF      =    0x000010  ; auxiliary flag
EFLAGS_ZF      =    0x000040  ; zero flag
EFLAGS_SF      =    0x000080  ; sign flag
EFLAGS_TF      =    0x000100  ; trap flag
EFLAGS_IF      =    0x000200  ; interrupt flag
EFLAGS_DF      =    0x000400  ; direction flag
EFLAGS_OF      =    0x000800  ; overflow flag
EFLAGS_IOPL    =    0x003000  ; i/o priviledge level
EFLAGS_NT      =    0x004000  ; nested task flag
EFLAGS_RF      =    0x010000  ; resume flag
EFLAGS_VM      =    0x020000  ; virtual 8086 mode flag
EFLAGS_AC      =    0x040000  ; alignment check flag
EFLAGS_VIF     =    0x080000  ; virtual interrupt flag
EFLAGS_VIP     =    0x100000  ; virtual interrupt pending
EFLAGS_ID      =    0x200000  ; id flag

IRQ_PIC        =    0
IRQ_APIC       =    1

struct  TSS
        _back       rw 2
        _esp0       rd 1
        _ss0        rw 2
        _esp1       rd 1
        _ss1        rw 2
        _esp2       rd 1
        _ss2        rw 2
        _cr3        rd 1
        _eip        rd 1
        _eflags     rd 1
        _eax        rd 1
        _ecx        rd 1
        _edx        rd 1
        _ebx        rd 1
        _esp        rd 1
        _ebp        rd 1
        _esi        rd 1
        _edi        rd 1
        _es         rw 2
        _cs         rw 2
        _ss         rw 2
        _ds         rw 2
        _fs         rw 2
        _gs         rw 2
        _ldt        rw 2
        _trap       rw 1
        _io         rw 1
                    rb 24
        _io_map_0   rb 4096
        _io_map_1   rb 4096
ends

DRIVE_DATA_SIZE     = 16

OS_BASE             = 0x80000000

window_data         = OS_BASE + 0x0001000
background_window   = window_data + sizeof.WDATA

;CDDataBuf           = OS_BASE + 0x0005000

BOOT_VARS           = 0x9000

idts                = OS_BASE + 0x000B100
WIN_STACK           = OS_BASE + 0x000C000
WIN_POS             = OS_BASE + 0x000C400
FDD_BUFF            = OS_BASE + 0x000D000     ;512

WIN_TEMP_XY         = OS_BASE + 0x000F300
KEY_COUNT           = OS_BASE + 0x000F400
KEY_BUFF            = OS_BASE + 0x000F401 ; 120*2 + 2*2 = 244 bytes, actually 255 bytes

BTN_COUNT           = OS_BASE + 0x000F500
BTN_BUFF            = OS_BASE + 0x000F501


BTN_ADDR            = OS_BASE + 0x000FE88
MEM_AMOUNT          = OS_BASE + 0x000FE8C

SYS_SHUTDOWN        = OS_BASE + 0x000FF00


TMP_STACK_TOP       = 0x007CC00

sys_proc            = OS_BASE + 0x007E000

SLOT_BASE           = OS_BASE + 0x0080000

VGABasePtr          = OS_BASE + 0x00A0000

virtual at            OS_BASE + 0x05FFF80
  tss  TSS
end virtual

HEAP_BASE           = OS_BASE + 0x0800000
HEAP_MIN_SIZE       = 0x01000000

page_tabs           = 0xFDC00000
app_page_tabs       = 0xFDC00000
kernel_tabs         = page_tabs + (OS_BASE shr 10)   ;0xFDE00000
master_tab          = page_tabs + (page_tabs shr 10) ;0xFDFF70000

LFB_BASE            = 0xFE000000

std_application_base_address   = 0
RING0_STACK_SIZE    = 0x2000

REG_SS              = RING0_STACK_SIZE -  4
REG_APP_ESP         = RING0_STACK_SIZE -  8
REG_EFLAGS          = RING0_STACK_SIZE - 12
REG_CS              = RING0_STACK_SIZE - 16
REG_EIP             = RING0_STACK_SIZE - 20
REG_EAX             = RING0_STACK_SIZE - 24
REG_ECX             = RING0_STACK_SIZE - 28
REG_EDX             = RING0_STACK_SIZE - 32
REG_EBX             = RING0_STACK_SIZE - 36
REG_ESP             = RING0_STACK_SIZE - 40  ;RING0_STACK_SIZE-20
REG_EBP             = RING0_STACK_SIZE - 44
REG_ESI             = RING0_STACK_SIZE - 48
REG_EDI             = RING0_STACK_SIZE - 52
REG_RET             = RING0_STACK_SIZE - 56  ;irq0.return


PAGE_SIZE           = 4096

PG_UNMAP            = 0x000
PG_READ             = 0x001
PG_WRITE            = 0x002
PG_USER             = 0x004
PG_PCD              = 0x008
PG_PWT              = 0x010
PG_ACCESSED         = 0x020
PG_DIRTY            = 0x040
PG_PAT              = 0x080
PG_GLOBAL           = 0x100
PG_SHARED           = 0x200

PG_SWR              = 0x003 ; PG_WRITE + PG_READ
PG_UR               = 0x005 ; PG_USER + PG_READ
PG_UWR              = 0x007 ; PG_USER + PG_WRITE + PG_READ
PG_NOCACHE          = 0x018 ; PG_PCD + PG_PWT

PDE_LARGE           = 0x080

MEM_WB              = 6     ; write-back memory
MEM_WC              = 1     ; write combined memory
MEM_UC              = 0     ; uncached memory

PAT_WB              = 0x000
PAT_WC              = 0x008
PAT_UCM             = 0x010
PAT_UC              = 0x018

PAT_TYPE_UC         = 0
PAT_TYPE_WC         = 1
PAT_TYPE_WB         = 6
PAT_TYPE_UCM        = 7

PAT_VALUE           = 0x00070106; (UC<<24)|(UCM<<16)|(WC<<8)|WB

MAX_MEMMAP_BLOCKS   = 32

EVENT_REDRAW       = 0x00000001
EVENT_KEY          = 0x00000002
EVENT_BUTTON       = 0x00000004
EVENT_BACKGROUND   = 0x00000010
EVENT_MOUSE        = 0x00000020
EVENT_IPC          = 0x00000040
EVENT_NETWORK      = 0x00000080
EVENT_DEBUG        = 0x00000100
EVENT_NETWORK2     = 0x00000200
EVENT_EXTENDED     = 0x00000400

EVM_MOUSE_FILTER  = 0x80000000  ; see in macros.inc for apps
EVM_CURSOR_FILTER = 0x40000000

EV_INTR            = 1

STDIN_FILENO       = 0
STDOUT_FILENO      = 1
STDERR_FILENO      = 2

SYSTEM_SHUTDOWN    = 2
SYSTEM_REBOOT      = 3
SYSTEM_RESTART     = 4

BLIT_CLIENT_RELATIVE = 0x20000000

struct SYSCALL_STACK
        eip     dd ?    ;  +0
        edi     dd ?    ;  +4
        esi     dd ?    ;  +8
        ebp     dd ?    ; +12
        esp     dd ?    ; +16
        ebx     dd ?    ; +20
        edx     dd ?    ; +24
        ecx     dd ?    ; +28
        eax     dd ?    ; +32
ends

struct  LHEAD
        next            dd ?   ;next object in list
        prev            dd ?   ;prev object in list
ends

struct  MUTEX_WAITER
        list    LHEAD
        task    dd ?
        type    dd ?
ends

struct  MUTEX
        wait_list       LHEAD
        count           dd ?
ends

struct  RWSEM
        wait_list       LHEAD
        count           dd ?
ends

struct  FUTEX
        list            LHEAD
        magic           dd ?
        handle          dd ?
        destroy         dd ?

        wait_list       LHEAD
        pointer         dd ?
        flags           dd ?
ends

FUTEX_INIT      = 0
FUTEX_DESTROY   = 1
FUTEX_WAIT      = 2
FUTEX_WAKE      = 3

struct  FILED
        list            LHEAD
        magic           rd 1
        handle          rd 1
        destroy         rd 1
        mode            rd 1
        file            rd 1
ends

struct  PIPE
        pipe_ops        rd 1
        buffer          rd 1
        readers         rd 1
        writers         rd 1

        pipe_lock       MUTEX
        count           rd 1

        read_end        rd 1
        write_end       rd 1
        rlist           LHEAD
        wlist           LHEAD
ends

struct  PROC
        list            LHEAD
        thr_list        LHEAD
        heap_lock       MUTEX
        heap_base       rd 1
        heap_top        rd 1
        mem_used        rd 1
        dlls_list_ptr   rd 1
        pdt_0_phys      rd 1
        pdt_1_phys      rd 1
        io_map_0        rd 1
        io_map_1        rd 1

        ht_lock         rd 1
        ht_free         rd 1                ;htab[0] stdin
        ht_next         rd 1                ;htab[1] stdout
        htab            rd 1024-PROC.htab/4 ;htab[2] stderr
        pdt_0           rd 1024
ends

struct  DBG_REGS
        dr0             dd ?
        dr1             dd ?
        dr2             dd ?
        dr3             dd ?
        dr7             dd ?
ends

struct  POINT
        x       dd ?
        y       dd ?
ends

struct  RECT
        left    dd ?
        top     dd ?
        right   dd ?
        bottom  dd ?
ends

struct  BOX
        left    dd ?
        top     dd ?
        width   dd ?
        height  dd ?
ends

struct  APPDATA
        app_name        rb 11
                        rb 5

        list            LHEAD           ;+16
        process         dd ?            ;+24
        fpu_state       dd ?            ;+28
        exc_handler     dd ?            ;+32
        except_mask     dd ?            ;+36
        pl0_stack       dd ?            ;+40
                        dd ?            ;+44
        fd_ev           dd ?            ;+48
        bk_ev           dd ?            ;+52
        fd_obj          dd ?            ;+56
        bk_obj          dd ?            ;+60
        saved_esp       dd ?            ;+64
        io_map          rd 2            ;+68
        dbg_state       dd ?            ;+76
        cur_dir         dd ?            ;+80   ; directory this thread
        wait_timeout    dd ?            ;+84
        saved_esp0      dd ?            ;+88
        wait_begin      dd ?            ;+92   +++
        wait_test       dd ?            ;+96   +++
        wait_param      dd ?            ;+100  +++
        tls_base        dd ?            ;+104
        event_mask      dd ?            ;+108   stores event types allowed for task
        tid             dd ?            ;+112   thread id
                        dd ?            ;+116
                        dd ?            ;+120
        state           db ?            ;+124   thread state
        wnd_number      db ?            ;+125
                        dw ?            ;+126
        window          dd ?            ;+128   ptr WDATA
                        dd ?            ;+132
                        dd ?            ;+136
        counter_sum     dd ?            ;+140
                        rd 4            ;+144
        ipc_start       dd ?            ;+160
        ipc_size        dd ?            ;+164
        occurred_events dd ?            ;+168  ; mask which accumulates occurred events
        debugger_slot   dd ?            ;+172  ; index in SLOT_BASE array
        terminate_protection dd ?       ;+176
        keyboard_mode   db ?            ;+180
                        rb 3
        exec_params     dd ?            ;+184
        dbg_event_mem   dd ?            ;+188
        dbg_regs        DBG_REGS        ;+192
                        dd ?            ;+212
                        rd 4            ;+216
        priority        dd ?            ;+232
        in_schedule     LHEAD           ;+236
        counter_add     dd ?            ;+244
        cpu_usage       dd ?            ;+248
                        dd ?            ;+252  ; temporary place to save cursor
ends

assert sizeof.APPDATA = 256

APP_OBJ_OFFSET  = 48
APP_EV_OFFSET   = 40

; Thread states:
TSTATE_RUNNING        = 0
TSTATE_RUN_SUSPENDED  = 1
TSTATE_WAIT_SUSPENDED = 2
TSTATE_ZOMBIE         = 3
TSTATE_TERMINATING    = 4
TSTATE_WAITING        = 5
TSTATE_FREE           = 9

; Window constants:
WSTATE_NORMAL    = 00000000b
WSTATE_MAXIMIZED = 00000001b
WSTATE_MINIMIZED = 00000010b
WSTATE_ROLLEDUP  = 00000100b
WSTATE_USED      = 10000000b

; fl_redraw
WSTATE_REDRAW    = 00000001b
WSTATE_WNDDRAWN  = 00000010b

WSTYLE_HASCAPTION     = 00010000b
WSTYLE_CLIENTRELATIVE = 00100000b

ZPOS_DESKTOP            = -2
ZPOS_ALWAYS_BACK        = -1
ZPOS_NORMAL             = 0
ZPOS_ALWAYS_TOP         = 1     ;ZPOS_ALWAYS_TOP is always last and has max number!

; Window structure:
struct  WDATA
        box             BOX
        cl_workarea     dd ?
        cl_titlebar     dd ?
        cl_frames       dd ?
        z_modif         db ?
        fl_wstate       db ?
        fl_wdrawn       db ?
        fl_redraw       db ?
        clientbox       BOX
        shape           dd ?
        shape_scale     dd ?
        caption         dd ?
        captionEncoding db ?
                        rb 3
        saved_box       BOX
        cursor          dd ?
        temp_cursor     dd ?
        draw_bgr_x      dd ?
        draw_bgr_y      dd ?
        draw_data       RECT
        thread          dd ? ; prt APPDATA
                        rb 12
ends

label WDATA.fl_wstyle byte at WDATA.cl_workarea + 3

assert sizeof.WDATA = 128

struct  SYS_VARS
        bpp             dd ?
        scanline        dd ?
        vesa_mode       dd ?
        x_res           dd ?
        y_res           dd ?
ends

struct  APPOBJ                  ; common object header
        magic           dd ?    ;
        destroy         dd ?    ; internal destructor
        fd              dd ?    ; next object in list
        bk              dd ?    ; prev object in list
        pid             dd ?    ; owner id
ends

struct  CURSOR          APPOBJ
        base            dd ?   ;allocated memory
        hot_x           dd ?   ;hotspot coords
        hot_y           dd ?

        list_next       dd ?   ;next cursor in cursor list
        list_prev       dd ?   ;prev cursor in cursor list
        dev_obj         dd ?   ;device depended data
ends


struct  EVENT           APPOBJ
        id              dd ?   ;event uid
        state           dd ?   ;internal flags
        code            dd ?
                        rd 5
ends


struct  SMEM
        bk              dd ?
        fd              dd ?    ;+4
        base            dd ?    ;+8
        size            dd ?    ;+12
        access          dd ?    ;+16
        refcount        dd ?    ;+20
        name            rb 32   ;+24
ends

struct  SMAP            APPOBJ
        base            dd ?   ;mapped base
        parent          dd ?   ;SMEM
ends

struct  DLLDESCR
        bk              dd ?
        fd              dd ?    ;+4
        data            dd ?    ;+8
        size            dd ?    ;+12
        timestamp       dq ?
        refcount        dd ?
        defaultbase     dd ?
        coff_hdr        dd ?
        symbols_ptr     dd ?
        symbols_num     dd ?
        symbols_lim     dd ?
        exports         dd ?   ;export table
        name            rb 260
ends

struct  HDLL
        fd              dd ?   ;next object in list
        bk              dd ?   ;prev object in list
        pid             dd ?   ;owner id

        base            dd ?   ;mapped base
        size            dd ?   ;mapped size
        refcount        dd ?   ;reference counter for this process and this lib
        parent          dd ?   ;DLLDESCR
ends

struct DQ
        union
                lo    dd ?
                hi_be dd ?      ; big endian
        ends
        union
                hi    dd ?
                lo_be dd ?
        ends
ends

struct e820entry
        addr DQ ?
        size DQ ?
        type dd ?
ends

RD_LOAD_FROM_FLOPPY = 1
RD_LOAD_FROM_HD     = 2
RD_LOAD_FROM_MEMORY = 3
RD_LOAD_FROM_FORMAT = 4
RD_LOAD_FROM_NONE   = 5

struct boot_pci_data
        access_mechanism db ?
        last_bus         db ?
        version          dw ?   ; bcd minor, then major
        pm_entry         dd ?   ; physical address of protected-mode entry point
ends

struct  boot_data
        bpp             db ?    ; bits per pixel
        pitch           dw ?    ; scanline length
                        db ?
                        dd ?
        vesa_mode       dw ?
        x_res           dw ?
        y_res           dw ?
                        dw ?
                        dd ?
        bank_switch     dd ?    ; Vesa 1.2 pm bank switch
        lfb             dd ?    ; Vesa 2.0 LFB address
        mtrr            db ?    ; 0 or 1: enable MTRR graphics acceleration
        launcher_start  db ?    ; 0 or 1: start the first app (right now it's
                                ; LAUNCHER) after kernel is loaded
        debug_print     db ?    ; if nonzero, duplicates debug output to the screen
        dma             db ?    ; DMA write: 1=yes, 2=no
        pci_data        boot_pci_data
                        rb 8
        shutdown_type   db ?    ; see sysfn 18.9
                        rb 15
        apm_entry       dd ?    ; entry point of APM BIOS
        apm_version     dw ?    ; BCD
        apm_flags       dw ?
                        rb 8
        apm_code_32     dw ?
        apm_code_16     dw ?
        apm_data_16     dw ?
        rd_load_from    db ?    ; Device to load ramdisk from, RD_LOAD_FROM_*
                        db ?
        kernel_restart  dw ?
        sys_disk        dw ?    ; Device to mount on /sys/, see loader_doc.txt for details
        acpi_rsdp       dd ?
        syspath         rb 0x17
        devicesdat_data dd ?
        devicesdat_size dd ?
        bios_hd_cnt     db ?    ; number of BIOS hard disks
        bios_hd         rb 0x80 ; BIOS hard disks
        memmap_block_cnt dd ?   ; available physical memory map: number of blocks
        memmap_blocks   e820entry
                        rb sizeof.e820entry * (MAX_MEMMAP_BLOCKS - 1)
ends

virtual at BOOT_VARS
        BOOT_LO boot_data
end virtual
virtual at OS_BASE + BOOT_VARS
        BOOT boot_data
end virtual

MAX_SCREEN_WIDTH  = 3840
MAX_SCREEN_HEIGHT = 2160

struct  display_t
        x               dd ?
        y               dd ?
        width           dd ?
        height          dd ?
        bits_per_pixel  dd ?
        vrefresh        dd ?
        current_lfb     dd ?
        lfb_pitch       dd ?

        win_map_lock    RWSEM
        win_map         dd ?
        win_map_pitch   dd ?
        win_map_size    dd ?

        modes           dd ?
        ddev            dd ?
        connector       dd ?
        crtc            dd ?

        cr_list.next    dd ?
        cr_list.prev    dd ?

        cursor          dd ?

        init_cursor     dd ?
        select_cursor   dd ?
        show_cursor     dd ?
        move_cursor     dd ?
        restore_cursor  dd ?
        disable_mouse   dd ?
        mask_seqno      dd ?
        check_mouse     dd ?
        check_m_pixel   dd ?

        bytes_per_pixel dd ?

        put_pixel       dd ?
        put_rect        dd ?
        put_image       dd ?
        put_line        dd ?
        get_pixel       dd ?
        get_rect        dd ?
        get_image       dd ?
        get_line        dd ?
ends

struct  DISPMODE
        width   dw ?
        height  dw ?
        bpp     dw ?
        freq    dw ?
ends


struct  PCIDEV
        bk              dd ?
        fd              dd ?
        vendor_device_id dd ?
        class           dd ?
        devfn           db ?
        bus             db ?
                        rb 2
        owner           dd ? ; pointer to SRV or 0
ends

struct  IDE_DATA
        ProgrammingInterface dd ?
        Interrupt            dw ?
        RegsBaseAddres       dw ?
        BAR0_val             dw ?
        BAR1_val             dw ?
        BAR2_val             dw ?
        BAR3_val             dw ?
        dma_hdd_channel_1    db ?
        dma_hdd_channel_2    db ?
        pcidev               dd ?       ; pointer to corresponding PCIDEV structure
ends

struct  IDE_CACHE
        pointer              dd ?
        size                 dd ?   ; not use
        data_pointer         dd ?
        system_data_size     dd ?   ; not use
        appl_data_size       dd ?   ; not use
        system_data          dd ?
        appl_data            dd ?
        system_sad_size      dd ?
        appl_sad_size        dd ?
        search_start         dd ?
        appl_search_start    dd ?
ends

struct  IDE_DEVICE
        UDMA_possible_modes  db ?
        UDMA_set_mode        db ?
ends

; The following macro assume that we are on uniprocessor machine.
; Serious work is needed for multiprocessor machines.
macro spin_lock_irqsave spinlock
{
        pushf
        cli
}
macro spin_unlock_irqrestore spinlock
{
        popf
}
macro spin_lock_irq spinlock
{
        cli
}
macro spin_unlock_irq spinlock
{
        sti
}

struct  MEM_STATE
        mutex           MUTEX
        smallmap        dd ?
        treemap         dd ?
        topsize         dd ?
        top             dd ?
        smallbins       rd 4*32
        treebins        rd 32
ends

struct  PG_DATA
        mem_amount      dd ?
        vesa_mem        dd ?
        pages_count     dd ?
        pages_free      dd ?
        pages_faults    dd ?
        pagemap_size    dd ?
        kernel_pages    dd ?
        kernel_tables   dd ?
        sys_page_dir    dd ?
        mutex           MUTEX
ends

struct  SRV
        srv_name        rb 16    ;ASCIIZ string
        magic           dd ?     ;+0x10 ;'SRV '
        size            dd ?     ;+0x14 ;size of structure SRV
        fd              dd ?     ;+0x18 ;next SRV descriptor
        bk              dd ?     ;+0x1C ;prev SRV descriptor
        base            dd ?     ;+0x20 ;service base address
        entry           dd ?     ;+0x24 ;service START function
        srv_proc        dd ?     ;+0x28 ;user mode service handler
        srv_proc_ex     dd ?     ;+0x2C ;kernel mode service handler
ends

struct USBSRV
        srv             SRV
        usb_func        dd ?
ends

struct USBFUNC
        strucsize       dd ?
        add_device      dd ?
        device_disconnect dd ?
ends

DRV_ENTRY    =  1
DRV_EXIT     = -1

struct  COFF_HEADER
        machine         dw ?
        nSections       dw ?
        DataTime        dd ?
        pSymTable       dd ?
        nSymbols        dd ?
        optHeader       dw ?
        flags           dw ?
ends

struct  COFF_SECTION
        Name            rb 8
        VirtualSize     dd ?
        VirtualAddress  dd ?
        SizeOfRawData   dd ?
        PtrRawData      dd ?
        PtrReloc        dd ?
        PtrLinenumbers  dd ?
        NumReloc        dw ?
        NumLinenum      dw ?
        Characteristics dd ?
ends

struct  COFF_RELOC
        VirtualAddress  dd ?
        SymIndex        dd ?
        Type            dw ?
ends

struct  COFF_SYM
        Name            rb 8
        Value           dd ?
        SectionNumber   dw ?
        Type            dw ?
        StorageClass    db ?
        NumAuxSymbols   db ?
ends

struct  STRIPPED_PE_HEADER
        Signature           dw ?
        Characteristics     dw ?
        AddressOfEntryPoint dd ?
        ImageBase           dd ?
        SectionAlignmentLog db ?
        FileAlignmentLog    db ?
        MajorOSVersion      db ?
        MinorOSVersion      db ?
        SizeOfImage         dd ?
        SizeOfStackReserve  dd ?
        SizeOfHeapReserve   dd ?
        SizeOfHeaders       dd ?
        Subsystem           db ?
        NumberOfRvaAndSizes db ?
        NumberOfSections    dw ?
ends
STRIPPED_PE_SIGNATURE = 0x4503 ; 'PE' xor 'S'
SPE_DIRECTORY_IMPORT    = 0
SPE_DIRECTORY_EXPORT    = 1
SPE_DIRECTORY_BASERELOC = 2

struct  IOCTL
        handle          dd ?
        io_code         dd ?
        input           dd ?
        inp_size        dd ?
        output          dd ?
        out_size        dd ?
ends

struct  IRQH
        list            LHEAD
        handler         dd ?   ;handler roututine
        data            dd ?   ;user-specific data
        num_ints        dd ?   ;how many times handled
ends

