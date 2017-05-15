format coff
use32                                   ; Tell compiler to use 32 bit instructions
	
section '.flat' code			; Keep this line before includes or GCC messes up call addresses

include '../../../programs/struct.inc'
include '../../../programs/proc32.inc'
include '../../../programs/macros.inc'
purge section,mov,add,sub

include '../../../programs/network.inc'
include '../../../programs/develop/libraries/http/http.inc'
include '../../../programs/dll.inc'
	
virtual at 0
        http_msg http_msg
end virtual

public init_network as '_init_network_asm'
	
;;; Returns 0 on success. -1 on failure.

proc init_network
	stdcall dll.Load, @IMPORT
	test    eax, eax
	jnz     error

	mov eax, 0
	ret

error:
	mov eax, -1
	ret
endp

@IMPORT:

library lib_http,               'http.obj'

import  lib_http, \
        HTTP_get                , 'get'                 , \
        HTTP_head               , 'head'                , \
        HTTP_post               , 'post'                , \
        HTTP_find_header_field  , 'find_header_field'   , \
        HTTP_send               , 'send'                , \
        HTTP_receive            , 'receive'             , \
        HTTP_disconnect         , 'disconnect'          , \
        HTTP_free               , 'free'                , \
        HTTP_escape             , 'escape'              , \
        HTTP_unescape           , 'unescape'

public HTTP_get as '_http_get_asm'
public HTTP_head as '_http_head_asm'
public HTTP_post as '_http_post_asm'
public HTTP_find_header_field as '_http_find_header_field_asm'
public HTTP_send as '_http_send_asm'
public HTTP_receive as '_http_receive_asm'
public HTTP_disconnect as '_http_disconnect_asm'
public HTTP_free as '_http_free_asm'
public HTTP_escape as '_http_escape_asm'
public HTTP_unescape as '_http_unescape_asm'
