format coff
use32                                   ; Tell compiler to use 32 bit instructions

section '.flat' code

include '../../../programs/proc32.inc'
include '../../../programs/macros.inc'
purge section,mov,add,sub

include '../../../programs/dll.inc'

purge section,mov,add,sub
        
public lib_init as '_networklib_init'

proc lib_init
local retval dd ?
        mov [retval], eax
        pusha
        mcall 68, 11
        test eax, eax
        jnz @f
                mov [retval], -1
                jmp exit_init_networklib
@@:     
        stdcall dll.Load, @IMPORT
        test eax, eax
        jz      exit_init_networklib
                mov [retval], -1
exit_init_networklib:       
        popa
        mov eax, [retval]
        ret
endp    

@IMPORT:
library networklib, 	'network.obj'

import networklib, \
        inet_addr, 'inet_addr', \
        inet_ntoa, 'inet_ntoa', \
        getaddrinfo, 'getaddrinfo', \
        freeaddrinfo, 'freeaddrinfo'
	
public inet_addr as '_inet_addr'
public inet_ntoa as '_inet_ntoa'
public getaddrinfo as '_getaddrinfo'
public freeaddrinfo as '_freeaddrinfo'
