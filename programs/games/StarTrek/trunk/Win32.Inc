; -----------------------------------------------------------
; Win32 API definitions
; -----------------------------------------------------------

; -----------------------------------------------------------
; Type Alias Section
; -----------------------------------------------------------

INT32 equ dd
PINT32 equ dd
INT16 equ dw
SHORT equ dw
WNDPROC equ dd
COUNT equ dd
UINT32 equ dd
UINT8 equ db
WORD equ dw
BYTE equ db
CHAR equ db
PBYTE equ dd
NBYTES equ dd
HICON equ dd
HBRUSH equ dd
HBITMAP equ dd
HCURSOR equ dd
HINSTANCE equ dd
PWCHAR equ dd
PCHAR equ dd
PPCHAR equ dd
PDOUBLE equ dd
PPWCHAR equ dd
WCHARS equ rw
CHARS equ rb
HWND equ dd
DOUBLE equ dt
DOUBLES equ rt
HANDLE equ dd
HFILE equ dd
HDC equ dd
HFONT equ dd
BOOL equ dd
PBOOL equ dd
LONG equ dd
WPARAM equ dd
LPARAM equ dd
INDEX equ dd
COLORREF equ dd
PSIZE equ dd
PVOID equ dd
PCOORD equ dd
FUNCTOR equ dd
PDEVICE equ dd
STRINGS equ rd
INTEGERS equ rd
POINTERS equ rd
COLORREFS equ rd
CHARS equ rb
BYTES equ rb

; -----------------------------------------------------------
; Pointers to classes
; -----------------------------------------------------------

PTLine equ dd
PTOutputCell equ dd
PTCmdBuf equ dd
PTRandom equ dd
PTPalette equ dd
PTGameState equ dd
PTPlanet equ dd
PTKlingon equ dd
PTCharAttr equ dd
PTLineBuf equ dd
PTConsole equ dd
PTCharacter equ dd

; -----------------------------------------------------------
; Constants
; -----------------------------------------------------------

HEAP_NO_SERIALIZE = 1
DEFAULT_CHARSET = 1
LF_FACESIZE = 32
IDC_ARROW = 32512
IDI_EXCLAMATION = 32515
SM_CXSCREEN = 0
SM_CYSCREEN = 1
WS_EX_APPWINDOW = 40000h
WS_THICKFRAME = 40000h
WS_SYSMENU = 80000h
WS_MINIMIZEBOX = 20000h
WS_CAPTION = 0C00000h
WS_OVERLAPPEDWND = WS_CAPTION or WS_SYSMENU or WS_THICKFRAME or WS_MINIMIZEBOX
WS_NOSIZEWND = WS_CAPTION or WS_SYSMENU or WS_MINIMIZEBOX
WM_CREATE = 1
WM_DESTROY = 2
WM_PAINT = 0Fh
WM_CLOSE = 10h
WM_ERASEBKGND = 14h
WM_CHAR = 102h
WM_APP = 8000h
MEM_COMMIT = 1000h
MEM_RESERVE = 2000h
MEM_RELEASE = 8000h
PAGE_READWRITE = 4
SW_SHOWNORMAL = 1
SM_CYCAPTION = 4
SM_CXBORDER = 5
SM_CYBORDER = 6
LOGPIXELSY = 90
FW_NORMAL = 400
CHAR_BACKSPACE = 8
CHAR_ENTER = 0Dh
CHAR_ESC = 1Bh
CHAR_BLANK = 20h
CHAR_COSMOS = 0B7h
;CHAR_COSMOS = '.'
CHAR_ASTERISK = '*'
CHAR_ENTERPRISE = 'E'
CHAR_FQUEENE = 'F'
CHAR_ZERO = '0'
CHAR_PLANET = 'P'
CHAR_KLINGON = 'K'
CHAR_ROMULAN = 'R'
CHAR_SCOM = 'S'
CHAR_THOLIAN = 'T'
CHAR_WEB = '#'
CHAR_THING = '?'
CHAR_STARBASE = 'B'
CHAR_BLACK_HOLE = '@'
CHAR_COMMA = ','
CHAR_COMMANDER = 'C'
SRCCOPY = 0CC0020h
FIXED_PITCH = 1
TRANSPARENT = 1
TA_UPDATECP = 1
WHITE_BRUSH = 0
BLACK_BRUSH = 4
WM_CUSTOM_CMD_READY = (WM_APP + 1)
WM_CUSTOM_DO_PRELIM = (WM_APP + 2)
WM_CUSTOM_NEW_GAME = (WM_APP + 3)
GENERIC_READ = 80000000h
GENERIC_WRITE = 40000000h
CREATE_ALWAYS = 2
OPEN_EXISTING = 3
FILE_FLAG_SEQ_SCAN = 08000000h
FILE_ATTR_DIR = 10h

; -----------------------------------------------------------
; Structure Definitions
; -----------------------------------------------------------

virtual at 0
SIZE:
    .size_Width INT32 ?
    .size_Height INT32 ?
    .size = $
end virtual

virtual at 0
RECT:
    .rc_Left INT32 ?
    .rc_Top INT32 ?
    .rc_Right INT32 ?
    .rc_Bottom INT32 ?
    .size = $
end virtual

virtual at 0
MSG:
    .msg_hWnd HWND ?
    .msg_uiMsg UINT32 ?
    .msg_wParam WPARAM ?
    .msg_lParam LPARAM ?
    .msg_uiTime UINT32 ?
    .msg_xCoord INT32 ?
    .msg_yCoord INT32 ?
    .size = $
end virtual

virtual at 0
LOGFONT:
    .lf_Height LONG ?
    .lf_Width LONG ?
    .lf_Escapement LONG ?
    .lf_Orientation LONG ?
    .lf_Weight LONG ?
    .lf_Italic UINT8 ?
    .lf_Underline UINT8 ?
    .lf_StrikeOut UINT8 ?
    .lf_CharSet UINT8 ?
    .lf_OutPrecision UINT8 ?
    .lf_ClipPrecision UINT8 ?
    .lf_Quality UINT8 ?
    .lf_PitchAndFamily UINT8 ?
    .lf_FaceName WCHARS LF_FACESIZE
    .size = $
end virtual

virtual at 0
WIN32_FIND_DATA:
    .dwAttr UINT32 ?
    ._pad1 rd 10
    .strName WCHARS 260
    .strName2 WCHARS 14
    .size = $
end virtual

virtual at 0
SYSTIME:
    .wYear WORD ?
    .wMonth WORD ?
    .wDayOfWeek WORD ?
    .wDay WORD ?
    .wHour WORD ?
    .wMinute WORD ?
    .wSecond WORD ?
    .wMillisecond WORD ?
    .size = $
end virtual

virtual at 0
PAINTSTRUCT:
    .hDC HDC ?
    .bErase BOOL ?
    .rcPaint rb RECT.size
    .bRestore BOOL ?
    .bIncUpdate BOOL ?
    .rgbReserved rb 32
    .size = $
end virtual

virtual at 0
LOCATION:
    .coord_X BYTE ?
    .coord_Y BYTE ?
    .size = $
end virtual

virtual at 0
WNDCLASS:
    .wc_style UINT32 ?
    .wc_lpfnWndProc WNDPROC ?
    .wc_cbClsExtra NBYTES ?
    .wc_cbWndExtra NBYTES ?
    .wc_hInstance HINSTANCE ?
    .wc_hIcon HICON ?
    .wc_hCursor HCURSOR ?
    .wc_hbrBackground HBRUSH ?
    .wc_lpszMenuName PWCHAR ?
    .wc_lpszClassName PWCHAR ?
    .size = $
end virtual

;<EOF>
