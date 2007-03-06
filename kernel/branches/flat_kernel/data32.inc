
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

new_process_loading db 'K : New Process - loading',13,10,0
new_process_running db 'K : New Process - done',13,10,0
start_not_enough_memory db 'K : New Process - not enough memory',13,10,0

msg_unresolved db 'unresolved ',0
msg_module     db 'in module ',0
msg_version    db 'incompatible driver version',13,10,0
msg_www        db 'please visit www.kolibrios.org',13,10,0
msg_CR         db  13,10,0

intel_str      db "GenuineIntel",0
AMD_str        db "AuthenticAMD",0

;szSound        db 'SOUND',0
;szInfinity     db 'INFINITY',0
szHwMouse      db 'ATI2D',0

szSTART        db 'START',0
szEXPORTS      db 'EXPORTS',0
szIMPORTS      db 'IMPORTS',0

firstapp       db  '/rd/1/LAUNCHER',0
char           db  'FONTS/CHAR.MT',0
char2          db  'FONTS/CHAR2.MT',0
bootpath       db  '/KOLIBRI    '
bootpath2      db  0
vmode          db  'drivers/VMODE.MDR',0
vrr_m          db  '/rd/1/VRR_M',0
kernel_file    db  'KERNEL  MNT'


; mike.dld {
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


align 4
keyboard   dd 1
sound_dma  dd 1
syslang    dd 1

boot_y     dd 10


if __DEBUG__ eq 1
  include_debug_strings
end if

IncludeIGlobals

align 16
gdts:

        dw     gdte-$-1
        dd     gdts
        dw     0

; Attention! The order first four selectors not to change, is used in Fast System Call
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
        dw G32+D32+0x8000+0x7;

app_data_l:
        dw 0xFFFF
        dw 0
        db 0
        db drw3
        dw G32+D32+0x8000+0x7;

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

endofcode:
tss0_l:
        rq (max_processes+10)

gdte:

align 16
cur_saved_data    rb 4096
fpu_data:         rb 512

; device irq owners
irq_owner         rd 16 ; process id

; on irq read ports

irq00read         rd 16
irq01read         rd 16
irq02read         rd 16
irq03read         rd 16
irq04read         rd 16
irq05read         rd 16
irq06read         rd 16
irq07read         rd 16
irq08read         rd 16
irq09read         rd 16
irq10read         rd 16
irq11read         rd 16
irq12read         rd 16
irq13read         rd 16
irq14read         rd 16
irq15read         rd 16

irq_tab           rd 16

mem_block_map     rb 512
event_map         rb 64
mem_block_list    rd 64
mem_block_mask    rd 2

mem_used.fd       rd 1
mem_used.bk       rd 1

mem_block_arr     rd 1
mem_block_start   rd 1
mem_block_end     rd 1

heap_mutex        rd 1
heap_size         rd 1
heap_free         rd 1
heap_blocks       rd 1
free_blocks       rd 1

mst MEM_STATE

page_start        rd 1
page_end          rd 1
events            rd 1
event_start       rd 1
event_end         rd 1
event_uid         rd 1
sys_page_map      rd 1
os_stack          rd 1

srv.fd            rd 1
srv.bk            rd 1

scr_width         rd 1
scr_height        rd 1

create_cursor     rd 1
set_hw_cursor     rd 1
hw_restore        rd 1

def_cursor        rd 1
hw_cursor         rd 1
cur_def_interl    rd 1
cur_saved_base    rd 1
cur_saved_interl  rd 1
cur_saved_w       rd 1
cur_saved_h       rd 1

ipc_tmp           rd 1
ipc_pdir          rd 1
ipc_ptab          rd 1

proc_mem_map      rd 1
proc_mem_pdir     rd 1
proc_mem_tab      rd 1

tmp_task_pdir     rd 1
tmp_task_ptab     rd 1

LFBSize           rd 1

stall_mcs         rd 1

; status
hd1_status                  rd 1  ; 0 - free : other - pid
application_table_status    rd 1  ; 0 - free : other - pid

; device addresses
mididp            rd 1
midisp            rd 1

cdbase            rd 1
cdid              rd 1

hdbase            rd 1    ; for boot 0x1f0
hdid              rd 1
hdpos             rd 1    ; for boot 0x1
fat32part         rd 1    ; for boot 0x1

sb16              rd 1

;CPUID information
cpu_vendor        rd 3
cpu_sign          rd 1
cpu_info          rd 1
cpu_caps          rd 4


pg_data           PG_DATA
heap_test         rd 1

buttontype        rd 1
windowtypechanged rd 1

hd_entries        rd 1     ;unused ? 0xfe10

;* start code - Mario79
mouse_pause           rd 1
MouseTickCounter      rd 1
ps2_mouse_detected    rb 1
com1_mouse_detected   rb 1
com2_mouse_detected   rb 1
;* end code - Mario79

wraw_bacground_select rb 1

lba_read_enabled      rd 1  ; 0 = disabled , 1 = enabled
pci_access_enabled    rd 1  ; 0 = disabled , 1 = enabled

IncludeUGlobals

