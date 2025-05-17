align 4
import_tinygl:

macro E_LIB n
{
if defined sz_#n
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