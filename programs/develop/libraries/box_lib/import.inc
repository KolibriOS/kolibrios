align 4
import_box_lib:

macro E_LIB n, lfn
{
if n eq lib_init
	dd strz_#n
else if defined sz_#n
	n dd sz_#n
end if
}
include 'export.inc'
	dd 0,0
macro E_LIB n, lfn
{
if used n
if lfn eq
	sz_#n db `n,0
else
	sz_#n db `lfn,0
end if
end if
}
include 'export.inc'