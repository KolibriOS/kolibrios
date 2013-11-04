;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2009-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  downloader.asm - HTTP client for KolibriOS                     ;;
;;                                                                 ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

URLMAXLEN       = 1024
BUFFERSIZE      = 4096

__DEBUG__       = 1
__DEBUG_LEVEL__ = 1

format binary as ""

use32
        org     0x0

        db      'MENUET01'      ; header
        dd      0x01            ; header version
        dd      START           ; entry point
        dd      IM_END          ; image size
        dd      I_END+0x1000    ; required memory
        dd      I_END+0x1000    ; esp
        dd      params          ; I_PARAM
        dd      0x0             ; I_Path

include '../../macros.inc'
include '../../proc32.inc'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../dll.inc'
include '../../debug-fdo.inc'
include '../../develop/libraries/http/http.inc'

virtual at 0
        http_msg http_msg
end virtual

START:

        mcall   68, 11                  ; init heap so we can allocate memory dynamically

; load libraries
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     exit

; check parameters
        cmp     byte[params], 0         ; no parameters ?
        je      reset_events            ; load the GUI

download:

        DEBUGF  1, "Starting download\n"

        invoke  HTTP_get, params
        test    eax, eax
        jz      fail
        mov     [identifier], eax

  .loop:
        invoke  HTTP_process, [identifier]
        test    eax, eax
        jnz     .loop

reset_events:
        DEBUGF  1, "resetting events\n"

; Report events
; defaults + mouse
        mcall   40,EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER

redraw:
        call    draw_window

still:
;;        DEBUGF  1, "waiting for events\n"

        mcall   10      ; wait here for event

        cmp     eax, EV_REDRAW
        je      redraw

        cmp     eax, EV_KEY
        je      key

        cmp     eax, EV_BUTTON
        je      button
        
        cmp     eax, EV_MOUSE
        je      mouse

        jmp     still

key:
        mcall   2       ; read key

        stdcall [edit_box_key], dword edit1

        cmp     ax, 13 shl 8
        je      download
        
        jmp     still
        
button:

        mcall   17      ; get id

        cmp     ah, 26
        jne     @f
        call    save
        jmp     still
  @@:
        cmp     ah, 1   ; button id=1 ?
        je      exit

        jmp     download

mouse:
        stdcall [edit_box_mouse], edit1
        jmp     still

exit:
        DEBUGF  1, "Exiting\n"
        mcall   68, 13, [identifier]    ; free buffer
fail:
        or      eax, -1 ; close this program
        mcall


save:
        mov     ebp, [identifier]
        mov     eax, [ebp + http_msg.content_length]
        mov     [final_size], eax
        lea     ebx, [ebp + http_msg.data]
        add     ebx, [ebp + http_msg.header_length]
        mov     [final_buffer], ebx
        mcall   70, fileinfo

  .done:

; TODO: if called from command line, then exit

        mov     ecx, [sc.work_text]
        or      ecx, 0x80000000
        mcall   4, <10, 93>, , download_complete

        ret

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************

draw_window:

        mcall   12, 1   ; start window draw

; get system colors
        mcall   48, 3, sc, 40

; draw window
        mov     edx, [sc.work]
        or      edx, 0x34000000
        mcall   0, <50, 370>, <350, 140>, , 0, title

; draw "url:" text
        mov     ecx, [sc.work_text]
        or      ecx, 80000000h
        mcall   4, <14, 14>, , type_pls

; draw editbox
        edit_boxes_set_sys_color edit1, editboxes_end, sc
        stdcall [edit_box_draw], edit1

; draw buttons
        mcall   8, <90, 68>, <54, 16>, 22, [sc.work_button]     ; reload
        mcall   , <166, 50>, <54, 16>, 24                       ; stop
        mcall   , <224, 54>, , 26                               ; save

; draw buttons text
        mov     ecx, [sc.work_button_text]
        or      ecx, 80000000h
        mcall   4, <102, 59>, , button_text

        mcall   12, 2   ; end window redraw

        ret


;-----------------------------------------------------------------------------
; Data area
;-----------------------------------------------------------------------------
align   4
@IMPORT:

library lib_http,       'http.obj', \
        box_lib,        'box_lib.obj'

import  lib_http, \
        HTTP_get                , 'get' , \
        find_header_field       , 'find_header_field' , \
        HTTP_process            , 'process'

import  box_lib, \
        edit_box_draw,  'edit_box', \
        edit_box_key,   'edit_box_key', \
        edit_box_mouse, 'edit_box_mouse'

;---------------------------------------------------------------------
fileinfo        dd 2, 0, 0
final_size      dd 0
final_buffer    dd 0
                db '/rd/1/.download', 0
        
;---------------------------------------------------------------------

mouse_dd        dd 0
edit1           edit_box 295, 48, 10, 0xffffff, 0xff, 0x80ff, 0, 0x8000, URLMAXLEN, document_user, mouse_dd, ed_focus+ed_always_focus, 7, 7
editboxes_end:

;---------------------------------------------------------------------

include_debug_strings

;---------------------------------------------------------------------

type_pls        db 'URL:', 0
button_text     db 'DOWNLOAD     STOP     RESAVE', 0
download_complete db 'File saved as /rd/1/.download', 0
title           db 'HTTP Downloader', 0

;---------------------------------------------------------------------
document_user   db 'http://'
;---------------------------------------------------------------------
IM_END:
;---------------------------------------------------------------------
params          rb URLMAXLEN
;---------------------------------------------------------------------
                sc system_colors
;---------------------------------------------------------------------
identifier      dd ?
;---------------------------------------------------------------------

I_END:



