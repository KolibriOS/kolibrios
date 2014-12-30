; $$$$$$$$$$$$$$$$$$$ ABAKIS $$$$$$$$$$$$$$$$$$$$$
; *************** STAR^2 SOFTWARE ****************
; ????????????????? SYSTEM.INC ???????????????????

;;;;;;;;;;;;;;;;;;;;; IMPORT ;;;;;;;;;;;;;;;;;;;;;

; RVAs of dll names and tables, ending with 20
; zero bytes

macro library [names] {
 forward dd 0,0,0,\
  RVA names#_name, RVA names#_table
 common dd 0,0,0,0,0
}

; DLL name + import RVA table. each table ends with 0.
; finally, import names. dw 0 is "ordinal" (N/A)

macro import name, [names] {
 common
  name#_name \		  ; text DLL_name='DLL.DLL'
  db `name#'.DLL', 0
  name#_table:		  ; DLL_table:
 forward
  IF used !#names
   !#names dd RVA _#names ; import name RVAs
   macro names [p] \{	  ; call with no
    \common		  ; invoke prefix
     pushr p
     call [!#names]
   \}
  END IF
 common dd 0		  ; end
 forward
  IF used !#names
   _#names dw 0 	  ; import names
   db `names, 0 	  ; 'import'
  END IF
}

;;;;;;;;;;;;;;;;;;;; IMPORTS ;;;;;;;;;;;;;;;;;;;;;

data import

library MSVCRT, KERNEL32, USER32, SHELL32,\
 GDI32, COMDLG32

import MSVCRT, sprintf

import KERNEL32,\
ExitProcess, GetCommandLineA,\
HeapCreate, HeapAlloc, HeapReAlloc, HeapSize,\
HeapFree, HeapDestroy, VirtualAlloc, VirtualFree,\
GetModuleHandleA, GetModuleFileNameA,\
CreateFileA, GetFileSize, ReadFile, WriteFile,\
SetFilePointer, CloseHandle, CopyFileA,\
MoveFileA, DeleteFileA, GetTickCount,\
GetSystemTime, GetLocalTime, GetFileTime,\
FileTimeToSystemTime, SystemTimeToFileTime,\
FileTimeToLocalFileTime,\
SystemTimeToTzSpecificLocalTime,\
GetFileAttributesExA, CompareFileTimeA,\
GetCurrentDirectoryA, SetCurrentDirectoryA,\
CreateDirectoryA, LoadLibraryA, FreeLibrary,\
GetProcAddress, FindFirstFileA, FindNextFileA,\
FindClose, WaitForSingleObject, Sleep

import USER32,\
GetDC, ReleaseDC,\
MessageBoxA, RegisterClassExA, CreateWindowExA,\
DestroyWindow, ShowWindow, MoveWindow,\
UpdateWindow, GetMessageA, PeekMessageA,\
TranslateMessage, DispatchMessageA,\
SendMessageA, DefWindowProcA, PostQuitMessage,\
WaitMessage, GetAsyncKeyState, LoadImageA,\
LoadIconA, LoadCursorA, SetCursor, ShowCursor,\
SetCursorPos, OpenClipboard, SetClipboardData,\
IsClipboardFormatAvailable, GetClipboardData,\
CloseClipboard, EmptyClipboard,\
GetSystemMetrics, BeginPaint, EndPaint,\
FillRect, InvalidateRect, SetTimer

import SHELL32, ShellExecuteA, ShellExecuteExA

import GDI32, SelectObject, DeleteObject,\
GetObjectA, DeleteDC, TextOutA, CreateFontA,\
CreateFontIndirectA, SetDIBits, BitBlt, StretchBlt,\
CreateBitmap, CreateCompatibleDC

import COMDLG32, GetOpenFileNameA,\
GetSaveFileNameA, ChooseColorA, ChooseFontA

END data

;;;;;;;;;;;;;;;;;;;;; SYSTEM ;;;;;;;;;;;;;;;;;;;;;

align

void @module, @heap

void directory, file.name, command.line

?t equ directory

;;;;;;;;;;;;;;;;;;;; MINIMAL ;;;;;;;;;;;;;;;;;;;;;

; say t     - display message
; say t, m  - title and message
; say.n n   - number

; examples:

; say 'Hi'
; say name
; say.n 123

macro os.say t, m { MessageBoxA 0, m, t, 0 }

macro os.ask q, t { MessageBoxA, 0, q, t, 3 }

function _say, t, m
  os.say t, m
endf

macro say a, b {
  pusha
  IF a eqtype ''
    make.txt r0, a
  ELSE
    . r0=a
  END IF
  IF b eq
    _say r0, r0
  ELSE
    IF b eqtype ''
      make.txt r2, b
    ELSE
      . r2=b
    END IF
    _say r2, r0
  END IF
  popa
}

function say.n, n
  locale t(32)
  pusha
  . r1=&t
  u2t n, r1
  . r1=&t
  _say r1, r1
  popa
endf

function say.h, n
  locale t(32)
  pusha
  . r1=&t
  h2t n, r1
  . r1=&t
  _say r1, r1
  popa
endf

function say.b, n
  locale t(32)
  pusha
  . r1=&t
  b2t n, r1
  . r1=&t
  _say r1, r1
  popa
endf

macro sayz t {
 say ?LITERALS+?literals.i
 ?literal t
}

macro ask q, t { os.ask q, t }

macro cinvoke proc,[arg]
 { common
    size@ccall = 0
    IF ~ arg eq
   reverse
    pushd arg
    size@ccall = size@ccall+4
   common
    END IF
    call [proc]
    IF size@ccall
     add esp,size@ccall
    END IF }

macro sprintf t, f, [p] {
 common
  cinvoke !sprintf, t, f, p
}

;;;;;;;;;;;;;;;;;;;;; DEBUG ;;;;;;;;;;;;;;;;;;;;;;

bug.t db 'BUG', 0

macro bug { say bug.t }

macro bug.x t {
 log t
 execute log.file
 exit
}

macro BUG { db 0CCh } ; int3 breakpoint

;;;;;;;;;;;;;;;;;;;; MEMORY ;;;;;;;;;;;;;;;;;;;;;;

macro os.memory
 { get @heap=HeapCreate 0, 0, 0 }

macro os.allocate n { HeapAlloc @heap, 0, n }

macro os.reallocate p, n
 { HeapReAlloc @heap, 0, p, n }

macro os.destroy p { HeapFree @heap, 0, p }

;;;;;;;;;;;;;;; ALLOCATE, DESTROY ;;;;;;;;;;;;;;;;

; allocate n
; allocate.p &p, n
; destroy &p

; example: try p=allocate 4*KB

function allocate, n
  os.allocate n
endf

function allocate.p, p, n
  if p=0
    allocate n
    return
  end
  os.reallocate p, n
endf

function destroy, p
  if p
    os.destroy p
  end
endf

macro destroy [p] { forward destroy p }

;;;;;;;;;;;;;;;;;;;;; TIME ;;;;;;;;;;;;;;;;;;;;;;;

FILE.TIME fix u64

macro os.get.time {
 GetLocalTime local.time
 update.time local.time
}

function os.delay, ms
  locals start
  get start=GetTickCount
  @@:
   GetTickCount
   . r1=start, r1+ms
   cmp r0, r1
  jb @b
endf

get.clock fix GetTickCount

;;;;;;;;;;;;;;;;;;;;; RANDOM ;;;;;;;;;;;;;;;;;;;;;

align integer @seed

; generate unique random number: 0-n

; seed=(seed*343FDh)+269EC3h
; seed=((seed>>16)&7FFFh)/(n+1)

function random, n
  . r0=@seed
  if false	 ; initialize seed
    rdtsc	 ; read date/time stamp counter
    . @seed=r0
  end
  . r0*343FDh, r0+269EC3h,\
  @seed=r0, r0>>16, r0&7FFFh,\
  r1=n, r1+1, r0/r1, r0=r2
endf

; random(from-to-2)+from

function random.x, from, to
  . r0=from, r0-to, r0-2
  random r0
  . r0+from
endf

;;;;;;;;;;;;;;;;;;; FILE I/O ;;;;;;;;;;;;;;;;;;;;;

numeric EOF=-1,\
 CREATE_NEW=1, CREATE_ALWAYS, OPEN_EXISTING,\
 OPEN_ALWAYS, TRUNCATE_EXISTING,\
 GENERIC_READ=80000000h, GENERIC_WRITE=40000000h,\
 FILE_SHARE_READ=1, FILE_SHARE_WRITE,\
 FILE_ATTRIBUTE_NORMAL=80h,\
 SEEK.BEGIN=0, SEEK.SET, SEEK.END

;;;;;;;;;;;;;;;;; CURRENT FILE ;;;;;;;;;;;;;;;;;;;

align

void file.p	  ; pointer for load/save
integer file.h,\  ; handle
 file.n64, file.n ; size 64:32
integer tmp.rw

macro flush { destroy file.p }

; return handle or -1 if error

function os.create.file, file, access, share,\
  security, action, attributes, template
  call !text.copy, file.name, file
  CreateFileA file.name, access, share,\
   security, action, attributes, template
  . file.h=r0
endf

macro os.open file {
 os.create.file file, GENERIC_READ \
  or GENERIC_WRITE, FILE_SHARE_READ,\
  0, OPEN_EXISTING, 0, 0
}

macro os.create file {
 os.create.file file, GENERIC_WRITE,\
  0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0
}

macro os.seek n, r
 { SetFilePointer file.h, n, 0, r }

macro os.read p, n {
 . r0=&tmp.rw
 ReadFile file.h, p, n, r0, 0
}

macro os.write p, n {
 . r0=&tmp.rw
 WriteFile file.h, p, n, r0, 0
}

macro os.get.file.size { GetFileSize file.h, 0 }

macro os.close { CloseHandle file.h }

;;;;;;;;;;;;;;; COPY, MOVE, DELETE ;;;;;;;;;;;;;;;

macro copy.file a, b, r { CopyFileA a, b, r }
macro move.file a, b	{ MoveFileA a, b }
macro delete.file f	{ DeleteFileA f }
macro rename.file a, b	{ os.move.file a, b }

;;;;;;;;;;;;;;;;;; DIRECTORIES ;;;;;;;;;;;;;;;;;;;

macro os.get.directory
 { GetCurrentDirectoryA 256, directory }

macro os.set.directory f
 { SetCurrentDirectoryA f }

macro os.create.directory f
 { CreateDirectoryA f, 0 }

macro os.get.file.name
 { GetModuleFileNameA 0, directory, 1*KB }

macro os.get.command.line { GetCommandLineA }

;;;;;;;;;;;;;;;;;;;; EXECUTE ;;;;;;;;;;;;;;;;;;;;;

macro execute file
 { ShellExecuteA 0, 0, file, 0, 0, 3 }

;;;;;;;;;;;;;;;;;; FIND FILES ;;;;;;;;;;;;;;;;;;;;

macro os.find.data {
 BLOCK find.data(338) ; WIN32_FIND_DATA
 os.found.file equ find.data+44
}

macro os.find.first file
 { FindFirstFileA file, find.data }

macro os.find.next
 { FindNextFileA find.data.h, find.data }

macro os.find.end { FindClose find.data.h }

;;;;;;;;;;;;;;;;;;;;; REDRAW ;;;;;;;;;;;;;;;;;;;;;

macro create.blank.screen w, h {
  create.vga os.w, os.h, WHITE ; 0
  create.blank.window w, h
}

macro redraw {
 calle draw
}

macro render b {
 ; call !clear.screen
 IF b eq
  redraw
 END IF
 IF used cursor
  call !draw.cursor, cursor
 END IF
 IF b eq
  call !show.vga
 ELSE
  copy.box box, b
  call !show.vga.box
 END IF
}

;;;;;;;;;;;;;;;;;;;; WINDOZE ;;;;;;;;;;;;;;;;;;;;;

macro POINT [a] { a: integer a#.x, a#.y }

macro RECT [a] { a: integer a#.left,\
 a#.top, a#.right, a#.bottom }

macro MSG [a] {
 a: integer a#.hwnd, a#.message,\
 a#.wParam, a#.lParam, a#.time
 POINT a#.pt
}

macro WNDCLASSEX [a] {
 a: integer a#.cbSize=48,\
 a#.style, a#.lpfnWndProc, a#.cbClsExtra,\
 a#.cbWndExtra, a#.hInstance, a#.hIcon,\
 a#.hCursor, a#.hbrBackground,\
 a#.lpszMenuName, a#.lpszClassName, a#.hIconSm
 WNDCLASSEX.$=$-a
}

macro PAINTSTRUCT [a] {
 a: integer a#.hdc, a#.fErase
 RECT a#.rcPaint
 integer a#.fRestore, a#.fIncUpdate
 text a#.rgbReserved(32)
}

macro BITMAP [a] {
 a: integer a#.bmType,\
  a#.bmWidth, a#.bmHeight, a#.bmWidthBytes
 short a#.bmPlanes, a#.bmBitsPixel
 void a#.bmBits
 BITMAP.$=$-a
}

macro BITMAPINFOHEADER [a] {
 a: integer a#.biSize, a#.biWidth, a#.biHeight
 short a#.biPlanes, a#.biBitCount
 integer a#.biCompression, a#.biSizeImage,\
  a#.biXPelsPerMeter, a#.biYPelsPerMeter,\
  a#.biClrUsed, a#.biClrImportant
 BITMAPINFOHEADER.$=$-a
}

macro BITMAPINFO [a] {
  BITMAPINFOHEADER a
  integer bmiColors
  BITMAPINFO.$=BITMAPINFOHEADER.$+4
}

; window messages

numeric WM_*, \
 CREATE=1, DESTROY=2, MOVE=3, SIZE=5,\
 SETFOCUS=7, KILLFOCUS=8, GETTEXT=0Dh,\
 SETTEXT=0Ch, GETTEXTLENGTH=0Eh,\
 PAINT=0Fh, CLOSE=10h, QUIT=12h, CUT=300h,\
 COPY=301h, PASTE=302h, CLEAR=303h,\
 SETFONT=30h, COMMAND=111h, TIMER=0113h

; window styles

numeric WS_*, \
 POPUP=80000000h, MINIMIZE=20000000h,\
 VISIBLE=10000000h, MAXIMIZE=1000000h,\
 CAPTION=0C00000h, BORDER=800000h,\
 DLGFRAME=400000h, VSCROLL=200000h,\
 HSCROLL=100000h, SYSMENU=80000h,\
 THICKFRAME=40000h, MINIMIZEBOX=20000h,\
 MAXIMIZEBOX=10000h

WS_BLANK   = WS_VISIBLE+WS_POPUP
WS_DEFAULT = WS_VISIBLE+WS_CAPTION+\
	     WS_MINIMIZEBOX+WS_SYSMENU

CS_DBLCLKS=8

; keyboard+mouse messages

numeric WM_*,\
 KEYDOWN=100h, KEYUP, CHAR, DEADCHAR,\
 SYSKEYDOWN, SYSKEYUP, SYSCHAR

numeric WM_*,\
 MOUSEMOVE=200h, LBUTTONDOWN, LBUTTONUP,\
 LBUTTONDBLCLK, RBUTTONDOWN, RBUTTONUP,\
 RBUTTONDBLCLK, MBUTTONDOWN, MBUTTONUP,\
 MBUTTONDBLCLK, MOUSEWHEEL

; virtual key codes. function keys=(6Fh+N).
; example: F1=70h (6Fh+1)

numeric K.*,\
 FUNCTION=6Fh, LEFT=25h, UP=26h, RIGHT=27h,\
 DOWN=28h, ESCAPE=1Bh, SPACE=20h, DELETE=2Eh,\
 CONTROL=11h, LCONTROL=0A2h, RCONTROL=0A3h,\
 LALT=0A4h, RALT=0A5h, BACK=8, TAB=9,\
 RETURN=0Dh, END=23h, HOME=24h,\
 A='A', S='S', D='D', W='W'

SRCCOPY=00CC0020h

macro os.show.cursor { ShowCursor 1 }

;;;;;;;;;;;;;;;;;;;;; SYSTEM ;;;;;;;;;;;;;;;;;;;;;

align

integer os.w, os.h, os.bpp

void _hwnd	; handle
void _dc	; device context
void _mdc	; memory dc

WNDCLASSEX _wc	; window
MSG _wm 	; message
PAINTSTRUCT _ps ; for PAINT
BITMAP _bm	; for draw.bitmap.w

void vga.hbm
BITMAPINFO vga.bmi
RECT vga.rect

text _cn='WC', _wt='' ; classname, title

;;;;;;;;;;;;;;;;; CREATE WINDOW ;;;;;;;;;;;;;;;;;;

function create.window.x, style, procedure,\
 w, h, title, class.name
  locals x, y

  . _wc.cbSize=WNDCLASSEX.$
  . _wc.hInstance=@module
  . _wc.lpfnWndProc=procedure
  . _wc.lpszClassName=class.name
  . _wc.style=CS_DBLCLKS, _wc.hbrBackground=8
  get _wc.hIcon=LoadIconA 0, 7F00h
  get _wc.hCursor=LoadCursorA 0, 7F00h

  try RegisterClassExA _wc

  . r0=os.w, r0>>1, r2=w, r2>>1, r0-r2, x=r0
  . r0=os.h, r0>>1, r2=h, r2>>1, r0-r2, y=r0

  try _hwnd=CreateWindowExA 0, class.name,\
   title, style, x, y, w, h, 0, 0, @module, 0
endf 1

macro create.blank.window w, h {
 create.window.x WS_BLANK, !_window.procedure,\
  w, h, _wt, _wc
}

macro create.default.window title {
 create.window.x WS_DEFAULT, !_window.procedure,\
  os.w, os.h, title, _wc
}

;;;;;;;;;;;;;;;;;; MESSAGE LOOP ;;;;;;;;;;;;;;;;;;

macro begin.message.loop {
 .begin.ml:
 GetMessageA _wm, 0, 0, 0
 fail .end.ml
 TranslateMessage _wm
 DispatchMessageA _wm
}

macro end.message.loop {
 go .begin.ml
 .end.ml:
 . r0=_wm.wParam
}

macro message.loop {
 begin.message.loop
 end.message.loop
}

macro process.messages {
 .begin.ml:
 PeekMessageA _wm, 0, 0, 0, 0
 fail .no.message
 GetMessageA _wm, 0, 0, 0
 fail .end.ml
 TranslateMessage _wm
 DispatchMessageA _wm
 go .begin.ml
 .no.message:
}

macro end.messages {
 go .begin.ml
 .end.ml:
 ExitProcess _wm.wParam
}

macro minimize.window { ShowWindow _hwnd, 6 }

;;;;;;;;;;;;;;;;;;;;;; INPUT ;;;;;;;;;;;;;;;;;;;;;

align 4

integer event.id, key.event, mouse.event,\
 key, any.key, key.c, exit.if.esc=YES,\
 mouse.1, mouse.2, mouse.x, mouse.y,\
 mouse.px, mouse.py, mouse.double, mouse.wheel,\
 mouse.drag, mouse.drag.x, mouse.drag.y,\
 mouse.drop, mouse.drop.x, mouse.drop.y

macro os.key.state k { GetAsyncKeyState k }

macro os.set.cursor.xy x, y
 { SetCursorPos x, y }

function key.state, k
  os.key.state k
endf

; if key state

macro if.key k { !if key.state K.#k }
macro if.not.keys k { !if.n key.state K.#k }

function select.box, box
  . r0=mouse.x, r1=mouse.y
  IF defined cursor
    . r0+cursor.spot.x, r1+cursor.spot.y
  END IF
  call !point.inside, box, r0, r1
endf

macro if.select box { !if select.box, box }
macro else.if.select box
 { !else.if select.box, box }

macro if.not.select box
 { !if.n select.box, box }

macro if.click box {
  select.box box
  and r0, mouse.1
  if true
}

;;;;;;;;;;;;;;;;;;;;; EVENTS ;;;;;;;;;;;;;;;;;;;;;

align
void !_on.event, !on.main,\
 !_on.create, !_on.destroy, !_on.close,\
 !_on.draw, !_on.game, !_on.command,\
 !_on.key, !_on.mouse, !_on.timer, !_on.exit

macro define.events [e]
 { mov [!!_on.#e], !on.#e }

macro calle e {
 if dword [!!_on.#e]
  call dword [!!_on.#e]
 end
}

!call fix calle

macro !on name { function on.#name }
macro !end { endf 1 }

;;;;;;;;;;;;;;;;;;;;; TIMER ;;;;;;;;;;;;;;;;;;;;;;

macro os.set.timer f, ms
 { SetTimer _hwnd, 1, ms, f }

macro set.timer a, b {
 IF b eq
  os.set.timer !on.timer, a
 ELSE
  os.set.timer a, b
 END IF
}

;;;;;;;;;;;;;;;; WINDOW PROCEDURE ;;;;;;;;;;;;;;;;

function _window.procedure, window, message, wp, lp
  alias m=r0
  . m=message, event.id=0, mouse.double=0

  if m=WM_PAINT
    get _dc=BeginPaint _hwnd, _ps
    render
    EndPaint _hwnd, _ps
    go .default

  else.if m=WM_COMMAND
    calle command

  else.if m=WM_KEYDOWN
    . key=wp, event.id='k', key.event='k'
    if exit.if.esc
      if wp=K.ESCAPE
	SendMessageA window, WM_DESTROY, 0, 0
      end
    end
    .key:
    calle key
    return 0

  else.if m=WM_KEYUP
    . key=NO, event.id='k', key.event='r'
    go .key

  else.if m=WM_CHAR
    . key=wp, event.id='k', key.event='c'
    go .key

  else.if m=WM_MOUSEMOVE
    . mouse.event='m'
    if mouse.1
      if not mouse.drag
	. mouse.drag=YES,\
	mouse.drag.x=mouse.x,\
	mouse.drag.y=mouse.y
      end
    end
    .mouse:
     . event.id='m', r0=lp, r1=r0,\
     r0&0FFFFh, mouse.x=r0,\
     r1>>16, r1&0FFFFh, mouse.y=r1
    calle mouse
    if mouse.event='m'
      . mouse.px=mouse.x,\
       mouse.py=mouse.y
    end
    return 0

  else.if m=WM_LBUTTONDOWN
    . mouse.event='c', mouse.1=YES,\
    mouse.drop=NO
    go .mouse

  else.if m=WM_LBUTTONUP
    . mouse.event='r', mouse.1=NO
    if mouse.drag
      . mouse.drop=YES,\
      mouse.drop.x=mouse.x,\
      mouse.drop.y=mouse.y,\
      mouse.drag=NO
    end
    go .mouse

  else.if m=WM_LBUTTONDBLCLK
    . mouse.double=YES
    go .mouse

  else.if m=WM_RBUTTONDOWN
    . mouse.event='rc', mouse.2=YES
    go .mouse

  else.if m=WM_RBUTTONUP
    . mouse.event='rr', mouse.2=NO
    go .mouse

  else.if m=WM_MOUSEWHEEL
    . mouse.event='w', r1=wp,\
    r1>>16, mouse.wheel=r1
    go .mouse

  else.if m=WM_CREATE
    calle create
    go .default

  else.if m=WM_DESTROY
    .destroy:
    calle destroy
    PostQuitMessage 0
  end

  .default: DefWindowProcA \
   window, message, wp, lp
endf

;;;;;;;;;;;;;;; LOAD/DRAW H/BITMAP ;;;;;;;;;;;;;;;

function load.bitmap.w, file
  locals p
  try p=LoadImageA @module, file, 0, 0, 0, 10h
  GetObjectA p, BITMAP.$, _bm
endf p

function draw.bitmap.w, hbmp, x, y, w, h
  locals bmw, bmh
  GetObjectA hbmp, BITMAP.$, _bm
  . bmw=_bm.bmWidth, bmh=_bm.bmHeight
  get _mdc=CreateCompatibleDC _dc
  SelectObject _mdc, hbmp
  StretchBlt _dc, x, y, w, h,\
   _mdc, 0, 0, bmw, bmh, SRCCOPY
  DeleteDC _mdc
endf

;;;;;;;;;;;;;;;;;;;;;; VGA ;;;;;;;;;;;;;;;;;;;;;;;

macro os.get.screen.w
 { get os.w=GetSystemMetrics 0 }

macro os.get.screen.h
 { get os.h=GetSystemMetrics 1 }

function os.create.vga, w, h
  alias p=r0, x=r1
  ; set.screen screen.w, screen.h, screen.bpp
  try vga.hbm=CreateBitmap \
   screen.w, screen.h, 32, 1, vga.p
  memory.zero vga.bmi, BITMAPINFOHEADER.$
  . vga.bmi.biSize=BITMAPINFOHEADER.$
  . vga.bmi.biWidth=screen.w
  . x=screen.h, neg x, vga.bmi.biHeight=x
  . vga.bmi.biPlanes=1, vga.bmi.biBitCount=32
endf

function os.show.vga
  SetDIBits _dc, vga.hbm, 0, screen.h,\
   vga.p, vga.bmi, 0
  draw.bitmap.w vga.hbm, 0, 0, screen.w, screen.h
  . vga.rect.left=0, vga.rect.top=0,\
   vga.rect.right=screen.w,\
   vga.rect.bottom=screen.h
  InvalidateRect _hwnd, vga.rect, 0
endf

function show.vga.box
  SetDIBits _dc, vga.hbm, 0, screen.h,\
   vga.p, vga.bmi, 0
  draw.bitmap.w vga.hbm,\
   box.x, box.y, box.w, box.y
  . r0=box.x, r1=box.y,\
   vga.rect.left=r0, vga.rect.top=r1,\
   r0+box.w, vga.rect.right=r0,\
   r1+box.h, vga.rect.bottom=r1
  InvalidateRect _hwnd, vga.rect, 0
endf

macro show.vga.box b {
  IF ~b eq
    copy.box box, b
  END IF
  show.vga.box
}

macro define.vga { os.define.vga }

; create vga/buffer for drawing

function create.vga, w, h, c
  if vga.p=0, r0=w, r0*h, r0<<2
    try vga.p=allocate r0
  end
  call !clear.screen, c
  os.create.vga w, h
endf 1

function show.vga
  os.show.vga
endf

function set.vga, w, h
  os.set.vga
endf

function end.vga
  destroy vga
  os.end.vga
endf

;;;;;;;;;;;;;;;;;; ENTER+EXIT ;;;;;;;;;;;;;;;;;;;;

; user-defined enter/exit routines will be called
; if defined/nonzero

function os.enter
  try @module=GetModuleHandleA 0
  try @heap=HeapCreate 0, 0, 0
  try directory=allocate 1*KB
  . r0=directory, *r0=0
  try file.name=allocate 1*KB
  . r0=file.name, *r0=0
  os.get.directory
  os.get.command.line
  . command.line=r0
endf 1

function exit
  ExitProcess 0
endf

;;;;;;;;;;;;; EXECUTABLE STRUCTURE ;;;;;;;;;;;;;;;

align

section '.one' \
 code readable writable executable
 !main:
  os.enter
  if false
    say 'System error'
    exit
  end
  call !main!
  exit
  ret

function main!
  os.get.screen.w
  os.get.screen.h
  set.screen WINDOW.W, WINDOW.H, 32
  try create.vga screen.w, screen.h, BLACK
  define.events create, draw, key, mouse
  create.blank.window screen.w, screen.h
  os.show.cursor
  message.loop
endf

align