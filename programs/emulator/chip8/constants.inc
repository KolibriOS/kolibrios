; constants for formatted debug
__DEBUG__       = 1             ; 0 - disable debug output / 1 - enable debug output
__DEBUG_LEVEL__ = DBG_ERR       ; set the debug level
 
DBG_ALL       = 0  ; all messages
DBG_INFO      = 1  ; info and errors
DBG_ERR       = 2  ; only errors

; emulator constants
MAX_GAME_SIZE = 0x1000 - 0x200
FONTSET_ADDRESS = 0x00
FONTSET_BYTES_PER_CHAR = 5
MEM_SIZE = 4096
STACK_SIZE = 16
KEY_SIZE = 16
GFX_ROWS = 32
GFX_COLS = 64
GFX_SIZE = GFX_ROWS * GFX_COLS
GFX_PIX_SIZE = 10
CLOCK_RATE = 1 ; in 10^-2 seconds


include '../../macros.inc'
purge   mov, add, sub

include '../../debug-fdo.inc'
include '../../proc32.inc'

;=========================================