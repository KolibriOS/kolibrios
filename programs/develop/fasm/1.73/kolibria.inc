
; Kolibri programming headers (ASCII)

datadef@directives equ db,db.kol,dw,du,dd,dp,dq,dt
datarsv@directives equ rb,rw,rd,rp,rq,rt
algnmnt@directives equ
def.rb equ db
def.rw equ dw
def.rd equ dd
def.rp equ dp
def.rq equ dq
def.rt equ dt
db.align = 1
dw.align = 2
du.align = 2
dd.align = 4
dq.align = 8

sizeof.PTR =4
struc PTR [value:?] { common . dd value }
macro PTR [value:?] { common dd value }
sizeof.VOID =4
struc VOID [value:?] { common . dd value }
macro VOID [value:?] { common dd value }

;====== fundamental macros ======================
include 'kolibri/macro/struct.inc'
;====== OS specific macros ======================
include 'kolibri/macro/proc.inc'
include 'kolibri/macro/syscall.inc'
;====== file format specific macros =============
include 'kolibri/kex/import.inc'
include 'kolibri/kex/export.inc'

struc TCHAR [val:?] { common . db val }
sizeof.TCHAR = 1
struc ACHAR [val:?] { common . db val }
sizeof.ACHAR = 1
struc WCHAR [val:?] { common . du val }
sizeof.WCHAR = 2


;====== OS specific equates =====================
include 'kolibri/equates/syscalls.inc'
include 'kolibri/equates/kernel.inc'
include 'kolibri/equates/box_lib.inc'
include 'kolibri/equates/proc_lib.inc'
;====== COM technology equates ==================

force@alignment equ
;====== OS specific structures ==================
include 'kolibri/structs/kernel.inc'
include 'kolibri/structs/box_lib.inc'
include 'kolibri/structs/proc_lib.inc'
include 'kolibri/structs/coords.inc'
include 'kolibri/structs/aces.inc'
