;//NAME//      life2.cpp
;//COMPILER//  bcc32 -S -v- -R- -6 -a4 -O2 -Og -Oi -Ov -OS -k- -D__MENUET__ -Iinclude
;//UTIL_PATH// .

STACKSIZE   equ 102400
HEAPSIZE    equ 102400

include "include\me_start.inc"
include "include\me_func.inc"
include "include\me_heap.inc"
include "me_cdlg.inc"
include "mmxlife.inc"
;include "sse2life.inc"
