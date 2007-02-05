

drw0    equ  10010010b      ; data read/write dpl0
drw3    equ  11110010b      ; data read/write dpl3
cpl0    equ  10011010b      ; code read dpl0
cpl3    equ  11111010b      ; code read dpl3

D32     equ  01000000b      ; 32bit segment
G32    equ   10000000b      ; page gran


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


CR4_VME        equ    0x0001
CR4_PVI        equ    0x0002
CR4_TSD        equ    0x0004
CR4_DE         equ    0x0008
CR4_PSE        equ    0x0010
CR4_PAE        equ    0x0020
CR4_MCE        equ    0x0040
CR4_PGE        equ    0x0080
CR4_PCE        equ    0x0100
CR4_OSFXSR     equ    0x0200
CR4_OSXMMEXPT  equ    0x0400

SSE_IE         equ    0x0001
SSE_DE         equ    0x0002
SSE_ZE         equ    0x0004
SSE_OE         equ    0x0008
SSE_UE         equ    0x0010
SSE_PE         equ    0x0020
SSE_DAZ        equ    0x0040
SSE_IM         equ    0x0080
SSE_DM         equ    0x0100
SSE_ZM         equ    0x0200
SSE_OM         equ    0x0400
SSE_UM         equ    0x0800
SSE_PM         equ    0x1000
SSE_FZ         equ    0x8000

SSE_INIT equ (SSE_IM+SSE_DM+SSE_ZM+SSE_OM+SSE_UM+SSE_PM)

OS_BASE             equ 0;  0x80400000

window_data         equ OS_BASE+0x0000000

CURRENT_TASK        equ OS_BASE+0x0003000
TASK_COUNT          equ OS_BASE+0x0003004
TASK_BASE           equ OS_BASE+0x0003010
TASK_DATA           equ OS_BASE+0x0003020
TASK_EVENT          equ OS_BASE+0x0003020

;mouseunder          equ OS_BASE+0x0006900
FLOPPY_BUFF         equ OS_BASE+0x0008000
ACTIVE_PROC_STACK   equ OS_BASE+0x000A400
idts                equ OS_BASE+0x000B100
WIN_STACK           equ OS_BASE+0x000C000
WIN_POS             equ OS_BASE+0x000C400
FDD_DATA            equ OS_BASE+0x000D000

ENABLE_TASKSWITCH   equ OS_BASE+0x000E000
PUTPIXEL            equ OS_BASE+0x000E020
GETPIXEL            equ OS_BASE+0x000E024
BANK_SWITCH         equ OS_BASE+0x000E030

MOUSE_PICTURE       equ OS_BASE+0x000F200
MOUSE_VISIBLE       equ OS_BASE+0x000F204
XY_TEMP             equ OS_BASE+0x000F300
KEY_COUNT           equ OS_BASE+0x000F400
KEY_BUFF            equ OS_BASE+0x000F401

BTN_COUNT           equ OS_BASE+0x000F500
BTN_BUFF            equ OS_BASE+0x000F501

CPU_FREQ            equ OS_BASE+0x000F600
MOUSE_PORT          equ OS_BASE+0x000F604

PS2_CHUNK           equ OS_BASE+0x000FB00
MOUSE_X             equ OS_BASE+0x000FB0A
MOUSE_Y             equ OS_BASE+0x000FB0C

MOUSE_COLOR_MEM     equ OS_BASE+0x000FB10
COLOR_TEMP          equ OS_BASE+0x000FB30
BTN_DOWN            equ OS_BASE+0x000FB40
MOUSE_DOWN          equ OS_BASE+0x000FB44
X_UNDER             equ OS_BASE+0x000FB4A
Y_UNDER             equ OS_BASE+0x000FB4C
ScreenBPP           equ OS_BASE+0x000FBF1
MOUSE_BUFF_COUNT    equ OS_BASE+0x000FCFF
LFBAddress          equ OS_BASE+0x000FE80
MEM_AMOUNT          equ OS_BASE+0x000FE8C
;LFBSize             equ OS_BASE+0x02f9050

SCR_X_SIZE          equ OS_BASE+0x000FE00
SCR_Y_SIZE          equ OS_BASE+0x000FE04
SCR_BYTES_PER_LINE  equ OS_BASE+0x000FE08
SCR_MODE            equ OS_BASE+0x000FE0C

BTN_ADDR            equ OS_BASE+0x000FE88
SYS_SHUTDOWN        equ OS_BASE+0x000FF00
TASK_ACTIVATE       equ OS_BASE+0x000FF01

REDRAW_BACKGROUND   equ OS_BASE+0x000FFF0
BANK_RW             equ OS_BASE+0x000FFF2
MOUSE_BACKGROUND    equ OS_BASE+0x000FFF4
DONT_DRAW_MOUSE     equ OS_BASE+0x000FFF5
DONT_SWITCH         equ OS_BASE+0x000FFFF

STACK_TOP           equ OS_BASE+0x003EC00

FONT_II             equ OS_BASE+0x003EC00
FONT_I              equ OS_BASE+0x003F600
DISK_DATA           equ OS_BASE+0x0040000
PROC_BASE           equ OS_BASE+0x0080000
TMP_BUFF            equ OS_BASE+0x0090000

VGABasePtr          equ OS_BASE+0x00A0000

RAMDISK             equ OS_BASE+0x0100000
RAMDISK_FAT         equ OS_BASE+0x0280000
FLOPPY_FAT          equ OS_BASE+0x0282000
SB16_Status         equ OS_BASE+0x02B0000
BUTTON_INFO         equ OS_BASE+0x02C0000
RESERVED_PORTS      equ OS_BASE+0x02D0000
IRQ_SAVE            equ OS_BASE+0x02E0000
SYS_VAR             equ OS_BASE+0x02f0000
IMG_BACKGROUND      equ OS_BASE+0x0300000
WinMapAddress       equ OS_BASE+0x0460000
display_data        equ OS_BASE+0x0460000
HD_CACHE            equ OS_BASE+0x0600000
stack_data_start    equ OS_BASE+0x0700000
eth_data_start      equ OS_BASE+0x0700000
stack_data          equ OS_BASE+0x0704000
stack_data_end      equ OS_BASE+0x071ffff
VMODE_BASE          equ OS_BASE+0x0760000
resendQ             equ OS_BASE+0x0770000

;skin_data          equ OS_BASE+0x0778000

draw_data           equ OS_BASE+0x0800000
sysint_stack_data   equ OS_BASE+0x0803000

tss_data            equ OS_BASE+0x0920000

pages_tab           equ  0x60000000
master_tab          equ  0x60180000
current_pgdir       equ  0x60180000

sys_pgdir           equ  OS_BASE+0x00050000
sys_master_tab      equ  OS_BASE+0x00051000
sys_pgmap           equ  OS_BASE+0x00052000

;lfb_start          equ  0x00800000

;new_app_pdir          equ OS_BASE+0x01000000
;new_app_master_table  equ OS_BASE+0x01001000
;new_app_ptable        equ OS_BASE+0x01002000

new_app_base        equ  0x60400000

twdw                equ (CURRENT_TASK-window_data)

std_application_base_address   equ new_app_base

PAGES_USED          equ 4

PG_UNMAP            equ 0x000
PG_MAP              equ 0x001
PG_WRITE            equ 0x002
PG_SW               equ 0x003
PG_USER             equ 0x005
PG_UW               equ 0x007
PG_NOCACHE          equ 0x018
PG_LARGE            equ 0x080
PG_GLOBAL           equ 0x100

;;;;;;;;;;;boot time variables

;BOOT_BPP            equ 0x9000    ;byte   bits per pixel
BOOT_SCANLINE       equ 0x9001    ;word   scanline length
BOOT_VESA_MODE      equ 0x9008    ;word   vesa video mode
;;BOOT_X_RES          equ 0x900A    ;word   X res
;;BOOT_Y_RES          equ 0x900C    ;word   Y res
;;BOOT_MOUSE_PORT     equ 0x9010    ;byte   mouse port  - not used
BOOT_BANK_SW        equ 0x9014    ;dword  Vesa 1.2 pm bank switch
BOOT_LFB            equ 0x9018    ;dword  Vesa 2.0 LFB address
BOOT_MTRR           equ 0x901C    ;byte   0 or 1 : enable MTRR graphics acceleration
BOOT_LOG            equ 0x901D    ;byte   not used anymore (0 or 1 : enable system log display)
BOOT_DIRECT_LFB     equ 0x901E    ;byte   0 or 1 : enable direct lfb write, paging disabled
BOOT_PCI_DATA       equ 0x9020    ;8bytes pci data
BOOT_VRR            equ 0x9030    ;byte   VRR start enabled 1, 2-no
BOOT_IDE_BASE_ADDR  equ 0x9031   ;word   IDEContrRegsBaseAddr
BOOT_MEM_AMOUNT     equ 0x9034    ;dword  memory amount

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
EVENT_EXTENDED     equ 0x00000200

EV_INTR            equ 1

struc SYS_VARS
{ .bpp            dd ?
  .scanline       dd ?
  .vesa_mode      dd ?
  .x_res          dd ?
  .y_res          dd ?
  .cpu_caps       dd ?
                  dd ?
                  dd ?
                  dd ?
}

struc APPOBJ           ;common object header
{
   .magic       dd ?   ;
   .destroy     dd ?   ;internal destructor
   .fd          dd ?   ;next object in list
   .bk          dd ?   ;prev object in list
   .pid         dd ?   ;owner id
};

virtual at 0
  APPOBJ APPOBJ
end virtual

APP_OBJ_OFFSET  equ 48

struc CURSOR
{;common object header
   .magic       dd ?   ;'CURS'
   .destroy     dd ?   ;internal destructor
   .fd          dd ?   ;next object in list
   .bk          dd ?   ;prev object in list
   .pid         dd ?   ;owner id

 ;cursor data
   .base        dd ?   ;allocated memory
   .hot_x       dd ?   ;hotspot coords
   .hot_y       dd ?
}
virtual at 0
  CURSOR CURSOR
end virtual

CURSOR_SIZE     equ 32

struc BOOT_DATA
{ .bpp            dd ?
  .scanline       dd ?
  .vesa_mode      dd ?
  .x_res          dd ?
  .y_res          dd ?
  .mouse_port     dd ?
  .bank_switch    dd ?
  .lfb            dd ?
  .vesa_mem       dd ?
  .log            dd ?
  .direct_lfb     dd ?
  .pci_data       dd ?
;                  dd ?
  .vrr            dd ?
  .ide_base       dd ?
  .mem_amount     dd ?
  .pages_count    dd ?
  .pagemap_size   dd ?
  .kernel_max     dd ?
  .kernel_pages   dd ?
  .kernel_tables  dd ?

  .cpu_vendor     dd ?
                  dd ?
                  dd ?
  .cpu_sign       dd ?
  .cpu_info       dd ?
  .cpu_caps       dd ?
                  dd ?
                  dd ?
}

virtual at 0
  BOOT_DATA BOOT_DATA
end virtual

struc MEM_STATE
{  .mutex             rd 1
   .smallmap          rd 1
   .treemap           rd 1
   .topsize           rd 1
   .top               rd 1
   .smallbins         rd 4*32
   .treebins          rd 32
}

struc PG_DATA
{ .mem_amount        dd ?
  .vesa_mem          dd ?
  .pages_count       dd ?
  .pages_free        dd ?
  .pages_faults      dd ?
  .pagemap_size      dd ?
  .kernel_max        dd ?
  .kernel_pages      dd ?
  .kernel_tables     dd ?
  .sys_page_dir      dd ?
  .pg_mutex          dd ?
  .tmp_task_mutex    dd ?
}

;struc LIB
;{ .lib_name          rb 16
;  .lib_base          dd ?
;  .lib_start         dd ?
;  .export            dd ?
;  .import            dd ?
;}

struc SRV
{ .srv_name          rb 16           ;ASCIIZ string
  .magic             dd ?     ;+0x10 ;'SRV '
  .size              dd ?     ;+0x14 ;size of structure SRV
  .fd                dd ?     ;+0x18 ;next SRV descriptor
  .bk                dd ?     ;+0x1C ;prev SRV descriptor
  .base              dd ?     ;+0x20 ;service base address
  .entry             dd ?     ;+0x24 ;service START function
  .srv_proc          dd ?     ;+0x28 ;main service handler
}

SRV_FD_OFFSET  equ 0x18
SRV_SIZE       equ 44

struc COFF_HEADER
{  .machine          dw ?
   .nSections        dw ?
   .DataTime         dd ?
   .pSymTable        dd ?
   .nSymbols         dd ?
   .optHeader        dw ?
   .flags            dw ?
};


struc COFF_SECTION
{  .Name             rb 8
   .VirtualSize      dd ?
   .VirtualAddress   dd ?
   .SizeOfRawData    dd ?
   .PtrRawData       dd ?
   .PtrReloc         dd ?
   .PtrLinenumbers   dd ?
   .NumReloc         dw ?
   .NumLinenum       dw ?
   .Characteristics  dd ?
}
COFF_SECTION_SIZE equ 40

struc COFF_RELOC
{  .VirtualAddress   dd ?
   .SymIndex         dd ?
   .Type             dw ?
}

struc COFF_SYM
{  .Name             rb 8
   .Value            dd ?
   .SectionNumber    dw ?
   .Type             dw ?
   .StorageClass     db ?
   .NumAuxSymbols    db ?
}
CSYM_SIZE equ 18

struc IOCTL
{  .handle           dd ?
   .io_code          dd ?
   .input            dd ?
   .inp_size         dd ?
   .output           dd ?
   .out_size         dd ?
}

virtual at 0
  IOCTL IOCTL
end virtual

;virtual at 0
;  LIB LIB
;end virtual

virtual at 0
  SRV SRV
end virtual

virtual at 0
  CFH COFF_HEADER
end virtual

virtual at 0
  CFS COFF_SECTION
end virtual

virtual at 0
  CRELOC COFF_RELOC
end virtual

virtual at 0
  CSYM COFF_SYM
end virtual

