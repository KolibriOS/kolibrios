; SPDX-License-Identifier: GPL-2.0
; DBGBOARD - a console-based debug board
; Copyright (C) 2025 KolibriOS team

format binary as ""
use32
org 0
db      'MENUET01'    ; signature
dd      1             ; header version
dd      start         ; entry point
dd      _image_end    ; end of image
dd      _memory       ; required memory size
dd      _stacktop     ; address of stack top
dd      _cmdline      ; buffer for command line arguments
dd      0             ; buffer for path

; __DEBUG__       = 1
; __DEBUG_LEVEL__ = DBG_ALL
; DBG_ALL       = 0  ; all messages
; DBG_INFO      = 1  ; info and errors
; DBG_ERR       = 2  ; only errors

include '../../macros.inc'
purge mov,add,sub
include '../../KOSfuncs.inc'
; include '../../debug-fdo.inc'
include '../../proc32.inc'
include '../../dll.inc'
include '../../if.inc'
include '../../string.inc'

CON_WINDOW_CLOSED = 0x200

TEXT_COLOR_YELLOW = 14
TEXT_COLOR_WHITE  = 15
TEXT_COLOR_LIGHTGRAY = 7
; TEXT_COLOR_LIGHTGREEN = 10

proc set_text_color stdcall uses eax, _color
        and     [_color], 0x0F
        invoke  con_get_flags
        and     eax, 0x3F0
        or      eax, [_color]
        invoke  con_set_flags, eax
        ret
endp

; returns eax = 0 or fs error code
proc get_file_attrib stdcall uses ebx, _path, _buf
        push    [_path]
        dec     esp
        mov     byte[esp], 0
        push    [_buf] ; 40 bytes buf
        push    0
        push    0
        push    0
        push    SSF_GET_INFO
        mov     ebx, esp
        mcall   SF_FILE
        add     esp, 25
        ret
endp

; returns eax = 0 or fs error code
proc create_file stdcall uses ebx, _path
        push   [_path]
        dec    esp
        mov    byte[esp], 0
        push   0
        push   0
        push   0
        push   0
        push   SSF_CREATE_FILE
        mov    ebx, esp
        mcall  SF_FILE
        add    esp, 25
        ret
endp

; returns eax = 0 or fs error code
proc write_file stdcall uses ebx, _path, _buf, _count, _pos_lo, _pos_hi, _out_bytes_written
        push   [_path]
        dec    esp
        mov    byte[esp], 0
        push   [_buf]
        push   [_count]
        push   [_pos_hi]
        push   [_pos_lo]
        push   SSF_WRITE_FILE
        mov    ebx, esp
        mcall  SF_FILE
        add    esp, 25
        mov    ecx, [_out_bytes_written]
        mov    [ecx], ebx
        ret
endp

start:
        ;; if there is a second instance of conboard is running then exit
        mcall   SF_THREAD_INFO, thread_info, -1
        stdcall string.copy, thread_info + process_information.process_name, thread_name
        stdcall string.to_lower_case, thread_name
        xor     edx, edx ; conboard instance count
        xor     esi, esi
        .while esi < 256 ; NOTE: add to macros.inc MAX_THREAD_COUNT = 256
            mcall SF_THREAD_INFO, thread_info, esi
            .if [thread_info + process_information.slot_state] <> TSTATE_FREE
                stdcall string.to_lower_case, thread_info + process_information.process_name
                stdcall string.cmp, thread_info + process_information.process_name, thread_name, -1
                .if eax = 0
                    inc edx
                    .if edx >= 2
                        jmp .raw_exit
                    .endif
                .endif
            .endif
            inc esi
        .endw

        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     .exit

        invoke  con_start, 1
        invoke  con_init, 80, 32, -1, -1, title

        .if byte [_cmdline] <> 0
            mov [log_file_path], _cmdline
        .endif

.main_loop:
        invoke con_kbhit
        .if eax = 1
            invoke con_getch2
            .if ah = 60 ; F2
                mov eax, [log_file_path]
                mov [struct_open_in_notepad.filename], eax
                mcall SF_FILE, struct_open_in_notepad
            .endif
        .endif

        mcall   SF_BOARD, SSF_DEBUG_READ
        .if ebx = 0
            mcall SF_SLEEP, 50
            jz .main_loop_cond
        .endif
        mov     [chr], al

        .if [is_start_line] = 1
            mov eax, prefix
            add eax, [prefix_index]
            mov bl, byte [chr]
            mov [eax], bl
            .if [prefix_index] = 2
                .if dword [prefix] = 'K :'
                    stdcall set_text_color, TEXT_COLOR_YELLOW
                .elseif dword [prefix] = 'L: '
                    stdcall set_text_color, TEXT_COLOR_WHITE
                .else
                    stdcall set_text_color, TEXT_COLOR_LIGHTGRAY
                .endif
                mov     [is_start_line], 0
                mov     [prefix_index], 0
                invoke  con_write_asciiz, prefix
                mov     dword [prefix], 0
            .else
                inc [prefix_index]
            .endif
        .else
            invoke con_write_asciiz, chr
            .if byte [chr] = 10
                mov [is_start_line], 1
                stdcall set_text_color, TEXT_COLOR_LIGHTGRAY
            .endif
        .endif

        stdcall get_file_attrib, [log_file_path], file_info_buf
        .if eax = 5 ; file not found
            stdcall create_file, [log_file_path]
        .endif
        stdcall write_file, [log_file_path], chr, 1, dword [file_info_buf + 32], dword [file_info_buf + 32 + 4], bytes_written

.main_loop_cond:
        invoke  con_get_flags
        and     eax, CON_WINDOW_CLOSED
        test    eax, eax
        jz      .main_loop

.exit:
        invoke  con_exit, 0
.raw_exit:
        mcall   SF_TERMINATE_PROCESS


; data:
title         db 'Debug & message board',0
log_file_path dd default_log_file_path
default_log_file_path db '/tmp0/1/BOARDLOG.TXT',0
prefix        db 0,0,0,0
prefix_index  dd 0
is_start_line dd 1
bytes_written dd 0
chr           db 0, 0

struct_open_in_notepad:
        dd SSF_START_APP
        dd 0
        .filename dd ?
        dd 0
        dd 0
        db '/sys/develop/cedit', 0


; include_debug_strings

align 4
@IMPORT:
library console, 'console.obj'
import  console, \
        con_start,      'START', \
        con_init,       'con_init', \
        con_write_asciiz, 'con_write_asciiz', \
        con_exit,       'con_exit', \
        con_kbhit,      'con_kbhit', \
        con_getch2,     'con_getch2', \
        con_set_flags,  'con_set_flags', \
        con_get_flags,  'con_get_flags'

align 16
_image_end:

file_info_buf rb 40

align 4
_cmdline rb 256

thread_info     process_information
thread_name     rb 16

; reserve for stack:
rb      4096
align 16
_stacktop:
_memory:
