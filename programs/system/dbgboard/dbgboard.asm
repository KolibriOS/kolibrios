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
include '../../struct.inc'
include '../../dll.inc'
include '../../if.inc'
include '../../string.inc'

CON_WINDOW_CLOSED = 0x200

TEXT_COLOR_LIGHTGRAY    = 7
TEXT_COLOR_LIGHTBLUE    = 9
TEXT_COLOR_LIGHTGREEN   = 10
TEXT_COLOR_LIGHTCYAN    = 11
TEXT_COLOR_LIGHTRED     = 12
TEXT_COLOR_LIGHTMAGENTA = 13
TEXT_COLOR_YELLOW       = 14
TEXT_COLOR_WHITE        = 15

RB_CAPACITY = 4096*6

MODES_COUNT = 3
MODE_USER   = 0
MODE_KERNEL = 1
MODE_BOTH   = 2

struct RING_BUFFER
        buffer       dd ?
        capacity     dd ?
        pos          dd ?
        bytes_filled dd ?
ends

proc set_text_color stdcall uses eax ecx edx, _color
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


proc ring_buffer_push_byte stdcall uses eax edx esi edi, _rb, _byte
        mov     esi, [_rb]
        mov     eax, [esi + RING_BUFFER.bytes_filled]
        .if eax < [esi + RING_BUFFER.capacity]
            inc [esi + RING_BUFFER.bytes_filled]
        .endif

        mov     edi, [esi + RING_BUFFER.buffer]
        add     edi, [esi + RING_BUFFER.pos]
        mov     eax, [_byte]
        mov     byte [edi], al

        mov     eax, [esi + RING_BUFFER.pos]
        inc     eax
        xor     edx, edx
        div     [esi + RING_BUFFER.capacity]
        mov     [esi + RING_BUFFER.pos], edx ; put remainder

        ret
endp

proc print_ring_buffer stdcall uses eax ebx ecx edx esi, _rb
        mov     esi, [_rb]
        mov     eax, [esi + RING_BUFFER.capacity]
        xor     ebx, ebx
        .if eax = [esi + RING_BUFFER.bytes_filled]
            mov ebx, [esi + RING_BUFFER.pos]
        .endif
        xor     ecx, ecx
        .while  ecx < [esi + RING_BUFFER.bytes_filled]
            mov eax, ebx
            add eax, ecx
            xor edx, edx
            div [esi + RING_BUFFER.capacity]
            mov eax, [esi + RING_BUFFER.buffer]
            add eax, edx
            mov eax, [eax]
            mov byte [chr], al
            stdcall print_next_char, chr
            inc ecx
        .endw
        ret
endp

; in - __chr
proc push_to_buffers_next_char uses ebx
        .if [__is_start_line] = 1
            mov eax, __prefix
            add eax, [__prefix_index]
            mov bl, byte [__chr]
            mov [eax], bl
            .if [__prefix_index] = 2
                .if dword [__prefix] = 'K :'
                    mov [current_rb], rb_kernel
                .elseif dword [__prefix] = 'K: '
                    mov [current_rb], rb_kernel
                .else
                    mov [current_rb], rb_user
                .endif
                mov     [__is_start_line], 0
                mov     [__prefix_index], 0
                movzx   eax, byte [__prefix]
                stdcall ring_buffer_push_byte, [current_rb], eax
                movzx   eax, byte [__prefix + 1]
                stdcall ring_buffer_push_byte, [current_rb], eax
                movzx   eax, byte [__prefix + 2]
                stdcall ring_buffer_push_byte, [current_rb], eax
                mov     dword [__prefix], 0
            .else
                inc [__prefix_index]
            .endif
        .else
            movzx eax, byte [__chr]
            stdcall ring_buffer_push_byte, [current_rb], eax
            .if byte [__chr] = 10
                mov [__is_start_line], 1
                mov [current_rb], rb_user
            .endif
        .endif
        ret
endp

; in - chr
proc print_next_char uses ebx
        .if [is_start_line] = 1
            mov eax, prefix
            add eax, [prefix_index]
            mov bl, byte [chr]
            mov [eax], bl
            .if [prefix_index] = 2
                .if dword [prefix] = 'K :'
                    stdcall set_text_color, TEXT_COLOR_YELLOW
                    mov [is_kernel_printing], 1
                .elseif dword [prefix] = 'K: '
                    stdcall set_text_color, TEXT_COLOR_YELLOW
                    mov [is_kernel_printing], 1
                .elseif dword [prefix] = 'L: '
                    stdcall set_text_color, TEXT_COLOR_WHITE
                    mov [is_kernel_printing], 0
                .elseif dword [prefix] = 'I: '
                    stdcall set_text_color, TEXT_COLOR_LIGHTCYAN
                    mov [is_kernel_printing], 0
                .elseif dword [prefix] = 'W: '
                    stdcall set_text_color, TEXT_COLOR_LIGHTMAGENTA
                    mov [is_kernel_printing], 0
                .elseif  dword [prefix] = 'E: '
                    stdcall set_text_color, TEXT_COLOR_LIGHTRED
                    mov [is_kernel_printing], 0
                .elseif dword [prefix] = 'S: '
                    stdcall set_text_color, TEXT_COLOR_LIGHTGREEN
                    mov [is_kernel_printing], 0
                .else
                    stdcall set_text_color, TEXT_COLOR_LIGHTGRAY
                    mov [is_kernel_printing], 0
                .endif
                .if [is_kernel_printing]
                    .if [current_mode] = MODE_KERNEL | [current_mode] = MODE_BOTH
                        invoke con_write_asciiz, prefix
                    .endif
                .else
                    .if [current_mode] = MODE_USER | [current_mode] = MODE_BOTH
                        invoke con_write_asciiz, prefix
                    .endif
                .endif
                ; invoke  con_write_asciiz, prefix

                mov     [is_start_line], 0
                mov     [prefix_index], 0
                mov     dword [prefix], 0
            .else
                inc [prefix_index]
            .endif
        .else
            .if [is_kernel_printing]
                .if [current_mode] = MODE_KERNEL | [current_mode] = MODE_BOTH
                    invoke con_write_asciiz, chr
                .endif
            .else
                .if [current_mode] = MODE_USER | [current_mode] = MODE_BOTH
                    invoke con_write_asciiz, chr
                .endif
            .endif
        ;     invoke con_write_asciiz, chr
            .if byte [chr] = 10
                mov [is_start_line], 1
                stdcall set_text_color, TEXT_COLOR_LIGHTGRAY
            .endif
        .endif
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
         mov eax, [current_mode]
        shl eax, 2 ; *4
        add eax, title_base
        invoke  con_init, 80, 32, -1, -1, [eax]

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
            .if ah = 0x0F ; Tab
                ; invoke con_set_title, [title_base + 4]

                ; invoke  con_exit, 1
                ; jmp .raw_exit
                mov eax, [current_mode]
                inc eax
                .if eax >= MODES_COUNT
                    xor eax, eax
                .endif
                mov [current_mode], eax
                shl eax, BSF sizeof.RING_BUFFER ; assert on sizeof, must be power of two
                add eax, rb_base
                mov [current_rb], eax
                mov eax, [current_mode]
                shl eax, 2 ; *4
                add eax, title_base
                invoke con_set_title, [eax]                
                invoke con_cls

                ; clear the printer context before printing the ring buffer
                mov dword [chr], 0
                mov dword [prefix], 0
                mov [prefix_index], 0
                mov [is_start_line], 1
                mov [is_kernel_printing], 0 ;;
                stdcall print_ring_buffer, [current_rb]
            .endif
        .endif

        mcall   SF_BOARD, SSF_DEBUG_READ
        .if ebx = 0
            mcall SF_SLEEP, 50
            jz .main_loop_cond
        .endif
        mov     [chr], al
        mov     [__chr], al

        stdcall ring_buffer_push_byte, rb_both, eax ; we always push to "both" buffer
        stdcall push_to_buffers_next_char ; push byte to user or kernel messages ring buffer depending on current state

        stdcall print_next_char

        ; append char to logfile, if no logfile then create it
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
title_base:
        dd title_mode_user
        dd title_mode_kernel
        dd title_mode_both
title_mode_user   db 'Debug board - [Tab] switches mode: USER_kernel_both  [F2] opens log file',0
title_mode_kernel db 'Debug board - [Tab] switches mode: user_KERNEL_both  [F2] opens log file',0
title_mode_both   db 'Debug board - [Tab] switches mode: user_kernel_BOTH  [F2] opens log file',0
; [Tab] changes mode: user_kernel_BOTH   [F2] opens log-file


log_file_path dd default_log_file_path
default_log_file_path db '/tmp0/1/BOARDLOG.TXT',0

; to use only in print_next_char
chr           db 0, 0, 0, 0
prefix        db 0,0,0,0
prefix_index  dd 0
is_start_line dd 1
is_kernel_printing dd 0 ; 1 if kernel is now printing (after K:), else 0

; to use only in push_to_buffers_next_char
__chr           db 0, 0, 0, 0
__prefix        db 0,0,0,0
__prefix_index  dd 0
__is_start_line dd 1

current_mode  dd MODE_BOTH
current_rb    dd 0

bytes_written dd 0

struct_open_in_notepad:
        dd SSF_START_APP
        dd 0
        .filename dd ?
        dd 0
        dd 0
        db '/sys/develop/cedit', 0

rb_base:
rb_user:
        dd rb_user_buf
        dd RB_CAPACITY
        dd 0
        dd 0
rb_kernel:
        dd rb_kernel_buf
        dd RB_CAPACITY
        dd 0
        dd 0
rb_both:
        dd rb_both_buf
        dd RB_CAPACITY
        dd 0
        dd 0


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
        con_get_flags,  'con_get_flags', \
        con_set_title,  'con_set_title',\
        con_cls,        'con_cls'

align 16
_image_end:

file_info_buf rb 40

align 4
_cmdline rb 256

thread_info     process_information
thread_name     rb 16

rb_user_buf     rb RB_CAPACITY
rb_kernel_buf   rb RB_CAPACITY
rb_both_buf     rb RB_CAPACITY

; reserve for stack:
rb      4096
align 16
_stacktop:
_memory:
