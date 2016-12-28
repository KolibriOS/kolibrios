use32
    org 0x0
    db  'MENUET01'
    dd  0x01,start,i_end,e_end,e_end,0,0

__DEBUG__ = 1
__DEBUG_LEVEL__ = 2

LOG_FLOW equ 1
LOG_CALC equ 3
LOG_SEND equ 

include '../../../macros.inc'
include '../../../debug-fdo.inc'

start:
        DEBUGFG 1, LOG_FLOW, '1 flow\n'
        DEBUGFG 2, LOG_FLOW, '2 flow\n'
        DEBUGFG 3, LOG_FLOW, '3 flow\n'

        DEBUGFG 1, LOG_CALC, '1 calc\n'
        DEBUGFG 2, LOG_CALC, '2 calc\n'
        DEBUGFG 3, LOG_CALC, '3 calc\n'

        DEBUGFG 1, LOG_SEND, '1 send\n'
        DEBUGFG 2, LOG_SEND, '2 send\n'
        DEBUGFG 3, LOG_SEND, '3 send\n'

        DEBUGF 1, '1 blah\n'
        DEBUGF 2, '2 blah\n'
        DEBUGF 3, '3 blah\n'

        mov     eax, -1
        int     0x40


include_debug_strings 

i_end:
rb 0x100        ;stack
e_end:
