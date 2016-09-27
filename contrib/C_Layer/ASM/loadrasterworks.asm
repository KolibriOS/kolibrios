format coff
use32                                   ; Tell compiler to use 32 bit instructions
	
section '.flat' code			; Keep this line before includes or GCC messes up call addresses

;include 'struct.inc'
include '../../../programs/proc32.inc'
include '../../../programs/macros.inc'
purge section,mov,add,sub

include '../../../programs/dll.inc'
	
public init_rasterworks as '_kolibri_rasterworks_init'
	
;;; Returns 0 on success. -1 on failure.

proc init_rasterworks
	pusha
	mcall 68,11
	stdcall dll.Load, @IMPORT
	popa
	ret
endp	
	
@IMPORT:

library lib_rasterworks,     'rasterworks.obj'

import  lib_rasterworks, \
		drawText           , 'drawText'    , \
        countUTF8Z         , 'cntUTF-8'    , \
		charsFit           , 'charsFit'    , \
		strWidth           , 'strWidth'

public drawText         as  '_drawText' 
public countUTF8Z       as  '_countUTF8Z'
public charsFit         as  '_charsFit'
public strWidth         as  '_strWidth'
