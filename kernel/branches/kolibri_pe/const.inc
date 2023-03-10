;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


dpl0    equ  10010000b      ; data read       dpl0
drw0    equ  10010010b      ; data read/write dpl0
drw1    equ  10110010b      ; data read/write dpl1
drw3    equ  11110010b      ; data read/write dpl3

cpl0    equ  10011010b      ; code read dpl0
cpl1    equ  10111010b      ; code read dpl1
cpl3    equ  11111010b      ; code read dpl3

D32     equ  01000000b      ; 32bit segment
G32     equ  10000000b      ; page gran


;;;;;;;;;;;;cpu_caps flags;;;;;;;;;;;;;;;;

CPU_386        equ     3
CPU_486        equ     4
CPU_PENTIUM    equ     5
CPU_P6         equ     6
CPU_PENTIUM4   equ  0x0F

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

; CPU MSR names
MSR_SYSENTER_CS		equ	0x174
MSR_SYSENTER_ESP	equ	0x175
MSR_SYSENTER_EIP	equ	0x176
MSR_AMD_EFER		equ	0xC0000080	; Extended Feature Enable Register
MSR_AMD_STAR		equ	0xC0000081	; SYSCALL/SYSRET Target Address Register

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


struc TSS
{
  ._back       rw 2
  ._esp0       rd 1
  ._ss0        rw 2
  ._esp1       rd 1
  ._ss1        rw 2
  ._esp2       rd 1
  ._ss2        rw 2
  ._cr3        rd 1
  ._eip        rd 1
  ._eflags     rd 1
  ._eax        rd 1
  ._ecx        rd 1
  ._edx        rd 1
  ._ebx        rd 1
  ._esp        rd 1
  ._ebp        rd 1
  ._esi        rd 1
  ._edi        rd 1
  ._es         rw 2
  ._cs         rw 2
  ._ss         rw 2
  ._ds         rw 2
  ._fs         rw 2
  ._gs         rw 2
  ._ldt        rw 2
  ._trap       rw 1
  ._io         rw 1
               rb 24
  ._io_map_0   rb 4096
  ._io_map_1   rb 4096
}

virtual at 0
  TSS  TSS
end virtual

TSS_SIZE  equ (128+8192)

page_tabs           equ 0xFF800000
;app_page_tabs       equ 0xDD800000

;shared_tabs         equ 0xDDC00000

;heap_tabs           equ (page_tabs+ (HEAP_BASE shr 9))
kernel_tabs         equ page_tabs)
master_tab          equ (page_tabs+ (page_tabs shr 9))

_16BIT_BASE         equ 0x00010000
LOAD_BASE           equ 0x00100000
IMAGE_BASE          equ LOAD_BASE

LAST_PAGE           equ 0x01CB000

NCPU                equ 8

RING0_STACK_SIZE    equ (0x2000 - 512)    ;512 ???? ??? ????????? FPU

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

REG_USTACK          equ (RING0_STACK_SIZE-56)
REG_CSTACK          equ (RING0_STACK_SIZE-60)
REG_RAW             equ (RING0_STACK_SIZE-64)
REG_RESTART         equ (RING0_STACK_SIZE-68)
REG_ENTRY           equ (RING0_STACK_SIZE-72)

PG_UNMAP            equ 0x000
PG_MAP              equ 0x001
PG_WRITE            equ 0x002
PG_SW               equ 0x003
PG_USER             equ 0x005
PG_UW               equ 0x007
PG_NOCACHE          equ 0x018
PG_LARGE            equ 0x080
PG_GLOBAL           equ 0x100

PG_SHARED           equ 0x200

;;;;;;;;;;;boot time variables

;BOOT_BPP            equ 0x9000    ;byte   bits per pixel
BOOT_SCANLINE       equ 0x9001    ;word   scanline length
BOOT_VESA_MODE      equ 0x9008    ;word   vesa video mode
;;BOOT_X_RES          equ 0x900A    ;word   X res
;;BOOT_Y_RES          equ 0x900C    ;word   Y res
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

struc THR_DATA
{
                  rb (8192-512)
  .pl0_stack:
  .fpu_state      rb 512
  .tls_page       rb 4096
  .pdbr           rb 4096
}

THR_DATA_SIZE     equ 4096*4

virtual at (OS_BASE-THR_DATA_SIZE)
  thr_data  THR_DATA
end virtual

struc SYS_VARS
{ .bpp            dd ?
  .scanline       dd ?
  .vesa_mode      dd ?
  .x_res          dd ?
  .y_res          dd ?
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
APP_EV_OFFSET   equ 40

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

struc EVENT
{
   .magic       dd ?   ;'EVNT'
   .destroy     dd ?   ;internal destructor
   .fd          dd ?   ;next object in list
   .bk          dd ?   ;prev object in list
   .pid         dd ?   ;owner id

   .id          dd ?   ;event uid
   .state       dd ?   ;internal flags
   .code        dd ?
                rd 5
}
EVENT_SIZE  equ 52

virtual at 0
  EVENT EVENT
end virtual


struc SMEM
{
    .bk         dd ?
    .fd         dd ?    ;+4
    .base       dd ?    ;+8
    .size       dd ?    ;+12
    .access     dd ?    ;+16
    .refcount   dd ?    ;+20
    .name       rb 32   ;+24
    .sizeof:
}

struc SMAP
{
   .magic       dd ?   ; SMAP
   .destroy     dd ?   ;internal destructor
   .fd          dd ?   ;next object in list
   .bk          dd ?   ;prev object in list
   .pid         dd ?   ;owner id

   .base        dd ?   ;mapped base
   .parent      dd ?   ;SMEM
   .sizeof:
}

virtual at 0
  SMEM SMEM
end virtual

virtual at 0
  SMAP SMAP
end virtual


struc HEAP_DATA
{
   .mutex         rd 1
   .refcount      rd 1
   .heap_base     rd 1
   .heap_top      rd 1
   .app_mem       rd 1
}

HEAP_DATA_SIZE  equ 20
virtual at 0
  HEAP_DATA HEAP_DATA
end virtual

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
{  .mutex            rd 1
   .smallmap         rd 1
   .treemap          rd 1
   .topsize          rd 1
   .top              rd 1
   .smallbins        rd 4*32
   .treebins         rd 32
}

struc PG_DATA
{ .mem_amount        dd ?
  .vesa_mem          dd ?
  .pages_faults      dd ?
  .pagemap_size      dd ?
  .kernel_pages      dd ?
  .kernel_tables     dd ?
  .sys_page_dir      dd ?
  .pg_mutex          dd ?
}

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

DRV_ENTRY    equ  1
DRV_EXIT     equ -1

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

struc THR
{
  .edi            rd 1     ;   0x00
  .esi            rd 1     ;   0x04
  .ebp            rd 1     ;   0x08
                  rd 1     ;   0x0C    ;esp
  .ebx            rd 1     ;   0x10
  .edx            rd 1     ;   0x14
  .ecx            rd 1     ;   0x18
  .eax            rd 1     ;   0x1C
  .retaddr        rd 1     ;   0x20    ;??? ??? ??????
  .eip            rd 1     ;   0x24
  .cs             rd 1     ;   0x28
  .eflags         rd 1     ;   0x2C
  .esp            rd 1     ;   0x30
  .ss             rd 1     ;   0x34

  .pl0_stack:

  .tid            rd 1     ;       ; number of this process (for fast access)
  .slot           rd 1     ;       ; number of this process (for fast access)
  .pdir           rd 1     ;       ;
  .thr_flags      rd 1     ;       ; process is runnable only if zero
  .ticks_left     rd 1     ;       ; number of scheduling ticks left
  .quantum_size   rd 1     ;       ; quantum size in ticks
  .user_time      rd 1     ;       ; user time in ticks
  .sys_time       rd 1     ;       ; sys time in ticks
}

virtual at 0
  THR  THR
end virtual


