;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2014-2015. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  downloader.asm - HTTP client for KolibriOS                     ;;
;;                                                                 ;;
;;      hidnplayr@kolibrios.org                                    ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

URLMAXLEN       = 65535

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
        dd      url
        dd      0x0             ; I_Path


include '../../macros.inc'
include '../../proc32.inc'
include '../../dll.inc'
include '../../debug-fdo.inc'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
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

; wanted events
        mcall   40, EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER

; prepare filename buffers
        mov     edi, fname_buf
        mov     esi, download_file_path
  @@:
        lodsb
        stosb
        test    al, al
        jnz     @r

; Initialise OpenDialog
        invoke  OpenDialog_Init, OpenDialog_data

; If user provided parameters, start download right away!
        cmp     byte[url], 0
        jne     download

        mov     [OpenDialog_data.draw_window], draw_window

redraw:
        call    draw_window

mainloop:
        mcall   10      ; wait here for event
        cmp     eax, EV_REDRAW
        je      redraw
        cmp     eax, EV_KEY
        je      .key
        cmp     eax, EV_BUTTON
        je      .button
        cmp     eax, EV_MOUSE
        je      .mouse
        jmp     mainloop

  .key:
        mcall   2       ; read key
        invoke  edit_box_key, edit1
        cmp     ax, 13 shl 8
        je      download
        jmp     mainloop

  .button:
        mcall   17      ; get id
        cmp     ah, 1   ; button id=1 ?
        je      exit

        cmp     [btn_text], sz_download
        je      download

        cmp     [btn_text], sz_open
        je      open_file

  .mouse:
        invoke  edit_box_mouse, edit1
        jmp     mainloop

open_file:
;        mcall   70, ...
        jmp     mainloop

exit:
error:
        mcall   -1      ; exit

download:
;        int 3
; Extract the filename from URL
        mov     edi, url
        xor     al, al
        mov     ecx, URLMAXLEN
        repne scasb
        mov     esi, edi
        dec     esi
        dec     esi
        std
  .loop:
        lodsb
        cmp     al, '/'
        je      .done
        test    al, al
        jnz     .loop
  .done:
        cld
        mov     ecx, edi
        sub     ecx, esi
        inc     esi
        inc     esi
        mov     edi, filename_area
        rep movsb

; invoke OpenDialog
        invoke  OpenDialog_Start, OpenDialog_data
        mcall   40, EVM_REDRAW + EVM_BUTTON + EVM_STACK
        call    draw_window

; Create the file
        mov     [fileinfo], 2           ; create/write to file
        xor     eax, eax
        mov     [fileinfo.offset], eax
        mov     [fileinfo.offset+4], eax
        mov     [fileinfo.size], eax
        mcall   70, fileinfo
        test    eax, eax
        jnz     error                   ; TODO: print error message

; And start the download
        invoke  HTTP_get, url, 0, FLAG_STREAM or FLAG_REUSE_BUFFER, 0
        test    eax, eax
        jz      error                   ; TODO: print error message
        mov     [identifier], eax
        mov     [offset], 0
        mov     [btn_text], sz_cancel

        or      [edit1.flags], ed_figure_only
        call    draw_window

download_loop:
        mcall   10
        cmp     eax, EV_REDRAW
        je      .redraw
        cmp     eax, EV_BUTTON
        je      .button

        invoke  HTTP_receive, [identifier]
        test    eax, eax
        jz      save_chunk

        mov     eax, [identifier]
        push    [eax + http_msg.content_length]
        pop     [pb.max]
        push    [eax + http_msg.content_received]
        pop     [pb.value]

        invoke  progressbar_draw, pb
        jmp     download_loop

  .redraw:
        call    draw_window
        jmp     download_loop

  .button:
        jmp     http_free

save_chunk:
        mov     ebp, [identifier]
        test    [ebp + http_msg.flags], 0xffff0000      ; error?
        jnz     http_free

        cmp     [fileinfo], 3
        je      @f
        DEBUGF  1, "new file size=%u\n", [ebp + http_msg.content_length]
        mov     [fileinfo], 4                           ; set end of file
        mov     eax, [ebp + http_msg.content_length]
        mov     [fileinfo.offset], eax                  ; new file size
        mcall   70, fileinfo

        mov     [fileinfo], 3                           ; write to existing file
  @@:
        mov     ecx, [ebp + http_msg.content_received]
        sub     ecx, [offset]
        mov     [fileinfo.size], ecx
        mov     eax, [ebp + http_msg.content_ptr]
        mov     [fileinfo.buffer], eax
        mov     ebx, [offset]
        mov     [fileinfo.offset], ebx
        DEBUGF  1, "Writing to disk: size=%u offset=%u\n", ecx, ebx
        mcall   70, fileinfo

        mov     eax, [ebp + http_msg.content_received]
        mov     [offset], eax

        test    [ebp + http_msg.flags], FLAG_GOT_ALL_DATA
        jz      download_loop

        mov     [pb.progress_color], 0x0000c800         ; green

http_free:
        mcall   40, EVM_REDRAW + EVM_BUTTON
        push    [ebp + http_msg.content_received]
        pop     [pb.value]

        mov     ecx, [ebp + http_msg.content_ptr]
        test    ecx, ecx
        jz      @f
        mcall   68, 13                                  ; free the buffer
  @@:

        invoke  HTTP_free, [identifier]                 ; free headers and connection

        mov     [btn_text], sz_open
        call    draw_window
        jmp     mainloop



draw_window:
        mcall   12, 1   ; start window draw

; get system colors
        mcall   48, 3, sc, 40

; draw window
        mov     edx, [sc.work]
        or      edx, 0x34000000
        mcall   0, <50, 320>, <350, 110>, , 0, title

; draw button
        mcall   8, <229,75>, <60,16>, 22, [sc.work_button]      ; download

; draw button text
        mov     ecx, [sc.work_button_text]
        or      ecx, 80000000h
        mcall   4, <240,65>, , [btn_text]

; draw editbox
        edit_boxes_set_sys_color edit1, editboxes_end, sc
        invoke  edit_box_draw, edit1

        cmp     [identifier], 0
        je     @f
; draw progressbar
        invoke  progressbar_draw, pb
  @@:
        mcall   12, 2   ; end window draw
        ret


dont_draw:

        ret

;---------------------------------------------------------------------
; Data area
;-----------------------------------------------------------------------------
align   4
@IMPORT:

library lib_http,       'http.obj', \
        box_lib,        'box_lib.obj', \
        proc_lib,       'proc_lib.obj'

import  lib_http, \
        HTTP_get,       'get', \
        HTTP_receive,   'receive', \
        HTTP_free,      'free'

import  box_lib, \
        edit_box_draw,    'edit_box', \
        edit_box_key,     'edit_box_key', \
        edit_box_mouse,   'edit_box_mouse', \
        progressbar_draw, 'progressbar_draw', \
        progressbar_prog, 'progressbar_progress'

import  proc_lib, \
        OpenDialog_Init,  'OpenDialog_init', \
        OpenDialog_Start, 'OpenDialog_start'


fileinfo        dd 2
  .offset       dd 0, 0
  .size         dd 0
  .buffer       dd 0
                db 0
                dd fname_buf

edit1           edit_box 299, 5, 10, 0xffffff, 0x0000ff, 0x0080ff, 0x000000, 0x8000, URLMAXLEN, url, mouse_dd, ed_focus+ed_always_focus, 0, 0
editboxes_end:

identifier      dd 0
btn_text        dd sz_download
sz_download     db 'Download', 0
sz_cancel       db ' Cancel ', 0
sz_open         db '  Open  ', 0
title           db 'HTTP Downloader', 0

OpenDialog_data:
.type                   dd 1    ; Save
.procinfo               dd procinfo
.com_area_name          dd communication_area_name
.com_area               dd 0
.opendir_path           dd temp_dir_path
.dir_default_path       dd communication_area_default_path
.start_path             dd open_dialog_path
.draw_window            dd dont_draw
.status                 dd 0
.openfile_patch         dd fname_buf
.filename_area          dd filename_area
.filter_area            dd filter
.x:
.x_size                 dw 420  ; Window X size
.x_start                dw 200  ; Window X position
.y:
.y_size                 dw 320  ; Window y size
.y_start                dw 120  ; Window Y position

communication_area_name         db 'FFFFFFFF_open_dialog',0
open_dialog_path                db '/sys/File Managers/opendial',0
communication_area_default_path db '/sys',0

filter:
dd      0
db      0

pb:
.value          dd 0
.left           dd 5
.top            dd 35
.width          dd 300
.height         dd 16
.style          dd 1
.min            dd 0
.max            dd 0
.back_color     dd 0xefefef
.progress_color dd 0xc8c8c8
.frame_color    dd 0x94aece
.frame_color2   dd 0xffffff

include_debug_strings

download_file_path db '/tmp0/1/', 0

IM_END:

url             rb URLMAXLEN
sc              system_colors
offset          dd ?
mouse_dd        dd ?

filename_area   rb 256
temp_dir_path   rb 4096
procinfo        rb 1024
fname_buf       rb 4096
text_work_area  rb 1024

I_END: