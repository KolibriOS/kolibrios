;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2014. All rights reserved.         ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  pasta.asm - Paste something to paste.kolibrios.org using POST  ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary as ""
use32
        org     0x0

        db      'MENUET01'      ; header
        dd      0x01            ; header version
        dd      START           ; entry point
        dd      IM_END          ; image size
        dd      I_END+0x1000    ; required memory
        dd      I_END+0x1000    ; esp
        dd      0
        dd      0               ; I_Path


include '../../../../macros.inc'
include '../../../../proc32.inc'
include '../../../../dll.inc'
include '../../http/http.inc'

virtual at 0
        http_msg http_msg
end virtual


START:
        mcall   68, 11                  ; init heap so we can allocate memory dynamically

; load libraries
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     exit

        invoke  HTTP_get, sz_url, 0
        test    eax, eax
        jz      error
        mov     [identifier], eax

  .again:
        invoke  HTTP_process, [identifier]
        test    eax, eax
        jnz     .again

        invoke  HTTP_find_header_field, [identifier], sz_set_cookie
        mov     edi, cookie
        test    eax, eax
        jz      .no_cookie

        mov     esi, eax
  .cookie_loop:
        lodsb
        stosb
        cmp     al, 13
        je      .cookie_end
        cmp     al, 10
        je      .cookie_end
        cmp     al, 0
        je      .cookie_end
        jmp     .cookie_loop
  .cookie_end:
        dec     edi
  .no_cookie:
        mov     ax, 0x0a0d
        stosw
        xor     al, al
        stosb

        invoke  HTTP_free, [identifier]

        invoke  HTTP_post, sz_url, sz_cookie, sz_ctype, sz_paste.length
        test    eax, eax
        jz      error
        mov     [identifier], eax

        mov     ecx, [eax + http_msg.socket]
        mcall   75, 6, , sz_paste, sz_paste.length, 0

  .again2:
        invoke  HTTP_process, [identifier]
        test    eax, eax
        jnz     .again2

        mov     ebp, [identifier]
        cmp     [ebp + http_msg.status], 302    ; found
        jne     error

        invoke  HTTP_find_header_field, [identifier], sz_location
        test    eax, eax
        jz      error
        mov     esi, eax
        mov     edi, paste_url
        mov     al, '"'
        stosb
  .url_loop:
        lodsb
        stosb
        cmp     al, 13
        je      .url_end
        cmp     al, 10
        je      .url_end
        cmp     al, 0
        je      .url_end
        jmp     .url_loop
  .url_end:
        dec     edi
        mov     eax, '" -N'
        stosd
        xor     al, al
        stosb

        mov     [notify_struct.msg], paste_url
        mcall   70, notify_struct

        invoke  HTTP_free, [identifier]
        jmp     exit

error:
        mov     [notify_struct.msg], sz_failed
        mcall   70, notify_struct
exit:
        mcall   -1


;---------------------------------------------------------------------
; Data area
;-----------------------------------------------------------------------------
align   4
@IMPORT:

library lib_http,       'http.obj'

import  lib_http, \
        HTTP_get,       'get', \
        HTTP_process,   'process', \
        HTTP_free,      'free', \
        HTTP_stop,      'stop', \
        HTTP_post,      'post', \
        HTTP_find_header_field, 'find_header_field'


identifier      dd 0

IM_END:

notify_struct:
        dd 7            ; run application
        dd 0
  .msg  dd paste_url
        dd 0
        dd 0
        db '/sys/@notify', 0

sz_url          db 'http://paste.kolibrios.org/', 0
sz_set_cookie   db 'set-cookie', 0
sz_location     db 'location', 0
sz_ctype        db 'application/x-www-form-urlencoded', 0
sz_failed       db '"Paste failed!" -N', 0

sz_paste        db 'code=test+from+KolibriOS+2%0D%0A&language=text&webpage='
.length = $ - sz_paste

sz_cookie       db 'Cookie: '
cookie          db 0
                rb 1024

paste_url       rb 1024

I_END:



