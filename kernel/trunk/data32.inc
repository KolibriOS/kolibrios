;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2022. All rights reserved. ;;
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
  boot_initirq      cp866 'Инициализация IRQ',0
  boot_picinit      cp866 'Инициализация PIC',0
  boot_v86machine   cp866 'Инициализация системной V86 машины',0
  boot_inittimer    cp866 'Инициализация системного таймера (IRQ0)',0
  boot_initapic     cp866 'Попытка инициализации APIC',0
  boot_enableirq    cp866 'Включить прерывания 2, 13',0
  boot_disabling_ide cp866 'Запрещение прерываний в контроллере IDE',0
  boot_enabling_ide cp866 'Разрешение прерываний в контроллере IDE',0
  boot_set_int_IDE  cp866 'Установка обработчиков прерываний IDE',0
  boot_detectfloppy cp866 'Поиск floppy дисководов',0
  boot_detecthdcd   cp866 'Поиск жестких дисков и ATAPI приводов',0
  boot_getcache     cp866 'Получение памяти для кэша',0
  boot_detectpart   cp866 'Поиск разделов на дисковых устройствах',0
  boot_init_sys     cp866 'Инициализация системного каталога /sys',0
  boot_loadlibs     cp866 'Загрузка библиотек (.obj)',0
  boot_memdetect    cp866 'Количество оперативной памяти','     ',' Мб',0
  boot_tss          cp866 'Установка TSSs',0
  boot_cpuid        cp866 'Чтение CPUIDs',0
;  boot_devices      cp866 'Поиск устройств',0
  boot_timer        cp866 'Установка таймера',0
  boot_initramdisk  cp866 'Инициализация рамдиска',0
  boot_irqs         cp866 'Переопределение IRQ',0
  boot_setmouse     cp866 'Установка мыши',0
  boot_windefs      cp866 'Установка настроек окон по умолчанию',0
  boot_bgr          cp866 'Установка фона',0
  boot_resirqports  cp866 'Резервирование IRQ и портов',0
  boot_setrports    cp866 'Установка адресов IRQ',0
  boot_setostask    cp866 'Создание процесса ядра',0
  boot_allirqs      cp866 'Открытие всех IRQ',0
  boot_tsc          cp866 'Чтение TSC',0
  boot_cpufreq      cp866 'Частота процессора ','    ',' МГц',0
  boot_pal_ega      cp866 'Установка EGA/CGA 320x200 палитры',0
  boot_pal_vga      cp866 'Установка VGA 640x480 палитры',0
  boot_failed       cp866 'Загрузка первого приложения не удалась',0
  boot_mtrr         cp866 'Установка MTRR',0

  boot_APIC_found   cp866 'APIC включен', 0
  boot_APIC_nfound  cp866 'APIC не найден', 0
if preboot_blogesc
  boot_tasking      cp866 'Все готово для запуска, нажмитре ESC для старта',0
end if
else if lang eq sp
  include 'data32sp.inc'
else if lang eq et
  include 'data32et.inc'
else
  boot_initirq      db   'Initialize IRQ',0
  boot_picinit      db   'Initialize PIC',0
  boot_v86machine   db   'Initialize system V86 machine',0
  boot_inittimer    db   'Initialize system timer (IRQ0)',0
  boot_initramdisk  db   'Initialize ramdisk',0
  boot_initapic     db   'Try to initialize APIC',0
  boot_enableirq    db   'Enable interrupts 2, 13',0
  boot_disabling_ide db   'Disable interrupts in IDE controller',0
  boot_enabling_ide db   'Enable interrupts in IDE controller',0
  boot_set_int_IDE  db   'Set handler of interrupts for IDE',0
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
;msg_module     db 'in module ',0
;if ~ lang eq sp
;msg_version    db 'incompatible driver version',13,10,0
;msg_www        db 'please visit www.kolibrios.org',13,10,0
;end if
msg_CR         db  13,10,0

szPS2MDriver    db '/sys/drivers/PS2MOUSE.SYS',0
;szCOM_MDriver   db 'COM_MOUSE',0
szVidintel      db '/sys/drivers/vidintel.sys',0
szUSB           db 'USB',0

szEXPORTS      db 'EXPORTS',0
sz_EXPORTS     db '_EXPORTS',0

szIMPORTS      db 'IMPORTS',0

read_firstapp  db '/sys/'
firstapp       db '/sys/LAUNCHER',0
notifyapp      db '/sys/@notify',0
if lang eq ru
ud_user_message  cp866 'Ошибка: неподдерживаемая инструкция процессора',0
mtrr_user_message cp866 '"Обнаружена проблема с конфигурацией MTRR.\nПроизводительность может быть пониженной" -dW',0
else if ~ lang eq sp
ud_user_message db 'Error: unsupported processor instruction',0
mtrr_user_message db '"There is a problem with MTRR configuration.\nPerformance can be low" -dW',0
end if

kernel_file_load:
; load kernel.mnt to _CLEAN_ZONE
        dd      0       ; subfunction
        dq      0       ; offset in file
        dd      0x31000 ; number of bytes to read
        dd      _CLEAN_ZONE ; buffer for data
        db      '/sys/KERNEL.MNT',0

dev_data_path  db '/RD/1/DRIVERS/DEVICES.DAT',0
; { Patch by Coldy, For DLL autoload
dll_lib_path  db '/RD/1/LIB/DLL.OBJ',0
dll_error_msg db '"DLL.OBJ not found!\nTerminate application!" -dE',0   
; } End patch by Coldy, For DLL autoload
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


align 4
keyboard   dd 1

if lang eq en
  SYSLANG = 1
else if lang eq fi
  SYSLANG = 2
else if lang eq de
  SYSLANG = 3
else if lang eq ru
  SYSLANG = 4
else if lang eq fr
  SYSLANG = 5
else if lang eq et
  SYSLANG = 6
else if lang eq sp
  SYSLANG = 7
else if lang eq it
  SYSLANG = 8
else if lang eq ca
  SYSLANG = 9
else
  display 'unsupported language specified',13,10
end if
syslang    dd SYSLANG

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

gdte:

diff16 "end of .data segment",0,$
endofcode:

align 16
cur_saved_data:
        rb  4096
align 64
fpu_data:
        rb  0xa80       ; bochs avx512
fpu_data_size = $ - fpu_data
draw_data:
        rb  32*256
BPSLine_calc_area   rd  MAX_SCREEN_HEIGHT
d_width_calc_area   rd  MAX_SCREEN_HEIGHT
mouseunder          rd  16*24

mem_block_list      rd  64*2
mem_used_list       rd  64*2
mem_hash_cnt        rd  64

thr_slot_map        rd  8

_display            display_t
bios_fb             FRB

mst                 MEM_STATE

cpu_freq            dq  ?

heap_mutex          MUTEX
heap_size           dd  ?
heap_free           dd  ?
heap_blocks         dd  ?
free_blocks         dd  ?
mem_block_mask      rd  2
next_memblock       dd  ?

pte_valid_mask      dd  ?
page_start          dd  ?
page_end            dd  ?
sys_page_map        dd  ?
os_stack_seg        dd  ?

srv.fd              dd  ?
srv.bk              dd  ?

LFBAddress          dd  ?

PUTPIXEL            dd  ?
GETPIXEL            dd  ?

if VESA_1_2_VIDEO
BANK_SWITCH         dd  ?   ; reserved for vesa 1.2
BANK_RW             dd  ?
end if

MOUSE_PICTURE       dd  ?

def_cursor          dd  ?
def_cursor_clock    dd  ?
current_cursor      dd  ?
hw_cursor           dd  ?
cur_saved_base      dd  ?

cur.lock            dd  ?   ; 1 - lock update, 2- hide
cur.left            dd  ?   ; cursor clip box
cur.top             dd  ?
cur.w               dd  ?
cur.h               dd  ?

ipc_tmp             dd  ?
ipc_pdir            dd  ?
ipc_ptab            dd  ?

proc_mem_map        dd  ?
proc_mem_pdir       dd  ?
proc_mem_tab        dd  ?

tmp_task_ptab       dd  ?

default_io_map      dd  ?

LFBSize             dd  ?

current_process     dd  ?
current_slot        dd  ?   ; pointer to APPDATA of current thread
current_slot_idx    dd  ?   ; index of current thread slot
thread_count        dd  ?

; device addresses
mididp  dd  ?
midisp  dd  ?

cdbase  dd  ?
cdid    dd  ?

hdbase  dd  ?   ; for boot 0x1f0
hdid    dd  ?
hdpos   dd  ?   ; for boot 0x1
cdpos   dd  ?

;CPUID information
cpu_vendor  rd  3
cpu_sign    dd  ?
cpu_info    dd  ?
cpu_caps    rd  4

xsave_area_size dd ?
xsave_eax       dd ?
xsave_edx       dd ?

pg_data     PG_DATA
heap_test   dd  ?

skin_data   dd  ?

mouse_active    dd  ?
mouse_pause     dd  ?

BgrDrawMode     dd  ?
BgrDataWidth    dd  ?
BgrDataHeight   dd  ?

buttontype          dd  ?
windowtypechanged   dd  ?

debug_step_pointer  dd  ?

lba_read_enabled    dd  ?   ; 0 = disabled , 1 = enabled
pci_access_enabled  dd  ?   ; 0 = disabled , 1 = enabled

NumBiosDisks        dd  ?
BiosDisksData       rb  200h    ; struct BiosDiskData
BiosDiskCaches      rb  80h*(cache_ide1-cache_ide0)
BiosDiskPartitions  rd  80h

img_background          dd  ?
mem_BACKGROUND          dd  ?
static_background_data  dd  ?

hd1_status              dd  ?   ; 0 - free : other - pid
application_table_owner dd  ?   ; 0 - free : other - pid
application_table_mutex MUTEX

redrawmouse_unconditional   dd  ?

MOUSE_SCROLL_H  rw  1
MOUSE_X:        rw  1
MOUSE_Y:        rw  1
MOUSE_SCROLL_V  rw  1

X_UNDER         rw  1
Y_UNDER         rw  1
COLOR_TEMP      dd  ?
MOUSE_COLOR_MEM dd  ?

SCR_MODE    rw  2

BTN_DOWN:   rb  4

cpu_phys_addr_width db  ?   ; also known as MAXPHYADDR in Intel manuals
hdd_appl_data       db  ?   ; 0 = system cache, 1 - application cache
cd_appl_data        db  ?   ; 0 = system cache, 1 - application cache

timer_ticks_enable  db  ?  ; for cd driver

REDRAW_BACKGROUND   db  ?

align 16
DRIVE_DATA: rb  DRIVE_DATA_SIZE

IncludeUGlobals

uglobals_size = $ - endofcode

if ~ lang eq sp
diff16 "end of .bss",0,$
end if

; check if kernel fits memmap
assert $-OS_BASE+0x1000 < TMP_STACK_TOP

org (OS_BASE+0x0100000)

; Currently size of memory allocated for the ramdisk is fixed.
; This should be revisited when/if memory map would become more dynamic.
RAMDISK_CAPACITY = 2880 ; in sectors

RAMDISK:
        rb  RAMDISK_CAPACITY*512

_CLEAN_ZONE:
CLEAN_ZONE = _CLEAN_ZONE - OS_BASE

BgrAuxTable     rb  32768
align 65536
SB16Buffer      rb  65536

align 4096
BUTTON_INFO     rb  64*1024
RESERVED_PORTS:
        rb  64*1024
sys_pgmap:
        rb  1024*1024/8
