align 4
B32:
include "clear.inc"
include "rmode.inc"
include "graphaddr.inc"
include "sysenter.inc"
           call mem_test
           call init_mtrr
           call init_mem
           call init_page_map
include "paging.inc"
include "graphbase.inc"

        call   build_scheduler ; sys32.inc
; LOAD IDT
        lidt   [cs:idtreg]
        cli

include 'detect/disks.inc'
;include 'rdload.inc'
        call  calculatefatchain
include 'vmodeld.inc'
include "loadfont.inc"
include "printmem.inc"
include "redirirq.inc"
include "timer.inc"
include "setmouse.inc"
include "windef.inc"
include "setbgr.inc"
include "resirq.inc"
include "setports.inc"
include "ostask.inc"
        call init_cursors
include "tsc.inc"
        call  set_variables
        call  stack_init
        call  fdc_init
include "palette.Inc"
include "loadskn.inc"
include "firstapp.inc"
include "kbset.inc"
include "tasking.Inc"
include "allirqs.inc"
sti
jmp   $                      ; wait here for timer to take control




