;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2014-2017. All rights reserved.    ;;
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
        jnz     mainloop.exit

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
        jne     display_url_and_download

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
        je      .exit

        cmp     [btn_text], sz_download
        je      download

        cmp     [btn_text], sz_open
        je      open_file

  .exit:
        mcall   -1      ; exit

  .mouse:
        invoke  edit_box_mouse, edit1
        jmp     mainloop


open_file:
        mcall   70, fileopen
        jmp     mainloop

display_url_and_download:
        xor     al, al
        mov     ecx, 4096
        mov     edi, url
        repne scasb
        sub     edi, url+1
        mov     [edit1.size], edi

download:
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

; Create the local file
        mov     [fileinfo], 2           ; create/write to file
        xor     eax, eax
        mov     [fileinfo.offset], eax
        mov     [fileinfo.offset+4], eax
        mov     [fileinfo.size], eax
        mcall   70, fileinfo
        test    eax, eax
        jnz     create_error

; Start the download
        invoke  HTTP_get, url, 0, FLAG_STREAM or FLAG_REUSE_BUFFER, 0
        test    eax, eax
        jz      get_error

        mov     [identifier], eax
        mov     [offset], 0
        mov     [btn_text], sz_cancel
        mov     [status], sz_downloading
        or      [edit1.flags], ed_figure_only
        and     [edit1.flags], not ed_focus
        push    [sc.work]
        pop     [edit1.color]
        call    draw_window

        jmp     download_loop

get_error:
        mov     [btn_text], sz_exit
        mov     [status], sz_err_http
        jmp     redraw

create_error:
        mov     [btn_text], sz_exit
        mov     [status], sz_err_create
        jmp     redraw

download_loop:
        mcall   10
        cmp     eax, EV_REDRAW
        je      .redraw
        cmp     eax, EV_BUTTON
        je      .button

        invoke  HTTP_receive, [identifier]
        test    eax, eax
        jz      got_data
        jmp     download_loop

  .redraw:
        call    draw_window
        jmp     download_loop

  .button:
        jmp     http_free

got_data:
        mov     ebp, [identifier]
        test    [ebp + http_msg.flags], 0xffff0000      ; error?
        jnz     http_error

        cmp     [fileinfo], 3                           ; Did we write before?
        je      .write

        test    [ebp + http_msg.flags], FLAG_CONTENT_LENGTH
        jz      .first_write

        mov     eax, [ebp + http_msg.content_length]
        mov     [pb.max], eax

        DEBUGF  1, "new file size=%u\n", eax
        mov     [fileinfo], 4                           ; set end of file
        mov     [fileinfo.offset], eax                  ; new file size
        mcall   70, fileinfo
        test    eax, eax
        jnz     write_error


  .first_write:
        mov     [fileinfo], 3                           ; write to existing file
  .write:
        mov     ecx, [ebp + http_msg.content_received]
        sub     ecx, [offset]
        jz      .no_data                                ; more then 0 data bytes?

        mov     [fileinfo.size], ecx
        mov     eax, [ebp + http_msg.content_ptr]
        mov     [fileinfo.buffer], eax
        mov     ebx, [offset]
        mov     [fileinfo.offset], ebx
        DEBUGF  1, "Writing to disk: size=%u offset=%u\n", ecx, ebx
        mcall   70, fileinfo
        test    eax, eax                                ; check error code
        jnz     write_error
        cmp     ebx, ecx                                ; check if all bytes were written to disk
        jne     write_error

        mov     eax, [ebp + http_msg.content_received]
        mov     [offset], eax
        mov     [pb.value], eax

        invoke  progressbar_draw, pb

  .no_data:
        test    [ebp + http_msg.flags], FLAG_GOT_ALL_DATA
        jz      download_loop

; Download completed successfully
        mov     [status], sz_complete
        mov     [pb.progress_color], 0x0000c800         ; green
        mov     [btn_text], sz_open
        jmp     http_free

write_error:
        mov     [status], sz_err_full
        mov     [pb.progress_color], 0x00c80000         ; red
        mov     [btn_text], sz_exit
        jmp     http_free

http_error:
        mov     [status], sz_err_http
        mov     [pb.progress_color], 0x00c80000         ; red
        mov     [btn_text], sz_exit
;        jmp     http_free

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
        jmp     redraw

draw_window:
        mcall   12, 1   ; start window draw

; get system colors
        mcall   48, 3, sc, 40

; draw window
        mov     edx, [sc.work]
        or      edx, 0x34000000
        mcall   0, <50, 420>, <350, 120>, , 0, title

; draw button
        mcall   8, <320,75>, <50,24>, 22, [sc.work_button]

; draw button text
        mov     ecx, [sc.work_button_text]
        or      ecx, 90000000h
        mcall   4, <325,56>, , [btn_text]

; draw status text
        mov     ecx, [sc.work_text]
        or      ecx, 90000000h
        mcall   4, <10,70>, , [status]

; draw editbox
        edit_boxes_set_sys_color edit1, editboxes_end, sc
        invoke  edit_box_draw, edit1

        cmp     [identifier], 0
        je     @f
; draw progressbar
        invoke  progressbar_draw, pb
  @@:
        mcall   12, 2   ; end window draw

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
        edit_box_draw,    'edit_box_draw', \
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

fileopen        dd 7
                dd 0                    ; flags
                dd fname_buf            ; parameters
                dd 0                    ; reserved
                dd 0                    ; reserved
                db "/sys/@open", 0      ; path

edit1           edit_box 400, 5, 10, 0xffffff, 0x0000ff, 0x0080ff, 0x000000, 0x90000000, URLMAXLEN, url, mouse_dd, ed_focus+ed_always_focus, 0, 0
editboxes_end:

identifier      dd 0
btn_text        dd sz_download
status          dd sz_null
sz_download     db 'Download', 0
sz_cancel       db ' Cancel ', 0
sz_open         db '  Open  ', 0
sz_exit         db '  Exit  ', 0

sz_null         db 0
sz_downloading  db 'Downloading..', 0
sz_complete     db 'Download completed', 0
sz_err_create   db 'Could not create the local file!', 0
sz_err_full     db 'Disk full!', 0
sz_err_http     db 'HTTP error!', 0
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
.y_start                dw 140  ; Window Y position

communication_area_name         db 'FFFFFFFF_open_dialog',0
open_dialog_path                db '/sys/File Managers/opendial',0
communication_area_default_path db '/sys',0

filter:
dd      0
db      0

pb:
.value          dd 0
.left           dd 5
.top            dd 45
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