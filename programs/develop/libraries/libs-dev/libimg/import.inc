align 4
import_libimg:

macro E_LIB n
{
if n eq lib_init
	dd strz_#n
else if defined sz_#n
	n dd sz_#n
end if
}
include 'export.inc'
	dd 0,0
macro E_LIB n
{
if used n
	sz_#n db `n,0
end if
}
include 'export.inc'