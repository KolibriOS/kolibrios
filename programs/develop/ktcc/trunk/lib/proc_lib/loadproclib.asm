
format elf
use32                                   ; Tell compiler to use 32 bit instructions

section '.text' executable			; Keep this line before includes or GCC messes up call addresses

include '../../../../../proc32.inc'
include '../../../../../macros.inc'
purge section,mov,add,sub
	
include '../../../../../dll.inc'
	
public init_proclib as 'kolibri_dialog_init'
;;; Returns 0 on success. -1 on failure.

proc init_proclib
	pusha
	mcall 68,11
	stdcall dll.Load, @IMPORT
	popa
	ret
endp	

section '.data' writeable

@IMPORT:
library lib_boxlib, 	'proc_lib.obj'

import lib_boxlib, \
	OpenDialog_init, 'OpenDialog_init' , \
	OpenDialog_start, 'OpenDialog_start' , \
	ColorDialog_init, 'ColorDialog_init' , \
	ColorDialog_start, 'ColorDialog_start'
	
public OpenDialog_init as 'OpenDialog_init'
public OpenDialog_start as 'OpenDialog_start'

public ColorDialog_init as 'ColorDialog_init'
public ColorDialog_start as 'ColorDialog_start'
