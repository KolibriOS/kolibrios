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
;--------------------------------------
frame_1:
  .x      = 5
  .y      = 10
  .width  = 350
  .height = 55
;--------------------------------------  
frame_2:
  .x      = 5
  .y      = 75
  .width  = 350
  .height = 55
;---------------------------------------------------------------------
use32
        org     0x0

        db      'MENUET01'      ; header
        dd      0x01            ; header version
        dd      START           ; entry point
        dd      IM_END          ; image size
        dd      I_END           ; required memory
        dd      stacktop        ; esp
        dd      params          ; I_PARAM
        dd      0x0             ; I_Path
;---------------------------------------------------------------------
include '../../../../macros.inc'
include '../../../../proc32.inc'
include '../../../../dll.inc'
include '../../../../debug-fdo.inc'
include '../../box_lib/trunk/box_lib.mac'
include '../../http/http.inc'

virtual at 0
        http_msg http_msg
end virtual
;---------------------------------------------------------------------
START:

        mcall   68, 11                  ; init heap so we can allocate memory dynamically

; load libraries
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     exit
;---------------------------------------------------------------------
	mov	edi,filename_area
	mov	esi,start_temp_file_name
	call	copy_file_name_path

	mov	edi,fname_buf
	mov	esi,start_file_path
	call	copy_file_name_path

;OpenDialog     initialisation
        push    dword OpenDialog_data
        call    [OpenDialog_Init]

; prepare for PathShow
        push    dword PathShow_data_1
        call    [PathShow_prepare]
;---------------------------------------------------------------------
; check parameters
        cmp     byte[params], 0         ; no parameters ?
        je      reset_events            ; load the GUI

        inc     [silently]

download:
	call	download_1

        test    [silently], 0xff
        jnz     save

reset_events:
        DEBUGF  1, "resetting events\n"

; Report events
; defaults + mouse
        mcall   40,EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER
;---------------------------------------------------------------------
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
;---------------------------------------------------------------------
key:
        mcall   2       ; read key

        stdcall [edit_box_key], dword edit1

        cmp     ax, 13 shl 8
        je      download
        
        jmp     still
;---------------------------------------------------------------------        
button:

        mcall   17      ; get id

        cmp     ah, 26
        jne     @f
; invoke OpenDialog
        push    dword OpenDialog_data
        call    [OpenDialog_Start]
        cmp     [OpenDialog_data.status],1
        jne     still

; prepare for PathShow
        push    dword PathShow_data_1
        call    [PathShow_prepare]
        call    draw_window
        jmp     still
@@:
        cmp     ah, 1   ; button id=1 ?
        je      exit

	call	download_1
        jmp     save
;---------------------------------------------------------------------
mouse:
        stdcall [edit_box_mouse], edit1
        jmp     still
;---------------------------------------------------------------------
exit:
        DEBUGF  1, "Exiting\n"
        invoke  HTTP_free, [identifier] ; free buffer
fail:
        or      eax, -1 ; close this program
        mcall
;---------------------------------------------------------------------
download_1:
        DEBUGF  1, "Starting download\n"

        invoke  HTTP_get, params, 0
        test    eax, eax
        jz      fail
        mov     [identifier], eax

  .loop:
        invoke  HTTP_process, [identifier]
        test    eax, eax
        jnz     .loop
	ret
;---------------------------------------------------------------------
save:
        mov     ebp, [identifier]
        mov     eax, [ebp + http_msg.content_received]
        mov     [final_size], eax
        lea     ebx, [ebp + http_msg.data]
        add     ebx, [ebp + http_msg.header_length]
        mov     [final_buffer], ebx
        mcall   70, fileinfo

        DEBUGF  1, "File saved\n"

        test    [silently], 0xff
        jnz     exit

        mov     ecx, [sc.work_text]
        or      ecx, 0x80000000
        mcall   4, <10, frame_2.y+frame_2.height+7>, , download_complete

        jmp     still
;---------------------------------------------------------------------
copy_file_name_path:
	xor	eax,eax
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@r
	ret
;---------------------------------------------------------------------
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************

draw_window:

        mcall   12, 1   ; start window draw
;-----------------------------------
; get system colors
        mcall   48, 3, sc, 40
;-----------------------------------
; draw window
        mov     edx, [sc.work]
        or      edx, 0x34000000
        mcall   0, <50, 370>, <350, 170>, , 0, title
;-----------------------------------
; draw frames
	mov	[frame_data.x],dword frame_1.x shl 16+frame_1.width
	mov	[frame_data.y],dword frame_1.y shl 16+frame_1.height
	mov	[frame_data.text_pointer],dword select_addr_text
	mov	eax,[sc.work]
	mov	[frame_data.font_backgr_color],eax
	mov	eax,[sc.work_text]
	mov	[frame_data.font_color],eax
	
	push	dword frame_data
	call	[Frame_draw]
;-----------------------------------
	mov	[frame_data.x],dword frame_2.x shl 16+frame_2.width
	mov	[frame_data.y],dword frame_2.y shl 16+frame_2.height
	mov	[frame_data.text_pointer],dword select_path_text

	push	dword frame_data
	call	[Frame_draw]
;-----------------------------------
; draw "url:" text
        mov     ecx, [sc.work_text]
        or      ecx, 80000000h
        mcall   4, <frame_1.x+10, frame_1.y+15>, , type_pls
;-----------------------------------
; draw editbox
        edit_boxes_set_sys_color edit1, editboxes_end, sc
        stdcall [edit_box_draw], edit1
;-----------------------------------
; draw buttons
        mcall   8,<frame_1.x+frame_1.width-(68+15+50+15),68>,<frame_1.y+30,16>,22,[sc.work_button] ; reload
        mcall   ,<frame_1.x+frame_1.width-(50+15),50>,<frame_1.y+30,16>, 24 ; stop
	
        mcall   , <frame_2.x+frame_2.width-(54+15),54>,<frame_2.y+30,16>,26 ; save
;-----------------------------------
; draw buttons text
        mov     ecx, [sc.work_button_text]
        or      ecx, 80000000h
        mcall   4, <frame_1.x+frame_1.width-(68+15+50+15)+10,frame_1.y+35>, , button_text.1
        mcall   , <frame_1.x+frame_1.width-(50+15)+15,frame_1.y+35>, , button_text.2
        mcall   , <frame_2.x+frame_2.width-(54+15)+10,frame_2.y+35>, , button_text.3
	
        mcall   13,<frame_2.x+17,frame_2.width-15*2>,<frame_2.y+10,15>,0xffffff
        push    dword PathShow_data_1
        call    [PathShow_draw]
	
        mcall   12, 2   ; end window redraw

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
        HTTP_get                , 'get' , \
        HTTP_process            , 'process'     ,\
        HTTP_free               , 'free'

import  box_lib, \
        edit_box_draw,    'edit_box', \
        edit_box_key,     'edit_box_key', \
        edit_box_mouse,   'edit_box_mouse', \
        PathShow_prepare, 'PathShow_prepare', \
        PathShow_draw,    'PathShow_draw', \
        Frame_draw,       'frame_draw'

import  proc_lib, \
        OpenDialog_Init,  'OpenDialog_init', \
        OpenDialog_Start, 'OpenDialog_start'
;---------------------------------------------------------------------
fileinfo        dd 2, 0, 0
final_size      dd 0
final_buffer    dd 0
                db 0
                dd fname_buf
;---------------------------------------------------------------------

mouse_dd        dd 0
edit1           edit_box 295, 48, (frame_1.y+10), 0xffffff, 0xff, 0x80ff, 0, 0x8000, URLMAXLEN, document_user, mouse_dd, ed_focus+ed_always_focus, 7, 7
editboxes_end:

;---------------------------------------------------------------------

include_debug_strings

;---------------------------------------------------------------------

type_pls        db 'URL:', 0
button_text:
.1:             db 'DOWNLOAD',0
.2:             db 'STOP',0
.3:             db 'SELECT', 0
download_complete db 'FILE SAVED!', 0
title           db 'HTTP Downloader', 0
silently        db 0

;---------------------------------------------------------------------
select_addr_text db ' NETWORK ADDRESS: ',0
select_path_text db ' PATH TO SAVE FILE: ',0
;---------------------------------------------------------------------
frame_data:
.type			dd 0 ;+0
.x:
.x_size			dw 0 ;+4
.x_start		dw 0 ;+6
.y:
.y_size			dw 0 ;+8
.y_start		dw 0 ;+10
.ext_fr_col		dd 0x0 ;+12
.int_fr_col		dd 0xffffff ;+16
.draw_text_flag		dd 1 ;+20
.text_pointer		dd 0 ;+24
.text_position		dd 0 ;+28
.font_number		dd 0 ;+32
.font_size_y		dd 9 ;+36
.font_color		dd 0x0 ;+40
.font_backgr_color	dd 0xffffff ;+44
;---------------------------------------------------------------------
PathShow_data_1:
.type                   dd 0    ;+0
.start_y                dw frame_2.y+14   ;+4
.start_x                dw frame_2.x+20   ;+6
.font_size_x            dw 6    ;+8     ; 6 - for font 0, 8 - for font 1
.area_size_x            dw frame_2.width-35  ;+10
.font_number            dd 0    ;+12    ; 0 - monospace, 1 - variable
.background_flag        dd 0    ;+16
.font_color             dd 0    ;+20
.background_color       dd 0    ;+24
.text_pointer           dd fname_buf    ;+28
.work_area_pointer      dd text_work_area       ;+32
.temp_text_length       dd 0    ;+36
;---------------------------------------------------------------------
OpenDialog_data:
.type                   dd 1    ; Save
.procinfo               dd procinfo     ;+4
.com_area_name          dd communication_area_name      ;+8
.com_area               dd 0    ;+12
.opendir_path           dd temp_dir_path        ;+16
.dir_default_path       dd communication_area_default_path      ;+20
.start_path             dd open_dialog_path     ;+24
.draw_window            dd draw_window  ;+28
.status                 dd 0    ;+32
.openfile_pach          dd fname_buf    ;+36
.filename_area          dd filename_area        ;+40
.filter_area            dd Filter
.x:
.x_size                 dw 420 ;+48 ; Window X size
.x_start                dw 200 ;+50 ; Window X position
.y:
.y_size                 dw 320 ;+52 ; Window y size
.y_start                dw 120 ;+54 ; Window Y position

communication_area_name:
        db 'FFFFFFFF_open_dialog',0
open_dialog_path:
    db '/sys/File Managers/opendial',0
communication_area_default_path:
        db '/sys',0

Filter:
dd      Filter.end - Filter
.1:
db      'IMG',0
db      'IMA',0
.end:
db      0

start_temp_file_name:   db 'some.garbage',0

start_file_path:  db '/sys/.download', 0
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
filename_area:
                rb 256
;---------------------------------------------------------------------
temp_dir_path:
                rb 4096
;---------------------------------------------------------------------
procinfo:
                rb 1024
;---------------------------------------------------------------------
fname_buf:
                rb 4096
;---------------------------------------------------------------------
text_work_area:
                rb 1024
;---------------------------------------------------------------------
                rb 4096
stacktop:
;---------------------------------------------------------------------
I_END:
;---------------------------------------------------------------------


