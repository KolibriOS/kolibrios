
format coff
use32                                   ; Tell compiler to use 32 bit instructions

section '.init' code			; Keep this line before includes or GCC messes up call addresses

include '../../../programs/proc32.inc'
include '../../../programs/macros.inc'
purge section,mov,add,sub
	
include '../../../programs/dll.inc'
	
public init_msgbox as '_kolibri_msgbox_init'
;;; Returns 0 on success. -1 on failure.

proc init_msgbox
	pusha
	mcall 68,11
	stdcall dll.Load, @IMPORT
	popa
	ret
endp	

@IMPORT:
library lib_boxlib, 	'msgbox.obj'

import lib_boxlib, \
	mb_create, 'mb_create' , \
	mb_reinit, 'mb_reinit' , \
	mb_setfunctions, 'mb_setfunctions'
	
public mb_create as '_msgbox_create'
public mb_reinit as '_msgbox_reinit'
public mb_setfunctions as '_msgbox_setfunctions'
