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

; Most aliases below was changed for compatible to latest version tcc and the libc.obj headers 	
public inet_addr
public inet_ntoa
public getaddrinfo as '__imp_getaddrinfo@16'
public freeaddrinfo as '__imp_freeaddrinfo@4'

public rand as '__imp_rand'
public memcmp as '__imp_memcmp'
public printf as '__imp_printf'
public calloc as '__imp_calloc'
public free as '__imp_free'
public strlen as '__imp_strlen'
public _strcmp as '__imp_strcmp'
public strstr as '__imp_strstr'
public gmtime
public vsnprintf as '__imp_vsnprintf'
public socket as '__imp_socket'
public connect as '__imp_connect'
public close as '__imp_close'
public recv as '__imp_recv'
public send as '__imp_send'
public time as '__imp_time'
public strncmp as '__imp_strncmp'
public strncpy as '__imp_strncpy'
public snprintf as '__imp_snprintf'
