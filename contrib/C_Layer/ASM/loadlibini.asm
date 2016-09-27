format coff
use32                                   ; Tell compiler to use 32 bit instructions
	
section '.flat' code			; Keep this line before includes or GCC messes up call addresses

;include 'struct.inc'
include '../../../programs/proc32.inc'
include '../../../programs/macros.inc'
purge section,mov,add,sub

include '../../../programs/dll.inc'
	
public init_libini as '_kolibri_libini_init'
	
;;; Returns 0 on success. -1 on failure.

proc init_libini
	pusha
	mcall 68,11
	stdcall dll.Load, @IMPORT
	popa
	ret
endp	
	
@IMPORT:

library lib_libini,               'libini.obj'

import  lib_libini, \
		LIBINI_enum_sections           , 'ini_enum_sections'    , \
        LIBINI_enum_keys               , 'ini_enum_keys'        , \
        LIBINI_get_str                 , 'ini_get_str'          , \
        LIBINI_get_int  			   , 'ini_get_int'   		, \
        LIBINI_get_color               , 'int_get_color'        , \
        LIBINI_get_shortcut            , 'int_get_shortcut'     , \
        LIBINI_set_str            	   , 'ini_set_str'          , \
        LIBINI_set_int         		   , 'ini_set_int'          , \          
        LIBINI_set_color          	   , 'ini_set_color'
        
public LIBINI_enum_sections         as  '_LIBINI_enum_sections' 
public LIBINI_enum_keys             as  '_LIBINI_enum_keys'     
public LIBINI_get_str               as  '_LIBINI_get_str'       
public LIBINI_get_int  			    as  '_LIBINI_get_int'  		
public LIBINI_get_color             as  '_LIBINI_get_color'     
public LIBINI_set_str            	as  '_LIBINI_set_str'       
public LIBINI_set_int         		as  '_LIBINI_set_int'       
