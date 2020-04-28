; events
EV_IDLE        = 0
EV_TIMER       = 0
EV_REDRAW      = 1
EV_KEY	       = 2
EV_BUTTON      = 3
EV_EXIT        = 4
EV_BACKGROUND  = 5
EV_MOUSE       = 6
EV_IPC	       = 7
EV_STACK       = 8

; event mask bits for function 40
EVM_REDRAW     =	  1b
EVM_KEY        =	 10b
EVM_BUTTON     =	100b
EVM_EXIT       =       1000b
EVM_BACKGROUND =      10000b
EVM_MOUSE      =     100000b
EVM_IPC        =    1000000b
EVM_STACK      =   10000000b
EVM_DEBUG      =  100000000b
EVM_STACK2     = 1000000000b

EVM_MOUSE_FILTER  = 0x80000000
EVM_CURSOR_FILTER = 0x40000000

; create window constants
CW_CAPTION	       = $10000000
CW_CLIENTRECTCOORDS    = $20000000
CW_TRANSPARENT	       = $40000000
CW_GRADIENTFILL        = $80000000
CW_DONTDRAW	       = $01000000
CW_SKINED	       = $03000000
CW_SKINEDFIXED	       = $04000000
CW_UNMOVABLE	       = $01000000

; colors
CLR_BLACK	       = $00000000