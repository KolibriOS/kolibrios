;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


dpl0    equ  10010000b      ; data read       dpl0
drw0    equ  10010010b      ; data read/write dpl0
drw3    equ  11110010b      ; data read/write dpl3
cpl0    equ  10011010b      ; code read dpl0
cpl3    equ  11111010b      ; code read dpl3

D32     equ  01000000b      ; 32bit segment
G32     equ  10000000b      ; page gran


;;;;;;;;;;;;cpu_caps flags;;;;;;;;;;;;;;;;

CPU_386        equ 3
CPU_486        equ 4
CPU_PENTIUM    equ 5
CPU_P6         equ 6
CPU_PENTIUM4   equ 0x0F

CAPS_FPU       equ    00 ;on-chip x87 floating point unit
CAPS_VME       equ    01 ;virtual-mode enhancements
CAPS_DE        equ    02 ;debugging extensions
CAPS_PSE       equ    03 ;page-size extensions
CAPS_TSC       equ    04 ;time stamp counter
CAPS_MSR       equ    05 ;model-specific registers
CAPS_PAE       equ    06 ;physical-address extensions
CAPS_MCE       equ    07 ;machine check exception
CAPS_CX8       equ    08 ;CMPXCHG8B instruction
CAPS_APIC      equ    09 ;on-chip advanced programmable
                         ; interrupt controller
;                     10 ;unused
CAPS_SEP       equ    11 ;SYSENTER and SYSEXIT instructions
CAPS_MTRR      equ    12 ;memory-type range registers
CAPS_PGE       equ    13 ;page global extension
CAPS_MCA       equ    14 ;machine check architecture
CAPS_CMOV      equ    15 ;conditional move instructions
CAPS_PAT       equ    16 ;page attribute table

CAPS_PSE36     equ    17 ;page-size extensions
CAPS_PSN       equ    18 ;processor serial number
CAPS_CLFLUSH   equ    19 ;CLFUSH instruction

CAPS_DS        equ    21 ;debug store
CAPS_ACPI      equ    22 ;thermal monitor and software
                         ;controlled clock supported
CAPS_MMX       equ    23 ;MMX instructions
CAPS_FXSR      equ    24 ;FXSAVE and FXRSTOR instructions
CAPS_SSE       equ    25 ;SSE instructions
CAPS_SSE2      equ    26 ;SSE2 instructions
CAPS_SS        equ    27 ;self-snoop
CAPS_HTT       equ    28 ;hyper-threading technology
CAPS_TM        equ    29 ;thermal monitor supported
CAPS_IA64      equ    30 ;IA64 capabilities
CAPS_PBE       equ    31 ;pending break enable

;ecx
CAPS_SSE3      equ    32 ;SSE3 instructions
;                     33
;                     34
CAPS_MONITOR   equ    35 ;MONITOR/MWAIT instructions
CAPS_DS_CPL    equ    36 ;
CAPS_VMX       equ    37 ;virtual mode extensions
;                     38 ;
CAPS_EST       equ    39 ;enhansed speed step
CAPS_TM2       equ    40 ;thermal monitor2 supported
;                     41
CAPS_CID       equ    42 ;
;                     43
;                     44
CAPS_CX16      equ    45 ;CMPXCHG16B instruction
CAPS_xTPR      equ    46 ;
CAPS_XSAVE     equ    (32 + 26) ; XSAVE and XRSTOR instructions
CAPS_OSXSAVE   equ    (32 + 27)
; A value of 1 indicates that the OS has set CR4.OSXSAVE[bit 18] to enable
; XSETBV/XGETBV instructions to access XCR0 and to support processor extended
; state management using XSAVE/XRSTOR.
CAPS_AVX       equ    (32 + 28) ; not AVX2
;
;reserved
;
;ext edx /ecx
CAPS_SYSCAL    equ    64 ;
CAPS_XD        equ    65 ;execution disable
CAPS_FFXSR     equ    66 ;
CAPS_RDTSCP    equ    67 ;
CAPS_X64       equ    68 ;
CAPS_3DNOW     equ    69 ;
CAPS_3DNOWEXT  equ    70 ;
CAPS_LAHF      equ    71 ;
CAPS_CMP_LEG   equ    72 ;
CAPS_SVM       equ    73 ;secure virual machine
CAPS_ALTMOVCR8 equ    74 ;

; CPU MSR names
MSR_SYSENTER_CS         equ     0x174
MSR_SYSENTER_ESP        equ     0x175
MSR_SYSENTER_EIP        equ     0x176
MSR_CR_PAT              equ     0x277
MSR_MTRR_DEF_TYPE       equ     0x2FF

MSR_AMD_EFER            equ     0xC0000080      ; Extended Feature Enable Register
MSR_AMD_STAR            equ     0xC0000081      ; SYSCALL/SYSRET Target Address Register

CR0_PE         equ    0x00000001   ;protected mode
CR0_MP         equ    0x00000002   ;monitor fpu
CR0_EM         equ    0x00000004   ;fpu emulation
CR0_TS         equ    0x00000008   ;task switch
CR0_ET         equ    0x00000010   ;extension type hardcoded to 1
CR0_NE         equ    0x00000020   ;numeric error
CR0_WP         equ    0x00010000   ;write protect
CR0_AM         equ    0x00040000   ;alignment check
CR0_NW         equ    0x20000000   ;not write-through
CR0_CD         equ    0x40000000   ;cache disable
CR0_PG         equ    0x80000000   ;paging


CR4_VME        equ    0x000001
CR4_PVI        equ    0x000002
CR4_TSD        equ    0x000004
CR4_DE         equ    0x000008
CR4_PSE        equ    0x000010
CR4_PAE        equ    0x000020
CR4_MCE        equ    0x000040
CR4_PGE        equ    0x000080
CR4_PCE        equ    0x000100
CR4_OSFXSR     equ    0x000200
CR4_OSXMMEXPT  equ    0x000400
CR4_OSXSAVE    equ    0x040000

XCR0_FPU_MMX   equ    0x0001
XCR0_SSE       equ    0x0002
XCR0_AVX       equ    0x0004
XCR0_MPX       equ    0x0018
XCR0_AVX512    equ    0x00e0

MXCSR_IE       equ    0x0001
MXCSR_DE       equ    0x0002
MXCSR_ZE       equ    0x0004
MXCSR_OE       equ    0x0008
MXCSR_UE       equ    0x0010
MXCSR_PE       equ    0x0020
MXCSR_DAZ      equ    0x0040
MXCSR_IM       equ    0x0080
MXCSR_DM       equ    0x0100
MXCSR_ZM       equ    0x0200
MXCSR_OM       equ    0x0400
MXCSR_UM       equ    0x0800
MXCSR_PM       equ    0x1000
MXCSR_FZ       equ    0x8000

MXCSR_INIT     equ (MXCSR_IM+MXCSR_DM+MXCSR_ZM+MXCSR_OM+MXCSR_UM+MXCSR_PM)

EFLAGS_CF      equ    0x000001  ; carry flag
EFLAGS_PF      equ    0x000004  ; parity flag
EFLAGS_AF      equ    0x000010  ; auxiliary flag
EFLAGS_ZF      equ    0x000040  ; zero flag
EFLAGS_SF      equ    0x000080  ; sign flag
EFLAGS_TF      equ    0x000100  ; trap flag
EFLAGS_IF      equ    0x000200  ; interrupt flag
EFLAGS_DF      equ    0x000400  ; direction flag
EFLAGS_OF      equ    0x000800  ; overflow flag
EFLAGS_IOPL    equ    0x003000  ; i/o priviledge level
EFLAGS_NT      equ    0x004000  ; nested task flag
EFLAGS_RF      equ    0x010000  ; resume flag
EFLAGS_VM      equ    0x020000  ; virtual 8086 mode flag
EFLAGS_AC      equ    0x040000  ; alignment check flag
EFLAGS_VIF     equ    0x080000  ; virtual interrupt flag
EFLAGS_VIP     equ    0x100000  ; virtual interrupt pending
EFLAGS_ID      equ    0x200000  ; id flag

IRQ_PIC        equ    0
IRQ_APIC       equ    1

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

DRIVE_DATA_SIZE     equ 16

OS_BASE             equ 0x80000000

window_data         equ (OS_BASE+0x0001000)

CURRENT_TASK        equ (OS_BASE+0x0003000)
TASK_COUNT          equ (OS_BASE+0x0003004)
TASK_BASE           equ (OS_BASE+0x0003010)
TASK_DATA           equ (OS_BASE+0x0003020)
TASK_EVENT          equ (OS_BASE+0x0003020)

CDDataBuf           equ (OS_BASE+0x0005000)

;unused                 0x6000 - 0x8fff

BOOT_VARS           equ (OS_BASE)               ;0x9000

idts                equ (OS_BASE+0x000B100)
WIN_STACK           equ (OS_BASE+0x000C000)
WIN_POS             equ (OS_BASE+0x000C400)
FDD_BUFF            equ (OS_BASE+0x000D000)     ;512

WIN_TEMP_XY         equ (OS_BASE+0x000F300)
KEY_COUNT           equ (OS_BASE+0x000F400)
KEY_BUFF            equ (OS_BASE+0x000F401) ; 120*2 + 2*2 = 244 bytes, actually 255 bytes

BTN_COUNT           equ (OS_BASE+0x000F500)
BTN_BUFF            equ (OS_BASE+0x000F501)


BTN_ADDR            equ (OS_BASE+0x000FE88)
MEM_AMOUNT          equ (OS_BASE+0x000FE8C)

SYS_SHUTDOWN        equ (OS_BASE+0x000FF00)
TASK_ACTIVATE       equ (OS_BASE+0x000FF01)


TMP_STACK_TOP       equ 0x006CC00

sys_proc            equ (OS_BASE+0x006F000)

SLOT_BASE           equ (OS_BASE+0x0080000)

VGABasePtr          equ (OS_BASE+0x00A0000)

CLEAN_ZONE          equ (_CLEAN_ZONE-OS_BASE)

UPPER_KERNEL_PAGES  equ (OS_BASE+0x0400000)

virtual at              (OS_BASE+0x05FFF80)
  tss  TSS
end virtual

HEAP_BASE           equ (OS_BASE+0x0800000)
HEAP_MIN_SIZE       equ 0x01000000

page_tabs           equ 0xFDC00000
app_page_tabs       equ 0xFDC00000
kernel_tabs         equ (page_tabs+ (OS_BASE shr 10))   ;0xFDE00000
master_tab          equ (page_tabs+ (page_tabs shr 10)) ;0xFDFF70000

LFB_BASE            equ 0xFE000000


new_app_base        equ 0;

twdw                equ 0x2000   ;(CURRENT_TASK-window_data)

std_application_base_address   equ new_app_base
RING0_STACK_SIZE    equ 0x2000

REG_SS              equ (RING0_STACK_SIZE-4)
REG_APP_ESP         equ (RING0_STACK_SIZE-8)
REG_EFLAGS          equ (RING0_STACK_SIZE-12)
REG_CS              equ (RING0_STACK_SIZE-16)
REG_EIP             equ (RING0_STACK_SIZE-20)
REG_EAX             equ (RING0_STACK_SIZE-24)
REG_ECX             equ (RING0_STACK_SIZE-28)
REG_EDX             equ (RING0_STACK_SIZE-32)
REG_EBX             equ (RING0_STACK_SIZE-36)
REG_ESP             equ (RING0_STACK_SIZE-40)  ;RING0_STACK_SIZE-20
REG_EBP             equ (RING0_STACK_SIZE-44)
REG_ESI             equ (RING0_STACK_SIZE-48)
REG_EDI             equ (RING0_STACK_SIZE-52)
REG_RET             equ (RING0_STACK_SIZE-56)  ;irq0.return


PAGE_SIZE           equ 4096

PG_UNMAP            equ 0x000
PG_READ             equ 0x001
PG_WRITE            equ 0x002
PG_USER             equ 0x004
PG_PCD              equ 0x008
PG_PWT              equ 0x010
PG_ACCESSED         equ 0x020
PG_DIRTY            equ 0x040
PG_PAT              equ 0x080
PG_GLOBAL           equ 0x100
PG_SHARED           equ 0x200

PG_SWR              equ 0x003 ; (PG_WRITE+PG_READ)
PG_UR               equ 0x005 ; (PG_USER+PG_READ)
PG_UWR              equ 0x007 ; (PG_USER+PG_WRITE+PG_READ)
PG_NOCACHE          equ 0x018 ; (PG_PCD+PG_PWT)

PDE_LARGE           equ 0x080

PAT_WB              equ 0x000
PAT_WC              equ 0x008
PAT_UCM             equ 0x010
PAT_UC              equ 0x018

PAT_TYPE_UC         equ 0
PAT_TYPE_WC         equ 1
PAT_TYPE_WB         equ 6
PAT_TYPE_UCM        equ 7

PAT_VALUE           equ 0x00070106; (UC<<24)|(UCM<<16)|(WC<<8)|WB

;;;;;;;;;;;boot time variables

BOOT_BPP            equ 0x9000    ;byte   bits per pixel
BOOT_PITCH          equ 0x9001    ;word   scanline length
BOOT_VESA_MODE      equ 0x9008    ;word   vesa video mode
BOOT_X_RES          equ 0x900A    ;word   X res
BOOT_Y_RES          equ 0x900C    ;word   Y res
BOOT_BANK_SW        equ 0x9014    ;dword  Vesa 1.2 pm bank switch
BOOT_LFB            equ 0x9018    ;dword  Vesa 2.0 LFB address
BOOT_MTRR           equ 0x901C    ;byte   0 or 1 : enable MTRR graphics acceleration
BOOT_LAUNCHER_START equ 0x901D    ;byte   (0 or 1) start the first app (right now it's LAUNCHER) after kernel is loaded?
BOOT_DEBUG_PRINT    equ 0x901E    ;byte   If nonzero, duplicates debug output to the screen.
BOOT_DMA            equ 0x901F    ;byte   DMA write : 1=yes, 2=no
BOOT_PCI_DATA       equ 0x9020    ;8bytes pci data
BOOT_SHUTDOWN_TYPE  equ 0x9030    ;byte   shutdown type (see sysfn 18.9)
BOOT_MEM_AMOUNT     equ 0x9034    ;dword  memory amount

BOOT_APM_ENTRY      equ 0x9040
BOOT_APM_VERSION    equ 0x9044
BOOT_APM_FLAGS      equ 0x9046
BOOT_APM_CODE_32    equ 0x9050
BOOT_APM_CODE_16    equ 0x9052
BOOT_APM_DATA_16    equ 0x9054
BOOT_DEV            equ 0x9056    ; byte
BOOT_KERNEL_RESTART equ 0x9058    ; word
BOOT_BX_FROM_LOAD   equ 0x905A    ; word
; low byte: a,b,c,d -- hdX, r -- rdX
; high byte: symbol 'X' as in the line above, e.g. '1', not 1
; see loader_doc.txt for details
BOOT_ACPI_RSDP      equ 0x905C    ; dword

BOOT_BIOS_HD_CNT     equ 0x907F   ; byte   number of BIOS hard disks
BOOT_BIOS_HD         equ 0x9080   ; Nbytes BIOS hard disks
BOOT_MEMMAP_BLOCK_CNT equ 0x9100  ; word   available physical memory map: number of blocks
BOOT_MEMMAP_BLOCKS   equ 0x9104   ; available physical memory map: blocks, i.e. e820entry structs
MAX_MEMMAP_BLOCKS   equ 32

TMP_FILE_NAME       equ     0
TMP_CMD_LINE        equ  1024
TMP_ICON_OFFS       equ  1280


EVENT_REDRAW       equ 0x00000001
EVENT_KEY          equ 0x00000002
EVENT_BUTTON       equ 0x00000004
EVENT_BACKGROUND   equ 0x00000010
EVENT_MOUSE        equ 0x00000020
EVENT_IPC          equ 0x00000040
EVENT_NETWORK      equ 0x00000080
EVENT_DEBUG        equ 0x00000100
EVENT_NETWORK2     equ 0x00000200
EVENT_EXTENDED     equ 0x00000400

EV_INTR            equ 1

STDIN_FILENO       equ 0
STDOUT_FILENO      equ 1
STDERR_FILENO      equ 2

SYSTEM_SHUTDOWN    equ 2
SYSTEM_REBOOT      equ 3
SYSTEM_RESTART     equ 4

BLIT_CLIENT_RELATIVE equ 0x20000000

struct SYSCALL_STACK
        _eip            dd ?
        _edi            dd ?    ;  +4
        _esi            dd ?    ;  +8
        _ebp            dd ?    ; +12
        _esp            dd ?    ; +16
        _ebx            dd ?    ; +20
        _edx            dd ?    ; +24
        _ecx            dd ?    ; +28
        _eax            dd ?    ; +32
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

FUTEX_INIT      equ 0
FUTEX_DESTROY   equ 1
FUTEX_WAIT      equ 2
FUTEX_WAKE      equ 3

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
        cursor          dd ?            ;+44
        fd_ev           dd ?            ;+48
        bk_ev           dd ?            ;+52
        fd_obj          dd ?            ;+56
        bk_obj          dd ?            ;+60
        saved_esp       dd ?            ;+64
        io_map          rd 2            ;+68
        dbg_state       dd ?            ;+76
        cur_dir         dd ?            ;+80
        wait_timeout    dd ?            ;+84
        saved_esp0      dd ?            ;+88
        wait_begin      dd ?            ;+92   +++
        wait_test       dd ?            ;+96   +++
        wait_param      dd ?            ;+100  +++
        tls_base        dd ?            ;+104
                        dd ?            ;+108
        event_filter    dd ?            ;+112
        draw_bgr_x      dd ?            ;+116
        draw_bgr_y      dd ?            ;+120
                        dd ?            ;+124
        wnd_shape       dd ?            ;+128
        wnd_shape_scale dd ?            ;+132
                        dd ?            ;+136
                        dd ?            ;+140
        saved_box       BOX             ;+144
        ipc_start       dd ?            ;+160
        ipc_size        dd ?            ;+164
        event_mask      dd ?            ;+168
        debugger_slot   dd ?            ;+172
        terminate_protection dd ?       ;+176
        keyboard_mode   db ?            ;+180
        captionEncoding db ?
                        rb 2
        exec_params     dd ?            ;+184
        dbg_event_mem   dd ?            ;+188
        dbg_regs        DBG_REGS        ;+192
        wnd_caption     dd ?            ;+212
        wnd_clientbox   BOX             ;+216
        priority        dd ?            ;+232
        in_schedule     LHEAD           ;+236
ends

APP_OBJ_OFFSET  equ 48
APP_EV_OFFSET   equ 40

struct  TASKDATA
        event_mask      dd ?
        pid             dd ?
                        dw ?
        state           db ?
                        db ?
                        dw ?
        wnd_number      db ?
                        db ?
        mem_start       dd ?
        counter_sum     dd ?
        counter_add     dd ?
        cpu_usage       dd ?
ends

TSTATE_RUNNING        = 0
TSTATE_RUN_SUSPENDED  = 1
TSTATE_WAIT_SUSPENDED = 2
TSTATE_ZOMBIE         = 3
TSTATE_TERMINATING    = 4
TSTATE_WAITING        = 5
TSTATE_FREE           = 9

; constants definition
WSTATE_NORMAL    = 00000000b
WSTATE_MAXIMIZED = 00000001b
WSTATE_MINIMIZED = 00000010b
WSTATE_ROLLEDUP  = 00000100b

WSTATE_REDRAW    = 00000001b
WSTATE_WNDDRAWN  = 00000010b

WSTYLE_HASCAPTION     = 00010000b
WSTYLE_CLIENTRELATIVE = 00100000b

ZPOS_DESKTOP            = -2
ZPOS_ALWAYS_BACK        = -1
ZPOS_NORMAL             = 0
ZPOS_ALWAYS_TOP         = 1     ;ZPOS_ALWAYS_TOP is always last and has max number!
; structures definition
struct  WDATA
        box             BOX
        cl_workarea     dd ?
        cl_titlebar     dd ?
        cl_frames       dd ?
        z_modif         db ?
        fl_wstate       db ?
        fl_wdrawn       db ?
        fl_redraw       db ?
ends

label WDATA.fl_wstyle byte at WDATA.cl_workarea + 3


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


struct  BOOT_DATA
        bpp             dd ?
        scanline        dd ?
        vesa_mode       dd ?
        x_res           dd ?
        y_res           dd ?
        mouse_port      dd ?
        bank_switch     dd ?
        lfb             dd ?
        vesa_mem        dd ?
        log             dd ?
        direct_lfb      dd ?
        pci_data        dd ?
                        dd ?
        ide_base        dd ?
        mem_amount      dd ?
        pages_count     dd ?
        pagemap_size    dd ?
        kernel_max      dd ?
        kernel_pages    dd ?
        kernel_tables   dd ?

        cpu_vendor      dd ?
                        dd ?
                        dd ?
        cpu_sign        dd ?
        cpu_info        dd ?
        cpu_caps        dd ?
                        dd ?
                        dd ?
ends

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

DRV_ENTRY    equ  1
DRV_EXIT     equ -1

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

struct DQ
        lo dd ?
        hi dd ?
ends

struct e820entry
        addr DQ ?
        size DQ ?
        type dd ?
ends

