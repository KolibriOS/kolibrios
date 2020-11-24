;//NAME//      life2.cpp
;//COMPILER//  bcc32 -S -v- -R- -6 -a4 -O2 -Og -Oi -Ov -OS -k- -D__KOLIBRI__ -Iinclude
;//UTIL_PATH// .

STACKSIZE   equ 8196
HEAPSIZE    equ 0

include "include\kos_start.inc"
include "include\kos_func.inc"
include "include\kos_heap.inc"
include "mmxlife.inc"
;include "sse2life.inc"
