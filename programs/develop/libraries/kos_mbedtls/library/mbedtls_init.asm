format coff
use32                                   ; Tell compiler to use 32 bit instructions

;section '.export'

section '.text' 

include '../../../../proc32.inc'
include '../../../../macros.inc'
include '../../../../debug-fdo.inc'
include '../../../../dll.inc'


public mbedtls_init
;;; Returns 0 on success. -1 on failure.

__DEBUG__       = 1
__DEBUG_LEVEL__ = 2


mbedtls_init:
        pushad
        stdcall dll.Load, @IMPORT
        ;int3
        test    eax, eax
        jnz     .error
        
        popad
        mov eax, 0
        ret

.error:
        popad
        mov eax, -1
        ret

gmtime:
        jmp [localtime]

;include_debug_strings

section '.data'

align 4
@IMPORT:
library libc, 'libc.obj', networklib, 'network.obj'
import  libc, \
        memcmp,           'memcmp', \
        printf,           'printf', \
        free,             'free', \
        strlen,           'strlen', \
        _strcmp,          'strcmp', \
        strstr,           'strstr', \
        rand,             'rand', \
        vsnprintf,        'vsnprintf', \
        socket,           'socket', \
        connect,          'connect', \
        close      ,      'close', \
        recv,             'recv', \
        send,             'send', \
        time,             'time', \
        strncmp,          'strncmp', \
        strncpy,          'strncpy', \
        calloc,           'calloc' , \
        snprintf,         'snprintf', \
        localtime,        'localtime'

import networklib, \
        inet_addr, 'inet_addr', \
        inet_ntoa, 'inet_ntoa', \
        getaddrinfo, 'getaddrinfo', \
        freeaddrinfo, 'freeaddrinfo'
	
public inet_addr
public inet_ntoa
public getaddrinfo
public freeaddrinfo

public rand
public memcmp
public printf
public calloc
public free
public strlen
public _strcmp as 'strcmp'
public strstr
public gmtime
public vsnprintf
public socket
public connect
public close
public recv
public send
public time
public strncmp
public strncpy
public snprintf
