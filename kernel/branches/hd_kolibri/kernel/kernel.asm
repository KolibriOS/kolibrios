include 'macros.inc'

$Revision: 435 $

include "proc32.inc"
include "kglobals.inc"
include "lang.inc"
include "const.inc"

max_processes      equ   255
tss_step           equ   (128+8192) ; tss & i/o - 65535 ports, * 256=557056*4

use16
                  org   0x0
                  jmp   start_of_code

org $+0x10000

db 0
dd servetable-0x10000
draw_line       dd __sys_draw_line
disable_mouse   dd __sys_disable_mouse
draw_pointer    dd __sys_draw_pointer
drawbar         dd __sys_drawbar.forced
putpixel        dd __sys_putpixel
version         db 'Kolibri OS  version 0.6.5.0      ',13,10,13,10,0

include "boot/preboot.inc"
include "kernel16.inc"
include "bootstrap/switch.inc"

use32

include 'unpacker.inc'

__DEBUG__ fix 1
__DEBUG_LEVEL__ fix 1

include 'fdo.inc'
include "bootstrap/bootmsg.inc"
include "bootstrap/bootlog.inc"
include "vars1.inc"
include "bootstrap/bootstrap.inc"
include "core/mainloop.inc"
include "core/idle.inc"
include "kernel32.inc"
include "hid/keymap.inc"
include "vars2.inc"

IncludeIGlobals
endofcode:
IncludeUGlobals
uglobals_size = $ - endofcode
diff16 "end of kernel code",0,$

__REV__ = __REV

diff10 "revision",0,__REV__
