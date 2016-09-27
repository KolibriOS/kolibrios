
format coff
use32                                   ; Tell compiler to use 32 bit instructions

section '.init' code			; Keep this line before includes or GCC messes up call addresses

include '../../../programs/proc32.inc'
include '../../../programs/macros.inc'
purge section,mov,add,sub
	
include '../../../programs/dll.inc'
	
public init_proclib as '_kolibri_proclib_init'
;;; Returns 0 on success. -1 on failure.

proc init_proclib
	pusha
	mcall 68,11
	stdcall dll.Load, @IMPORT
	popa
	ret
endp	

@IMPORT:
library lib_boxlib, 	'proc_lib.obj'

import lib_boxlib, \
	OpenDialog_init, 'OpenDialog_init' , \
	OpenDialog_start, 'OpenDialog_start' , \
	ColorDialog_init, 'ColorDialog_init' , \
	ColorDialog_start, 'ColorDialog_start'
	
public OpenDialog_init as '_OpenDialog_init'
public OpenDialog_start as '_OpenDialog_start'

public ColorDialog_init as '_ColorDialog_init'
public ColorDialog_start as '_ColorDialog_start'
