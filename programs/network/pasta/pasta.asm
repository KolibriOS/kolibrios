;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2014. All rights reserved.         ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  pasta.asm - Paste text to paste.kolibrios.org from a file or   ;;
;;              from clipboard.                                    ;;
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
        dd      param
        dd      0               ; I_Path


include '../../macros.inc'
include '../../proc32.inc'
include '../../dll.inc'
include '../../develop/libraries/http/http.inc'

virtual at 0
        http_msg http_msg
end virtual


START:
        mcall   68, 11                  ; init heap so we can allocate memory dynamically

; load libraries
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     error

; Got parameters?
        cmp     byte[param], 0
        je      clipboard

; Yep, try to read the file.
        mcall   68, 12, 32768
        test    eax, eax
        jz      error
        mov     [file_struct.buf], eax
        mov     [clipboard_data], eax
        mcall   70, file_struct
        cmp     eax, 6
        jne     error_free_clip
        mov     [clipboard_data_length], ebx
        mov     eax, [clipboard_data]

        jmp     escape

clipboard:

; Get number of slots on the clipboard
        mcall   54, 0
        cmp     eax, -1
        je      error

; Get last item on clipboard
        mov     ecx, eax
        dec     ecx
        inc     ebx
        mcall   54
        cmp     eax, -1
        je      error

; Verify if we can work with it
        mov     [clipboard_data], eax
        cmp     dword[eax + 4], 0               ; text ?
        jne     error_free_clip

; Save length in [clipboard_data_length]
        mov     ecx, dword[eax]
        sub     ecx, 8
        mov     [clipboard_data_length], ecx

; Skip clipboard containter params for escape proc
        add     eax, 12

escape:
; Escape all characters that need escaping
        invoke  HTTP_escape, eax
        test    eax, eax
        jz      error_free_clip
        mov     [clipboard_data_length], ebx

        push    eax
        mcall   68, 13, [clipboard_data]
        pop     [clipboard_data]

; Connect to the server
        invoke  HTTP_get, sz_url, 0
        test    eax, eax
        jz      error_free_clip
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

        mov     ecx, [clipboard_data_length]
        add     ecx, sz_paste_head.length + sz_paste_tail.length
        invoke  HTTP_post, sz_url, sz_cookie, sz_ctype, ecx
        test    eax, eax
        jz      error_free_clip
        mov     [identifier], eax

; Send the data to the server
        mov     ecx, [eax + http_msg.socket]
        mcall   75, 6, , sz_paste_head, sz_paste_head.length, 0
        mcall   75, 6, , [clipboard_data], [clipboard_data_length], 0
        mcall   75, 6, , sz_paste_tail, sz_paste_tail.length, 0

; Free the data
        mcall   68, 13, [clipboard_data]

  .again2:
        invoke  HTTP_process, [identifier]
        test    eax, eax
        jnz     .again2

        mov     ebp, [identifier]
        cmp     [ebp + http_msg.status], 302    ; found
        jne     error_free_http

        invoke  HTTP_find_header_field, [identifier], sz_location
        test    eax, eax
        jz      error_free_http

        push    eax
        mov     esi, sz_failed
        mov     edi, paste_url
        mov     ecx, 2
        rep movsd
        pop     esi
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
        lea     ecx, [edi - paste_url + 4]
        mov     eax, '" -t'
        stosd
        mov     ax, 'O'
        stosw
        mov     [notify_struct.msg], paste_url
        mcall   70, notify_struct

        mcall   54, 3                   ; Delete last slot in the clipboard

        mov     dword[paste_url-4], ecx
        mov     dword[paste_url+0], 0   ; Text
        mov     dword[paste_url+4], 1   ; cp0866
        mcall   54, 2, , paste_url-4    ; Write URL to the clipboard

        invoke  HTTP_free, [identifier]
        mcall   -1

error_free_http:
        invoke  HTTP_free, [identifier]
        jmp     error
error_free_clip:
        mcall   68, 13, [clipboard_data]
error:
        mov     [notify_struct.msg], sz_failed
        mcall   70, notify_struct

        mcall   -1


;---------------------------------------------------------------------
; Data area
;-----------------------------------------------------------------------------
align   4
@IMPORT:

library lib_http,               'http.obj'

import  lib_http, \
        HTTP_get,               'get', \
        HTTP_process,           'process', \
        HTTP_free,              'free', \
        HTTP_stop,              'stop', \
        HTTP_post,              'post', \
        HTTP_find_header_field, 'find_header_field', \
        HTTP_escape,            'escape'

IM_END:

file_struct:
        dd 0            ; read file
        dd 0            ; offset
        dd 0            ; reserved
        dd 32768        ; max file size
  .buf  dd 0            ; buffer ptr
        db 0
        dd param

notify_struct:
        dd 7            ; run application
        dd 0
  .msg  dd paste_url
        dd 0
        dd 0
        db '/sys/@notify', 0

sz_url                  db 'http://paste.kolibrios.org/', 0
sz_set_cookie           db 'set-cookie', 0
sz_location             db 'location', 0
sz_ctype                db 'application/x-www-form-urlencoded', 0
sz_failed               db '"Pasta',13,10,'Paste failed!" -E', 0

sz_paste_head           db 'code='
.length = $ - sz_paste_head
sz_paste_tail           db '%0D%0A&language=text&webpage='
.length = $ - sz_paste_tail

sz_cookie               db 'Cookie: '
cookie                  db 0
                        rb 1024

paste_url               rb 1024
param                   rb 1024

identifier              dd 0
clipboard_data          dd 0
clipboard_data_length   dd 0

I_END:



