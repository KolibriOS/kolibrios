;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2014-2017. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  pasta.asm - Paste text to dpaste.com from a file or from       ;;
;;              clipboard.                                         ;;
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
        cmp     eax, 1
        je      error

; Verify if we can work with it
        mov     [clipboard_data], eax
        cmp     dword[eax + 4], 0               ; text ?
        jne     error_free_clip

; Save length in [clipboard_data_length]
        mov     ecx, dword[eax]
        sub     ecx, 12
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

; Post to the server
        mov     ecx, [clipboard_data_length]
        add     ecx, sz_paste_head.length
        invoke  HTTP_post, sz_url, 0, FLAG_BLOCK, 0, sz_ctype, ecx
        test    eax, eax
        jz      error_free_clip_disconnect
        mov     [identifier], eax

; Send the data to the server
        invoke  HTTP_send, [identifier], sz_paste_head, sz_paste_head.length

        push    [clipboard_data]
        pop     [send_ptr]
  .again:
        invoke  HTTP_send, [identifier], [send_ptr], [clipboard_data_length]
        cmp     eax, -1
        je      error_free_clip_disconnect
        test    eax, eax
        jz      error_free_clip_disconnect
        add     [send_ptr], eax
        sub     [clipboard_data_length], eax
        ja      .again

; Free the data
        mcall   68, 13, [clipboard_data]

  .again2:
        invoke  HTTP_receive, [identifier]
        test    eax, eax
        jnz     .again2

        invoke  HTTP_disconnect, [identifier]

        mov     ebp, [identifier]
        test    [ebp + http_msg.flags], 0xffff0000             ; Test error bits
        jnz     error_free_http
        test    [ebp + http_msg.flags], FLAG_GOT_HEADER
        jz      error_free_http
        cmp     [ebp + http_msg.status], 201    ; created
        jne     error_http_code

        invoke  HTTP_find_header_field, [identifier], sz_location
        test    eax, eax
        jz      error_free_http

        mov     esi, eax
        call    notify

        mcall   54, 3                   ; Delete last slot in the clipboard

        mov     dword[notify_msg-4], ecx
        mov     dword[notify_msg+0], 0   ; Text
        mov     dword[notify_msg+4], 1   ; cp0866
        mcall   54, 2, , notify_msg-4    ; Write URL to the clipboard

        invoke  HTTP_free, [identifier]
        mcall   -1

error_http_code:
        lea     esi, [ebp + http_msg.http_header]
        call    notify
error_free_http:
        invoke  HTTP_free, [identifier]
        jmp     error
error_free_clip_disconnect:
        invoke  HTTP_disconnect, [identifier]
        invoke  HTTP_free, [identifier]
error_free_clip:
        mcall   68, 13, [clipboard_data]
error:
        mov     [notify_struct.msg], sz_failed
        mcall   70, notify_struct

        mcall   -1


notify:

        mov     edi, notify_msg.text
  .msg_loop:
        lodsb
        stosb
        cmp     al, 13
        je      .msg_end
        cmp     al, 10
        je      .msg_end
        cmp     al, 0
        je      .msg_end
        jmp     .msg_loop
  .msg_end:
        dec     edi
        lea     ecx, [edi - notify_msg + 4]
        mov     eax, '" -t'
        stosd
        mov     ax, 'O'
        stosw
        mov     [notify_struct.msg], notify_msg
        mcall   70, notify_struct

        ret


;---------------------------------------------------------------------
; Data area
;-----------------------------------------------------------------------------
align   4
@IMPORT:

library lib_http,               'http.obj'

import  lib_http, \
        HTTP_get,               'get', \
        HTTP_post,              'post', \
        HTTP_send,              'send', \
        HTTP_receive,           'receive', \
        HTTP_find_header_field, 'find_header_field', \
        HTTP_free,              'free', \
        HTTP_escape,            'escape', \
        HTTP_disconnect,        'disconnect'

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
  .msg  dd notify_msg
        dd 0
        dd 0
        db '/sys/@notify', 0

sz_url                  db 'http://dpaste.com/api/v2/', 0
sz_location             db 'location', 0
sz_ctype                db 'application/x-www-form-urlencoded', 0
sz_failed               db '"Pasta!',10,'Paste failed!" -E', 0

sz_paste_head           db 'content='
.length = $ - sz_paste_head

notify_msg              db '"Pasta!',10
  .text                 rb 1024

param                   rb 1024

identifier              dd 0
clipboard_data          dd 0
clipboard_data_length   dd 0
send_ptr                dd ?

I_END:



