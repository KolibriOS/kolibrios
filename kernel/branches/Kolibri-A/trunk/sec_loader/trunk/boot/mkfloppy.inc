; ---------------------------------------------------------------------------
; mkfloppy.inc
; ---------------------------------------------------------------------------
; Created by Phantom-84
; ---------------------------------------------------------------------------

FA_RO  equ 01h
FA_HID equ 02h
FA_SYS equ 04h
FA_VOL equ 08h
FA_DIR equ 10h
FA_ARC equ 20h

DSTAMP equ 28C1h
TSTAMP equ 6000h

root_size=0

macro reset id
{
local count, cur, disp, val, var
times 511-($+511) mod 512 db 0
if id#_size>0
count=(id#_size+511)/512
cur=id#_base/512-(33-2)
repeat count
if %=count
val=0FFFh
else
val=cur+1
end if
if cur and 1
val=val shl 4
end if
disp=(cur*3)/2
load var word from 512+disp
var=var or val
store word var at 512+disp
store word var at 10*512+disp
cur=cur+1
end repeat
end if
}

macro dent id, name, attr
{
@@ db name
times @b+11-$ db 32
db attr
dw 0, TSTAMP, DSTAMP, DSTAMP, 0, TSTAMP, DSTAMP
if id#_size=0
dw 0
else
dw id#_base/512-(33-2)
end if
if (attr) and FA_DIR
dd 0
else
dd id#_size
end if
}

macro orgdir id, parentid
{
id#_base:
dent id, ".", FA_DIR
dent parentid, "..", FA_DIR
}

macro findir id
{
id#_size=$-id#_base
reset id
}

macro stod id, parentid
{
orgdir id, parentid
id
findir id
}

macro stof id, name
{
id#_base: file name
id#_size=$-id#_base
reset id
}

defdir fix macro