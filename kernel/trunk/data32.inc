;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2012. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


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



if lang eq ru
  boot_initirq:     cp866 'Инициализация IRQ',0
  boot_picinit:     cp866 'Инициализация PIC',0
  boot_v86machine:  cp866 'Инициализация системы V86 машины',0
  boot_inittimer:   cp866 'Инициализация системного таймера (IRQ0)',0
  boot_initapic:    cp866 'Попытка инициализации APIC',0
  boot_enableirq:   cp866 'Включить прерывания 2, 6, 13, 14, 15',0
  boot_enablint_ide:cp866 'Разрешение прерываний в контроллере IDE',0
  boot_detectfloppy:cp866 'Поиск floppy дисководов',0
  boot_detecthdcd:  cp866 'Поиск жестких дисков и ATAPI приводов',0
  boot_getcache:    cp866 'Получение памяти для кэша',0
  boot_detectpart:  cp866 'Поиск разделов на дисковых устройствах',0
  boot_init_sys:    cp866 'Инициализация системного каталога /sys',0
  boot_loadlibs:    cp866 'Загрузка библиотек (.obj)',0
  boot_memdetect:   cp866 'Количество оперативной памяти','     ',' Мб',0
  boot_tss:         cp866 'Установка TSSs',0
  boot_cpuid:       cp866 'Чтение CPUIDs',0
;  boot_devices:     cp866 'Поиск устройств',0
  boot_timer:       cp866 'Установка таймера',0
  boot_irqs:        cp866 'Переопределение IRQ',0
  boot_setmouse:    cp866 'Установка мыши',0
  boot_windefs:     cp866 'Установка настроек окон по умолчанию',0
  boot_bgr:         cp866 'Установка фона',0
  boot_resirqports: cp866 'Резервирование IRQ и портов',0
  boot_setrports:   cp866 'Установка адресов IRQ',0
  boot_setostask:   cp866 'Создание процесса ядра',0
  boot_allirqs:     cp866 'Открытие всех IRQ',0
  boot_tsc:         cp866 'Чтение TSC',0
  boot_cpufreq:     cp866 'Частота процессора ','    ',' МГц',0
  boot_pal_ega:     cp866 'Установка EGA/CGA 320x200 палитры',0
  boot_pal_vga:     cp866 'Установка VGA 640x480 палитры',0
  boot_failed:      cp866 'Загрузка первого приложения не удалась',0
  boot_mtrr:        cp866 'Установка MTRR',0

  boot_APIC_found:  cp866 'APIC включен', 0
  boot_APIC_nfound: cp866 'APIC не найден', 0
if preboot_blogesc
  boot_tasking:     cp866 'Все готово для запуска, нажмитре ESC для старта',0
end if
else if lang eq sp
  include 'data32sp.inc'
else
  boot_initirq      db   'Initialize IRQ',0
  boot_picinit      db   'Initialize PIC',0
  boot_v86machine   db   'Initialize system V86 machine',0
  boot_inittimer    db   'Initialize system timer (IRQ0)',0
  boot_initapic     db   'Try to initialize APIC',0
  boot_enableirq    db   'Enable interrupts 2, 6, 13, 14, 15',0
  boot_enablint_ide db   'Enable interrupts in IDE controller',0
  boot_detectfloppy db   'Search floppy drives',0
  boot_detecthdcd   db   'Search hard drives and ATAPI drives',0
  boot_getcache     db   'Get memory for cache',0
  boot_detectpart   db   'Search partitions on disk devices',0
  boot_init_sys     db   'Initialize system directory /sys',0
  boot_loadlibs     db   'Loading librares (.obj)',0
  boot_memdetect    db   'Determining amount of memory',0
  boot_tss          db   'Setting TSSs',0
  boot_cpuid        db   'Reading CPUIDs',0
;  boot_devices      db   'Detecting devices',0
  boot_setmouse     db   'Setting mouse',0
  boot_windefs      db   'Setting window defaults',0
  boot_bgr          db   'Calculating background',0
  boot_resirqports  db   'Reserving IRQs & ports',0
  boot_setostask    db   'Setting OS task',0
  boot_allirqs      db   'Unmasking IRQs',0
  boot_tsc          db   'Reading TSC',0
  boot_cpufreq      db   'CPU frequency is ','    ',' MHz',0
  boot_pal_ega      db   'Setting EGA/CGA 320x200 palette',0
  boot_pal_vga      db   'Setting VGA 640x480 palette',0
  boot_failed       db   'Failed to start first app',0
  boot_mtrr         db   'Setting MTRR',0

  boot_APIC_found   db   'APIC enabled', 0
  boot_APIC_nfound  db   'APIC not found', 0
if preboot_blogesc
  boot_tasking      db   'All set - press ESC to start',0
end if
end if

;new_process_loading db 'K : New Process - loading',13,10,0
;new_process_running db 'K : New Process - done',13,10,0
start_not_enough_memory db 'K : New Process - not enough memory',13,10,0

msg_unresolved db 'unresolved ',0
msg_module     db 'in module ',0
if ~ lang eq sp
msg_version    db 'incompatible driver version',13,10,0
msg_www        db 'please visit www.kolibrios.org',13,10,0
end if
msg_CR         db  13,10,0

intel_str      db "GenuineIntel",0
AMD_str        db "AuthenticAMD",0

szHwMouse       db 'ATI2D',0
szPS2MDriver    db 'PS2MOUSE',0
;szCOM_MDriver   db 'COM_MOUSE',0
szVidintel      db 'vidintel',0
szUSB           db 'USB',0
szAtiHW         db '/rd/1/drivers/ati2d.drv',0

szSTART        db 'START',0
szEXPORTS      db 'EXPORTS',0
sz_EXPORTS     db '_EXPORTS',0

szIMPORTS      db 'IMPORTS',0

read_firstapp  db '/sys/'
firstapp       db  'LAUNCHER',0
notifyapp      db '@notify',0
if lang eq ru
ud_user_message: cp866 'Ошибка: неподдерживаемая инструкция процессора',0
else if ~ lang eq sp
ud_user_message db 'Error: unsupported processor instruction',0
end if

bootpath       db  '/KOLIBRI    '
bootpath2      db  0
vmode          db  '/sys/drivers/VMODE.MDR',0
;vrr_m          db  'VRR_M',0
kernel_file    db  'KERNEL  MNT'

dev_data_path  db '/RD/1/DRIVERS/DEVICES.DAT',0

align 4

shmem_list:
 .bk           dd shmem_list
 .fd           dd shmem_list

dll_list:
 .bk           dd dll_list
 .fd           dd dll_list

pcidev_list:
 .bk           dd pcidev_list
 .fd           dd pcidev_list

MAX_DEFAULT_DLL_ADDR = 0x80000000
MIN_DEFAULT_DLL_ADDR = 0x70000000
dll_cur_addr   dd MIN_DEFAULT_DLL_ADDR

; supported videomodes


; mike.dld {
;db 0
;dd servetable-0x10000
;align 4
;draw_line       dd __sys_draw_line
;draw_pointer    dd __sys_draw_pointer
;//mike.dld, 2006-08-02 [
;;drawbar         dd __sys_drawbar
;;drawbar         dd __sys_drawbar.forced
;drawbar         dd vesa20_drawbar
;//mike.dld, 2006-08-02 ]
;putpixel        dd __sys_putpixel
; } mike.dld


align 4
keyboard   dd 1
syslang    dd 1

boot_y     dd 10

pci_bios_entry  dd 0
                dw pci_code_sel

if __DEBUG__ eq 1
  include_debug_strings
end if

IncludeIGlobals

align 16
gdts:

        dw     gdte-$-1
        dd     gdts
        dw     0

; Attention! Do not change the order of the first four selectors. They are used in Fast System Call
; must be : os_code, os_data, app_code, app_data, ....

int_code_l:
os_code_l:
        dw     0xffff
        dw     0x0000
        db     0x00
        dw     11011111b *256 +10011010b
        db     0x00

int_data_l:
os_data_l:
        dw     0xffff
        dw     0x0000
        db     0x00
        dw     11011111b *256 +10010010b
        db     0x00

app_code_l:
        dw 0xFFFF
        dw 0
        db 0
        db cpl3
        dw G32+D32+0xF;

app_data_l:
        dw 0xFFFF
        dw 0
        db 0
        db drw3
        dw G32+D32+0xF;

; ------------- PCI BIOS ------------------

pci_code_32:
        dw 0         ;lim  0-15
        dw 0         ;base 0-15
        db 0         ;base 16-23
        db cpl0      ;type
        db D32       ;lim 16-19+props
        db 0         ;base 24-31

pci_data_32:
        dw 0         ;lim  0-15
        dw 0         ;base 0-15
        db 0         ;base 16-23
        db dpl0      ;type
        db D32       ;lim 16-19+props
        db 0         ;base 24-31

; --------------- APM ---------------------
apm_code_32:
        dw     0x0f        ; limit 64kb
        db     0, 0, 0
        dw     11010000b *256 +10011010b
        db     0x00
apm_code_16:
        dw     0x0f
        db     0, 0, 0
        dw     10010000b *256 +10011010b
        db     0x00
apm_data_16:
        dw     0x0f
        db     0, 0, 0
        dw     10010000b *256 +10010010b
        db     0x00
; -----------------------------------------

graph_data_l:

        dw     0x7ff
        dw     0x0000
        db     0x00
        dw     11010000b *256 +11110010b
        db     0x00
tss0_l:
        dw     sizeof.TSS-1
        dw     tss and 0xFFFF
        db     (tss shr 16) and 0xFF
        db     10001001b
        dw     (tss shr 16) and 0xFF00

tls_data_l:
        dw 0x0FFF
        dw 0
        db 0
        db drw3
        dw D32

endofcode:
gdte:

diff16 "end of .data segment",0,$

align 16
cur_saved_data:
                    rb 4096
fpu_data:
                    rb 512

mem_block_list      rd 64*2
mem_used_list       rd 64*2
mem_hash_cnt        rd 64

cpu_freq            rq 1

heap_mutex          MUTEX
heap_size           rd 1
heap_free           rd 1
heap_blocks         rd 1
free_blocks         rd 1

mem_block_mask      rd 2
next_memblock       rd 1


mst                 MEM_STATE

page_start          rd 1
page_end            rd 1
sys_page_map        rd 1
os_stack_seg        rd 1


srv.fd              rd 1
srv.bk              rd 1


align 16

_display            display_t

_WinMapAddress      rd 1
_WinMapSize         rd 1

LFBAddress          rd 1
Screen_Max_X        rd 1
Screen_Max_Y        rd 1

SCR_MODE            rw 2

PUTPIXEL            rd 1
GETPIXEL            rd 1

if VESA_1_2_VIDEO
BANK_SWITCH         rd 1  reserved for vesa 1.2
BANK_RW             rd 1
end if

REDRAW_BACKGROUND   rb 4

align 4
draw_data:          rb 16*256
BPSLine_calc_area   rd 1440
d_width_calc_area   rd 1140

mouseunder          rd 16*24

MOUSE_PICTURE       rd 1

MOUSE_SCROLL_H      rw 1
MOUSE_X:            rw 1
MOUSE_Y:            rw 1
MOUSE_SCROLL_V      rw 1

X_UNDER             rw 1
Y_UNDER             rw 1
COLOR_TEMP          rd 1
MOUSE_COLOR_MEM     rd 1

BTN_DOWN:           rb 4

align 4
def_cursor          rd 1
def_cursor_clock    rd 1
current_cursor      rd 1
hw_cursor           rd 1
cur_saved_base      rd 1

cur.lock            rd 1   ;1 - lock update, 2- hide
cur.left            rd 1   ;cursor clip box
cur.top             rd 1
cur.right           rd 1
cur.bottom          rd 1
cur.w               rd 1
cur.h               rd 1

ipc_tmp             rd 1
ipc_pdir            rd 1
ipc_ptab            rd 1

proc_mem_map        rd 1
proc_mem_pdir       rd 1
proc_mem_tab        rd 1

tmp_task_pdir       rd 1
tmp_task_ptab       rd 1

default_io_map      rd 1

LFBSize             rd 1

current_slot        rd 1

; status
hd1_status                      rd 1  ; 0 - free : other - pid
application_table_owner         rd 1  ; 0 - free : other - pid
application_table_mutex         MUTEX

; device addresses
mididp                          rd 1
midisp                          rd 1

cdbase                          rd 1
cdid                            rd 1

hdbase                          rd 1    ; for boot 0x1f0
hdid                            rd 1
hdpos                           rd 1    ; for boot 0x1
label   known_part dword
fat32part                       rd 1    ; for boot 0x1
cdpos                           rd 1

;CPUID information
cpu_vendor                      rd 3
cpu_sign                        rd 1
cpu_info                        rd 1
cpu_caps                        rd 4


pg_data                         PG_DATA
heap_test                       rd 1

buttontype                      rd 1
windowtypechanged               rd 1

hd_entries                      rd 1     ;unused ? 0xfe10

mouse_active                    rd 1
mouse_pause                     rd 1

redrawmouse_unconditional       rd 1

img_background                  rd 1
mem_BACKGROUND                  rd 1
static_background_data          rd 1

BgrDrawMode                     rd 1
BgrDataWidth                    rd 1
BgrDataHeight                   rd 1

skin_data                       rd 1

cache_ide0:
cache_ide0_pointer              rd 1
cache_ide0_size                 rd 1   ; not use
cache_ide0_data_pointer         rd 1
cache_ide0_system_data_size     rd 1   ; not use
cache_ide0_appl_data_size       rd 1   ; not use
cache_ide0_system_data          rd 1
cache_ide0_appl_data            rd 1
cache_ide0_system_sad_size      rd 1
cache_ide0_appl_sad_size        rd 1
cache_ide0_search_start         rd 1
cache_ide0_appl_search_start    rd 1

cache_ide1:
cache_ide1_pointer              rd 1
cache_ide1_size                 rd 1   ; not use
cache_ide1_data_pointer         rd 1
cache_ide1_system_data_size     rd 1   ; not use
cache_ide1_appl_data_size       rd 1   ; not use
cache_ide1_system_data          rd 1
cache_ide1_appl_data            rd 1
cache_ide1_system_sad_size      rd 1
cache_ide1_appl_sad_size        rd 1
cache_ide1_search_start         rd 1
cache_ide1_appl_search_start    rd 1

cache_ide2:
cache_ide2_pointer              rd 1
cache_ide2_size                 rd 1   ; not use
cache_ide2_data_pointer         rd 1
cache_ide2_system_data_size     rd 1   ; not use
cache_ide2_appl_data_size       rd 1   ; not use
cache_ide2_system_data          rd 1
cache_ide2_appl_data            rd 1
cache_ide2_system_sad_size      rd 1
cache_ide2_appl_sad_size        rd 1
cache_ide2_search_start         rd 1
cache_ide2_appl_search_start    rd 1

cache_ide3:
cache_ide3_pointer              rd 1
cache_ide3_size                 rd 1   ; not use
cache_ide3_data_pointer         rd 1
cache_ide3_system_data_size     rd 1   ; not use
cache_ide3_appl_data_size       rd 1   ; not use
cache_ide3_system_data          rd 1
cache_ide3_appl_data            rd 1
cache_ide3_system_sad_size      rd 1
cache_ide3_appl_sad_size        rd 1
cache_ide3_search_start         rd 1
cache_ide3_appl_search_start    rd 1

debug_step_pointer              rd 1

lba_read_enabled                rd 1  ; 0 = disabled , 1 = enabled
pci_access_enabled              rd 1  ; 0 = disabled , 1 = enabled

hdd_appl_data                   rb 1  ; 0 = system cache, 1 - application cache
cd_appl_data                    rb 1  ; 0 = system cache, 1 - application cache

timer_ticks_enable              rb 1  ; for cd driver

align 4
NumBiosDisks                    rd 1
BiosDisksData                   rb 200h
BiosDiskCaches                  rb 80h*(cache_ide1-cache_ide0)
BiosDiskPartitions              rd 80h

align 16
DRIVE_DATA:                     rb DRIVE_DATA_SIZE

IncludeUGlobals

uglobals_size = $ - endofcode

if ~ lang eq sp
diff16 "end of .bss",0,$
end if

org (OS_BASE+0x0100000)

RAMDISK:                        rb 2880*512
RAMDISK_FAT:                    rb 2856*2
FLOPPY_FAT:                     rb 2856*2

_CLEAN_ZONE:

align 4096
_IDE_DMA                        rb 16*512
BgrAuxTable                     rb 32768
BUTTON_INFO                     rb 64*1024
RESERVED_PORTS:                 rb 64*1024
FLOPPY_BUFF:                    rb 18*512           ;one track

sys_pgmap:                      rb 1024*1024/8
