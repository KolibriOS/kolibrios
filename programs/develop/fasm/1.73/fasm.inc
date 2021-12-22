ID_CLOSE_BTN	  = 1
ID_COMPILE_BTN	  = 2
ID_EXECUTE_BTN	  = 3
ID_EXECDBG_BTN	  = 4
ID_OPENDLG_BTN	  = 5

NORMAL_MODE    = 8
CONSOLE_MODE   = 32
MAGIC1	       = 6*(text.line_size-1)+14
MAGIC2	       = (14*3+16)/3+1

APP_MEMORY     = 0x00800000

DEFAULT_WIN_W = 450
DEFAULT_WIN_H = 350
WIN_MIN_W     = 400
WIN_MIN_H     = 350
LINE_H = 25
RIGHT_BTN_W = 80

MAX_PATH       = 100

macro mmov reg,a1,a2 {
 mov reg,(a1) shl 16 + (a2)
}

macro madd reg,a1,a2 {
 add reg,(a1) shl 16 + (a2)
}

macro msub reg,a1,a2 {
 sub reg,(a1) shl 16 + (a2)
}

struc path name:?& {
        .:
	db name
	rb MAX_PATH-$+. }
