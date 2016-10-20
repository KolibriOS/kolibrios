;   This program shows  information about thread   ;
;        usage:  tinfo [slot of the thread]        ;
; if slot number is omitted then assume  slot = -1 ;
;    to compile: nasm -f bin tinfo.asm -o tinfo    ;
ORG 0
BITS 32
; ---------------------------------------------------------------------------- ;
PATH_SIZE                         equ 1024
PARAMS_SIZE                       equ 256
STACK_SIZE                        equ 256
PROC_INFO_SIZE                    equ 1024
TMP_BUFFER_SIZE                   equ 64
; ---------------------------------------------------------------------------- ;
TEXT_WIDTH                        equ 8
BOLD_TEXT_WIDTH                   equ TEXT_WIDTH + 1
; ---------------------------------------------------------------------------- ;
COLUMN1_MAX_COUNT                 equ 13
COLUMN2_MAX_COUNT                 equ 12
COLUMN3_MAX_COUNT                 equ 26
; ---------------------------------------------------------------------------- ;
COLUMN_PADDING                    equ 3
COLUMN1_PADDING                   equ COLUMN_PADDING
COLUMN2_PADDING                   equ COLUMN_PADDING
COLUMN3_PADDING                   equ COLUMN_PADDING
; ---------------------------------------------------------------------------- ;
ITEM_HEIGHT                       equ 22
TEXT_HEIGHT                       equ 16
%IF 0
COLUMN1_ITEM_WIDTH                equ COLUMN1_MAX_COUNT * BOLD_TEXT_WIDTH + COLUMN1_PADDING * 2
COLUMN2_ITEM_WIDTH                equ COLUMN2_MAX_COUNT * BOLD_TEXT_WIDTH + COLUMN2_PADDING * 2
COLUMN3_ITEM_WIDTH                equ COLUMN3_MAX_COUNT * BOLD_TEXT_WIDTH + COLUMN3_PADDING * 2
%ENDIF
COLUMN1_ITEM_WIDTH                equ COLUMN1_MAX_COUNT * TEXT_WIDTH + COLUMN1_PADDING * 2
COLUMN2_ITEM_WIDTH                equ COLUMN2_MAX_COUNT * TEXT_WIDTH + COLUMN2_PADDING * 2
COLUMN3_ITEM_WIDTH                equ COLUMN3_MAX_COUNT * TEXT_WIDTH + COLUMN3_PADDING * 2
ITEM_MARGIN                       equ 6
ITEM_BACK_COLOR_1                 equ 0x00EAEAEA
ITEM_BACK_COLOR_2                 equ 0x00F4F4F4
ITEM_COUNT                        equ 18 ; at current time we have 18 items
; ---------------------------------------------------------------------------- ;
COLUMN_Y                          equ 10
COLUMN1_X                         equ 10
COLUMN2_X                         equ COLUMN1_X + COLUMN1_ITEM_WIDTH + ITEM_MARGIN
COLUMN3_X                         equ COLUMN2_X + COLUMN2_ITEM_WIDTH + ITEM_MARGIN
; ---------------------------------------------------------------------------- ;
COLUMN1_TEXT_X                    equ COLUMN1_X + COLUMN1_PADDING
COLUMN2_TEXT_X                    equ COLUMN2_X + COLUMN2_PADDING
COLUMN3_TEXT_X                    equ COLUMN3_X + COLUMN3_PADDING
; ---------------------------------------------------------------------------- ;
FRAME_TOP                         equ COLUMN_Y - ITEM_MARGIN / 2
FRAME_BOTTOM                      equ COLUMN_Y + ITEM_HEIGHT * ITEM_COUNT + ITEM_MARGIN / 2 - 1
 ; ---------------------------------------------------------------------------- ;
FRAME1_LEFT                       equ COLUMN1_X - ITEM_MARGIN / 2
FRAME1_RIGHT                      equ COLUMN1_X + COLUMN1_ITEM_WIDTH + ITEM_MARGIN / 2 - 1
; ---------------------------------------------------------------------------- ;
FRAME2_LEFT                       equ COLUMN2_X - ITEM_MARGIN / 2
FRAME2_RIGHT                      equ COLUMN2_X + COLUMN2_ITEM_WIDTH + ITEM_MARGIN / 2 - 1
; ---------------------------------------------------------------------------- ;
FRAME3_LEFT                       equ COLUMN3_X - ITEM_MARGIN / 2
FRAME3_RIGHT                      equ COLUMN3_X + COLUMN3_ITEM_WIDTH + ITEM_MARGIN / 2 - 1
; ---------------------------------------------------------------------------- ;
WINDOW_STYLE_SKINNED_FIXED        equ 0x4000000
WINDOW_STYLE_COORD_CLIENT         equ 0x20000000
WINDOW_STYLE_CAPTION              equ 0x10000000
; ---------------------------------------------------------------------------- ;
WINDOW_BORDER_SIZE                equ 5
WINDOW_WIDTH                      equ FRAME3_RIGHT + FRAME1_LEFT + WINDOW_BORDER_SIZE * 2
WINDOW_STYLE                      equ WINDOW_STYLE_SKINNED_FIXED | WINDOW_STYLE_COORD_CLIENT | WINDOW_STYLE_CAPTION
WINDOW_BACK_COLOR                 equ 0x00FFFFFF
; ---------------------------------------------------------------------------- ;
INDICATOR_WIDTH                   equ 3
INDICATOR_HEIGHT                  equ 3
INDICATOR_LEFT                    equ COLUMN3_X + COLUMN3_ITEM_WIDTH + ITEM_MARGIN / 2
INDICATOR_TOP                     equ COLUMN_Y  - ITEM_MARGIN / 2 - INDICATOR_HEIGHT
; ---------------------------------------------------------------------------- ;
UPDATE_TIME                       equ 28
; ---------------------------------------------------------------------------- ;
thread_info                       equ END + PATH_SIZE + PARAMS_SIZE
tmpbuffer                         equ END + PATH_SIZE + PARAMS_SIZE + PROC_INFO_SIZE
; ---------------------------------------------------------------------------- ;
KEYBOARD_MODE_ASCII               equ 0
KEYBOARD_MODE_SCAN                equ 1
; ---------------------------------------------------------------------------- ;
WINDOW_STATE_MAXIMIZED            equ 1
WINDOW_STATE_MINIMIZED            equ 2
WINDOW_STATE_ROLLED_UP            equ 4
; ---------------------------------------------------------------------------- ;
THREAD_STATE_RUNNING              equ 0
THREAD_STATE_SUSPENDED            equ 1
THREAD_STATE_SUSPENDED_WAIT_EVENT equ 2
THREAD_STATE_NORMAL_TERMINATING   equ 3
THREAD_STATE_EXCEPT_TERMINATING   equ 4
THREAD_STATE_WAIT_EVENT           equ 5
THREAD_STATE_SLOT_IS_FREE         equ 9
; ---------------------------------------------------------------------------- ;
EM_REDRAW                         equ         1b
EM_KEY                            equ        10b
EM_BUTTON                         equ       100b
EM_RESERVED0                      equ      1000b
EM_REDRAW_BACKGROUND              equ     10000b
EM_MOUSE                          equ    100000b
EM_IPC                            equ   1000000b
EM_NETWORK                        equ  10000000b
EM_DEBUG                          equ 100000000b
; ---------------------------------------------------------------------------- ;
struc THREAD_INFO
        .cpu_usage               resd 1  ; usage of the processor
        .win_stack_pos           resw 1  ; position of the window of thread in the window stack
        .reserved0               resw 1  ; has no relation to the specified thread
        .reserved1               resw 1  ; reserved
        .name                    resb 11 ; name of the started file - executable file without extension
        .reserved2               resb 1  ; reserved, this byte is not changed
        .mem_address             resd 1  ; address of the process in memory
        .mem_usage               resd 1  ; size of used memory - 1
        .identifier              resd 1  ; identifier (PID/TID)
        .x                       resd 1  ; coordinate of the thread window on axis x
        .y                       resd 1  ; coordinate of the thread window on axis y
        .size_x                  resd 1  ; size of the thread window on axis x
        .size_y                  resd 1  ; size of the thread window on axis y
        .thread_state            resw 1  ; status of the thread slot
        .reserved3               resw 1  ; reserved, this word is not changed
        .client_x                resd 1  ; coordinate of the client area on axis x
        .client_y                resd 1  ; coordinate of the client area on axis y
        .client_size_x           resd 1  ; width of the client area
        .client_size_y           resd 1  ; height of the client area
        .window_state            resb 1  ; state of the window - bitfield
        .event_mask              resd 1  ; event mask
        .keyboard_mode           resb 1  ; keyboard mode
endstruc
; ---------------------------------------------------------------------------- ;
MENUET01       db 'MENUET01'
version        dd 1
program.start  dd START
program.end    dd END
program.memory dd END + PATH_SIZE + PARAMS_SIZE + PROC_INFO_SIZE + TMP_BUFFER_SIZE + STACK_SIZE
program.stack  dd END + PATH_SIZE + PARAMS_SIZE + PROC_INFO_SIZE + TMP_BUFFER_SIZE + STACK_SIZE
program.params dd END + PATH_SIZE
program.path   dd END
; ---------------------------------------------------------------------------- ;
slot                              dd -1 ; for default if no params
; ---------------------------------------------------------------------------- ;
screen:
.height                           dw 0
.width                            dw 0
; ---------------------------------------------------------------------------- ;
window:
.left                             dd 0
.top                              dd 0
.width                            dd 0
.height                           dd 0
; ---------------------------------------------------------------------------- ;
fore_color                        dd 0x00000000
back_color                        dd ITEM_BACK_COLOR_1
frame_color                       dd 0x00CCCCCC
; ---------------------------------------------------------------------------- ;
BackColors:
                                  dd 1
                                  dd ITEM_BACK_COLOR_1
                                  dd ITEM_BACK_COLOR_2
; ---------------------------------------------------------------------------- ;
IndicatorColors:
                                  dd 1
                                  dd 0x000AF000
                                  dd 0x00000FA0
; ---------------------------------------------------------------------------- ;
%define x [Pos.x]
%define y [Pos.y]
Pos:
.x                                dd COLUMN1_X
.y                                dd COLUMN_Y
; ---------------------------------------------------------------------------- ;
sz_caption                        db "ThreadInfo",0
; ---------------------------------------------------------------------------- ;
sz_cpu_usage                      db "CPU usage",0
sz_win_stack_pos                  db "Win stack pos",0
sz_name                           db "Name",0
sz_mem_address                    db "Mem address",0
sz_mem_usage                      db "Mem usage",0
sz_identifier                     db "Identifier",0
sz_x                              db "X",0
sz_y                              db "Y",0
sz_size_x                         db "Size X",0
sz_size_y                         db "Size Y",0
sz_thread_state                   db "Thread state",0
sz_client_x                       db "Client X",0
sz_client_y                       db "Client Y",0
sz_client_size_x                  db "Client Size X",0
sz_client_size_y                  db "Client Size Y",0
sz_window_state                   db "Window state",0
sz_event_mask                     db "Event mask",0
sz_keyboard_mode                  db "Keyboard mode",0
; ---------------------------------------------------------------------------- ;
; state of the window
sz_maximized                      db "Max ",0
sz_minimized                      db "Min ",0
sz_rolled_up                      db "RollUp ",0
; ---------------------------------------------------------------------------- ;
; keyboard mode
sz_ascii                          db "ASCII",0
sz_scan                           db "SCAN ",0
; ---------------------------------------------------------------------------- ;
; status of the thread slot
sz_running                        db "running             ",0
sz_suspended                      db "suspended           ",0
sz_suspended_wait_event           db "suspended wait event",0
sz_normal_terminating             db "normal terminating  ",0
sz_except_terminating             db "except. terminating ",0
sz_wait_event                     db "wait event          ",0
sz_slot_is_free                   db "slot is free        ",0
; ---------------------------------------------------------------------------- ;
; event mask
sz_redraw                         db "rdrw ",0
sz_key                            db "key ",0
sz_button                         db "btn ",0
sz_reserved0                      db "rsrvd0 ",0
sz_redraw_background              db "bckgr ",0
sz_mouse                          db "mouse ",0
sz_ipc                            db "ipc ",0
sz_network                        db "net ",0
sz_debug                          db "dbg ",0
; ---------------------------------------------------------------------------- ;
sz_undefined                      db "UnDef               ",0
; **************************************************************************** ;
%macro DrawCpuUsage 0
; sz_cpu_usage
        push   dword COLUMN1_TEXT_X
        push   dword y
        push   dword sz_cpu_usage
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
; [cpu_usage]
        push   dword [thread_info + THREAD_INFO.cpu_usage]
        call   uint2str
        push   dword COLUMN2_MAX_COUNT
        call   PadBuffSpaces
        push   dword COLUMN2_TEXT_X
        push   dword y
        push   dword tmpbuffer
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   ChangeBackColor
        add    y, dword ITEM_HEIGHT
%endmacro
; **************************************************************************** ;
%macro DrawWinStackPos 0
; sz_win_stack_pos
        push   dword COLUMN1_TEXT_X
        push   dword y
        push   dword sz_win_stack_pos
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
; [win_stack_pos]
        movzx  eax, word [thread_info + THREAD_INFO.win_stack_pos]
        push   eax
        call   uint2str
        push   dword COLUMN2_MAX_COUNT
        call   PadBuffSpaces
        push   dword COLUMN2_TEXT_X
        push   dword y
        push   dword tmpbuffer
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   ChangeBackColor
        add    y, dword ITEM_HEIGHT
%endmacro
; **************************************************************************** ;
%macro DrawName 0
; sz_name
        push   dword COLUMN1_TEXT_X
        push   dword y
        push   dword sz_name
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
; name
        push   dword COLUMN2_TEXT_X
        push   dword y
        push   dword (thread_info + THREAD_INFO.name)
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   ChangeBackColor
        add    y, dword ITEM_HEIGHT
%endmacro
; **************************************************************************** ;
%macro DrawMemAddress 0
; sz_mem_address
        push   dword COLUMN1_TEXT_X
        push   dword y
        push   dword sz_mem_address
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
; [mem_address]
        push   dword [thread_info + THREAD_INFO.mem_address]
        call   uint2str
        push   dword COLUMN2_MAX_COUNT
        call   PadBuffSpaces
        push   dword COLUMN2_TEXT_X
        push   dword y
        push   dword tmpbuffer
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   ChangeBackColor
        add    y, dword ITEM_HEIGHT
%endmacro
; **************************************************************************** ;
%macro DrawMemUsage 0
; sz_mem_usage
        push   dword COLUMN1_TEXT_X
        push   dword y
        push   dword sz_mem_usage
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
; [mem_usage]
        push   dword [thread_info + THREAD_INFO.mem_usage]
        call   uint2str
        push   dword COLUMN2_MAX_COUNT
        call   PadBuffSpaces
        push   dword COLUMN2_TEXT_X
        push   dword y
        push   dword tmpbuffer
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   ChangeBackColor
        add    y, dword ITEM_HEIGHT
%endmacro
; **************************************************************************** ;
%macro DrawIdentifier 0
; sz_identifier
        push   dword COLUMN1_TEXT_X
        push   dword y
        push   dword sz_identifier
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
; [identifier]
        push   dword [thread_info + THREAD_INFO.identifier]
        call   uint2str
        push   dword COLUMN2_MAX_COUNT
        call   PadBuffSpaces
        push   dword COLUMN2_TEXT_X
        push   dword y
        push   dword tmpbuffer
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   ChangeBackColor
        add    y, dword ITEM_HEIGHT
%endmacro
; **************************************************************************** ;
%macro DrawWindowX 0
; sz_x
        push   dword COLUMN1_TEXT_X
        push   dword y
        push   dword sz_x
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
; [x]
        push   dword [thread_info + THREAD_INFO.x]
        call   uint2str
        push   dword COLUMN2_MAX_COUNT
        call   PadBuffSpaces
        push   dword COLUMN2_TEXT_X
        push   dword y
        push   dword tmpbuffer
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   ChangeBackColor
        add    y, dword ITEM_HEIGHT
%endmacro
; **************************************************************************** ;
%macro DrawWindowY 0
; sz_y
        push   dword COLUMN1_TEXT_X
        push   dword y
        push   dword sz_y
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
; [y]
        push   dword [thread_info + THREAD_INFO.y]
        call   uint2str
        push   dword COLUMN2_MAX_COUNT
        call   PadBuffSpaces
        push   dword COLUMN2_TEXT_X
        push   dword y
        push   dword tmpbuffer
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   ChangeBackColor
        add    y, dword ITEM_HEIGHT
%endmacro
; **************************************************************************** ;
%macro DrawWindowSizeX 0
; sz_size_x
        push   dword COLUMN1_TEXT_X
        push   dword y
        push   dword sz_size_x
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
; [size_x]
        push   dword [thread_info + THREAD_INFO.size_x]
        call   uint2str
        push   dword COLUMN2_MAX_COUNT
        call   PadBuffSpaces
        push   dword COLUMN2_TEXT_X
        push   dword y
        push   dword tmpbuffer
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   ChangeBackColor
        add    y, dword ITEM_HEIGHT
%endmacro
; **************************************************************************** ;
%macro DrawWindowSizeY 0
; sz_size_y
        push   dword COLUMN1_TEXT_X
        push   dword y
        push   dword sz_size_y
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
; [size_y]
        push   dword [thread_info + THREAD_INFO.size_y]
        call   uint2str
        push   dword COLUMN2_MAX_COUNT
        call   PadBuffSpaces
        push   dword COLUMN2_TEXT_X
        push   dword y
        push   dword tmpbuffer
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   ChangeBackColor
        add    y, dword ITEM_HEIGHT
%endmacro
; **************************************************************************** ;
%macro DrawThreadState 0
; sz_thread_state
        push   dword COLUMN1_TEXT_X
        push   dword y
        push   dword sz_thread_state
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
; decoded_thread_state & [thread_state]
        movzx  eax, word [thread_info + THREAD_INFO.thread_state]
        push   eax ; for "call uint2str" below
%%running:
        cmp    eax, THREAD_STATE_RUNNING
        jne    %%suspended
        mov    eax, sz_running
        jmp    %%draw_decoded_thread_state
%%suspended:
        cmp    eax, THREAD_STATE_SUSPENDED
        jne    %%suspended_w
        mov    eax, sz_suspended
        jmp    %%draw_decoded_thread_state
%%suspended_w:
        cmp    eax, THREAD_STATE_SUSPENDED_WAIT_EVENT
        jne    %%normal_term
        mov    eax, sz_suspended_wait_event
        jmp    %%draw_decoded_thread_state
%%normal_term:
        cmp    eax, THREAD_STATE_NORMAL_TERMINATING
        jne    %%except_term
        mov    eax, sz_normal_terminating
        jmp    %%draw_decoded_thread_state
%%except_term:
        cmp    eax, THREAD_STATE_EXCEPT_TERMINATING
        jne    %%wait_event
        mov    eax, sz_except_terminating
        jmp    %%draw_decoded_thread_state
%%wait_event:
        cmp    eax, THREAD_STATE_WAIT_EVENT
        jne    %%slot_free
        mov    eax, sz_wait_event
        jmp    %%draw_decoded_thread_state
%%slot_free:
        cmp    eax, THREAD_STATE_SLOT_IS_FREE
        jne    %%undefined
        mov    eax, sz_slot_is_free
        jmp    %%draw_decoded_thread_state
%%undefined:
        mov    eax, sz_undefined
%%draw_decoded_thread_state:
        push   dword COLUMN3_TEXT_X
        push   dword y
        push   eax
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   uint2str
        push   dword COLUMN2_MAX_COUNT
        call   PadBuffSpaces
        push   dword COLUMN2_TEXT_X
        push   dword y
        push   dword tmpbuffer
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   ChangeBackColor
        add    y, dword ITEM_HEIGHT
%endmacro
; **************************************************************************** ;
%macro DrawClientX 0
; sz_client_x
        push   dword COLUMN1_TEXT_X
        push   dword y
        push   dword sz_client_x
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
; [client_x]
        push   dword [thread_info + THREAD_INFO.client_x]
        call   uint2str
        push   dword COLUMN2_MAX_COUNT
        call   PadBuffSpaces
        push   dword COLUMN2_TEXT_X
        push   dword y
        push   dword tmpbuffer
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   ChangeBackColor
        add    y, dword ITEM_HEIGHT
%endmacro
; **************************************************************************** ;
%macro DrawClientY 0
; sz_client_y
        push   dword COLUMN1_TEXT_X
        push   dword y
        push   dword sz_client_y
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
; [client_y]
        push   dword [thread_info + THREAD_INFO.client_y]
        call   uint2str
        push   dword COLUMN2_MAX_COUNT
        call   PadBuffSpaces
        push   dword COLUMN2_TEXT_X
        push   dword y
        push   dword tmpbuffer
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   ChangeBackColor
        add    y, dword ITEM_HEIGHT
%endmacro
; **************************************************************************** ;
%macro DrawClientSizeX 0
; sz_client_size_x
        push   dword COLUMN1_TEXT_X
        push   dword y
        push   dword sz_client_size_x
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
; [client_size_x]
        push   dword [thread_info + THREAD_INFO.client_size_x]
        call   uint2str
        push   dword COLUMN2_MAX_COUNT
        call   PadBuffSpaces
        push   dword COLUMN2_TEXT_X
        push   dword y
        push   dword tmpbuffer
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   ChangeBackColor
        add    y, dword ITEM_HEIGHT
%endmacro
; **************************************************************************** ;
%macro DrawClientSizeY 0
; sz_client_size_y
        push   dword COLUMN1_TEXT_X
        push   dword y
        push   dword sz_client_size_y
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
; [client_size_y]
        push   dword [thread_info + THREAD_INFO.client_size_y]
        call   uint2str
        push   dword COLUMN2_MAX_COUNT
        call   PadBuffSpaces
        push   dword COLUMN2_TEXT_X
        push   dword y
        push   dword tmpbuffer
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   ChangeBackColor
        add    y, dword ITEM_HEIGHT
%endmacro
; **************************************************************************** ;
%macro DrawWindowState 0
; sz_window_state
        push   dword COLUMN1_TEXT_X
        push   dword y
        push   dword sz_window_state
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
; decoded_window_state & [window_state]
        movzx  eax, byte [thread_info + THREAD_INFO.window_state]
        push   eax ; for "call uint2str" below
        mov    ebx, eax
        mov    [tmpbuffer], byte 0
%%maximized:
        test   ebx, WINDOW_STATE_MAXIMIZED
        jz     %%minimized
        push   tmpbuffer
        push   sz_maximized
        call   StringConcatenate
%%minimized:
        test   ebx, WINDOW_STATE_MINIMIZED
        jz     %%rolled_up
        push   tmpbuffer
        push   sz_minimized
        call   StringConcatenate
%%rolled_up:
        test   ebx, WINDOW_STATE_ROLLED_UP
        jz     %%draw_decoded_window_state
        push   tmpbuffer
        push   sz_rolled_up
        call   StringConcatenate
%%draw_decoded_window_state:
        push   dword COLUMN3_MAX_COUNT
        call   PadBuffSpaces
        push   dword COLUMN3_TEXT_X
        push   dword y
        push   dword tmpbuffer
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   uint2str
        push   dword COLUMN2_MAX_COUNT
        call   PadBuffSpaces
        push   dword COLUMN2_TEXT_X
        push   dword y
        push   dword tmpbuffer
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   ChangeBackColor
        add    y, dword ITEM_HEIGHT
%endmacro
; **************************************************************************** ;
%macro DrawEventMask 0
; sz_event_mask
        push   dword COLUMN1_TEXT_X
        push   dword y
        push   dword sz_event_mask
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
; decoded_event_mask & [event_mask]
        mov    eax, [thread_info + THREAD_INFO.event_mask]
        push   eax ; for "call uint2str" below
        mov    ebx, eax
        mov    [tmpbuffer], byte 0
%%redraw:
        test   ebx, EM_REDRAW
        jz     %%key
        push   tmpbuffer
        push   sz_redraw
        call   StringConcatenate
%%key:
        test   ebx, EM_KEY
        jz     %%button
        push   tmpbuffer
        push   sz_key
        call   StringConcatenate
%%button:
        test   ebx, EM_BUTTON
        jz     %%reserved0
        push   tmpbuffer
        push   sz_button
        call   StringConcatenate
%%reserved0:
        test   ebx, EM_RESERVED0
        jz     %%redraw_background
        push   tmpbuffer
        push   sz_reserved0
        call   StringConcatenate
%%redraw_background:
        test   ebx, EM_REDRAW_BACKGROUND
        jz     %%mouse
        push   tmpbuffer
        push   sz_redraw_background
        call   StringConcatenate
%%mouse:
        test   ebx, EM_MOUSE
        jz     %%ipc
        push   tmpbuffer
        push   sz_mouse
        call   StringConcatenate
%%ipc:
        test   ebx, EM_IPC
        jz     %%network
        push   tmpbuffer
        push   sz_ipc
        call   StringConcatenate
%%network:
        test   ebx, EM_NETWORK
        jz     %%debug
        push   tmpbuffer
        push   sz_network
        call   StringConcatenate
%%debug:
        test   ebx, EM_DEBUG
        jz     %%draw_decoded_event_mask
        push   tmpbuffer
        push   sz_debug
        call   StringConcatenate
%%draw_decoded_event_mask:
        push   dword COLUMN3_MAX_COUNT
        call   PadBuffSpaces
        push   dword COLUMN3_TEXT_X
        push   dword y
        push   dword tmpbuffer
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   uint2str
        push   dword COLUMN2_MAX_COUNT
        call   PadBuffSpaces
        push   dword COLUMN2_TEXT_X
        push   dword y
        push   dword tmpbuffer
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   ChangeBackColor
        add    y, dword ITEM_HEIGHT
%endmacro
; **************************************************************************** ;
%macro DrawKeyboardMode 0
        push   dword COLUMN2_MAX_COUNT
        call   PadBuffSpaces
; sz_keyboard_mode
        push   dword COLUMN1_TEXT_X
        push   dword y
        push   dword sz_keyboard_mode
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
; decoded_keyboard_mode & [keyboard_mode]
        movzx  eax, byte [thread_info + THREAD_INFO.keyboard_mode]
        push   eax ; for "call uint2str" below
%%ascii:
        cmp    eax, KEYBOARD_MODE_ASCII
        jne    %%scan
        mov    eax, sz_ascii
        jmp    %%draw_decoded_keyboard_mode
%%scan:
        cmp    eax, KEYBOARD_MODE_SCAN
        jne    %%undefined
        mov    eax, sz_scan
        jmp    %%draw_decoded_keyboard_mode
%%undefined:
        mov    eax, sz_undefined
%%draw_decoded_keyboard_mode:
        push   dword COLUMN3_TEXT_X
        push   dword y
        push   eax
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   uint2str
        push   dword COLUMN2_TEXT_X
        push   dword y
        push   dword tmpbuffer
        push   dword [fore_color]
        push   dword [back_color]
        call   DrawText
        call   ChangeBackColor
        add    y, dword ITEM_HEIGHT
%endmacro
; **************************************************************************** ;
%macro DrawUpdateIndicator 0
        mov    eax, [IndicatorColors]
        sub    eax, 3
        neg    eax
        mov    [IndicatorColors], eax
; draw.rectangle
        mov    edx, [eax * 4 + IndicatorColors]
        mov    eax, 13
        mov    ebx, INDICATOR_LEFT << 16 | INDICATOR_WIDTH
        mov    ecx, INDICATOR_TOP  << 16 | INDICATOR_HEIGHT
        int    64
%endmacro
; **************************************************************************** ;
%macro DrawTable 0
; DrawFrames
        push   dword FRAME1_LEFT
        push   dword FRAME_TOP
        push   dword FRAME1_RIGHT
        push   dword FRAME_BOTTOM
        push   dword [frame_color]
        call   DrawFrame
        push   dword FRAME2_LEFT
        push   dword FRAME_TOP
        push   dword FRAME2_RIGHT
        push   dword FRAME_BOTTOM
        push   dword [frame_color]
        call   DrawFrame
        push   dword FRAME3_LEFT
        push   dword FRAME_TOP
        push   dword FRAME3_RIGHT
        push   dword FRAME_BOTTOM
        push   dword [frame_color]
        call   DrawFrame
; DrawItems
        mov    esi, COLUMN_Y
        xor    edi, edi
%%draw_item:
        cmp    edi, ITEM_COUNT
        jnl    %%done
        mov    ecx, esi
        shl    ecx, 16
        or     ecx, ITEM_HEIGHT
; draw.rectangle
        mov    eax, 13
        mov    ebx, COLUMN1_X << 16 | COLUMN1_ITEM_WIDTH
        mov    edx, [back_color]
        int    64
; draw.rectangle
        mov    eax, 13
        mov    ebx, COLUMN2_X << 16 | COLUMN2_ITEM_WIDTH
        mov    edx, [back_color]
        int    64
; draw.rectangle
        mov    eax, 13
        mov    ebx, COLUMN3_X << 16 | COLUMN3_ITEM_WIDTH
        mov    edx, [back_color]
        int    64
        call   ChangeBackColor
        add    esi, ITEM_HEIGHT
        inc    edi
        jmp    %%draw_item
%%done:
%endmacro
; ---------------------------------------------------------------------------- ;
align 4
START:
        mov    esi, [program.params]
        test   [esi], byte 0xFF
        jz     .no_params
; str2uint(program.params)
        xor    eax, eax
        xor    ecx, ecx
.convert:
        lodsb
        test   al, al
        jz     .converted
        lea    ecx, [ecx + ecx * 4]
        lea    ecx, [eax + ecx * 2 - 48]
        jmp    .convert
.converted:
        mov    [slot], ecx
.no_params:
; get.screen.size
        mov    eax, 61
        mov    ebx, 1
        int    64
        mov    [screen], eax
; skin.height
        mov    eax, 48
        mov    ebx, 4
        int    64
        add    eax, FRAME_BOTTOM + FRAME_TOP + WINDOW_BORDER_SIZE
        mov    [window.width], dword WINDOW_WIDTH
        mov    [window.height], eax
        movzx  eax, word [screen.width]
        movzx  edx, word [screen.height]
        sub    eax, [window.width]
        sub    edx, [window.height]
        shr    eax, 1
        shr    edx, 1
        mov    [window.left], eax
        mov    [window.top], edx
; set.event
        mov    eax, 40
        mov    ebx, EM_REDRAW | EM_BUTTON
        int    64
; ---------------------------------------------------------------------------- ;
align 4
on_redraw:
; redraw.start
        mov    eax, 12
        mov    ebx, 1
        int    64
; draw.window
        xor    eax, eax
        mov    ebx, [window.left]
        mov    ecx, [window.top]
        shl    ebx, 16
        shl    ecx, 16
        or     ebx, [window.width]
        or     ecx, [window.height]
        mov    edx, WINDOW_STYLE | WINDOW_BACK_COLOR
        mov    edi, sz_caption
        xor    esi, esi
        int    64
; redraw.finish
        mov    eax, 12
        mov    ebx, 2
        int    64
        DrawTable
        call   UpdateThreadInfo
align 4
wait.event.by.time:
        mov    eax, 23
        mov    ebx, UPDATE_TIME
        int    64
        dec    eax
        jz     on_redraw        ; IF      eax = 1 THEN   redraw
        jns    on_button        ; ELSEIF  eax = 2 THEN   button
        call   UpdateThreadInfo ; ELSE no event -> update thread info
        jmp    wait.event.by.time
align 4
on_button: ; terminate because we have only one button(close button)
        or     eax, -1
        int    64
; ---------------------------------------------------------------------------- ;
align 4
UpdateThreadInfo:
; get.thread.info
        mov    eax, 9
        mov    ebx, thread_info
        mov    ecx, [slot]
        int    64

        mov    x, dword COLUMN1_X
        mov    y, dword COLUMN_Y + (ITEM_HEIGHT - TEXT_HEIGHT) / 2
        mov    [back_color], dword ITEM_BACK_COLOR_1
        mov    [BackColors], dword 1
; order of next "Draw..." can be changed
        DrawName
        DrawThreadState
        DrawWindowState
        DrawEventMask
        DrawKeyboardMode
        DrawCpuUsage
        DrawMemUsage
        DrawMemAddress
        DrawIdentifier
        DrawWinStackPos
        DrawWindowX
        DrawWindowY
        DrawWindowSizeX
        DrawWindowSizeY
        DrawClientX
        DrawClientY
        DrawClientSizeX
        DrawClientSizeY

        DrawUpdateIndicator ; blinking thing at upper right corner

        ret
; ---------------------------------------------------------------------------- ;
align 4
DrawFrame:
%define Color  [esp +  4 +1*4]
%define Bottom [esp +  8 +1*4]
%define Top    [esp + 16 +1*4]
%define Right  [esp + 12 +1*4]
%define Left   [esp + 20 +1*4]
        push   ebp
        mov    eax, 38
        mov    edx, Color
        mov    esi, Bottom
        mov    edi, Top
        mov    ebp, Right
        mov    ebx, Left
        shl    ebx, 16
        mov    bx, bp
        shrd   ecx, edi, 16
        mov    cx, di
        int    64
        shrd   ecx, esi, 16
        mov    cx, si
        int    64
        shld   esi, ebx, 16
        mov    bx, si
        shrd   ecx, edi, 16
        int    64
        shrd   ebx, ebp, 16
        mov    bx, bp
        int    64
        pop    ebp
        ret    20
%undef Color
%undef Bottom
%undef Top
%undef Right
%undef Left
; ---------------------------------------------------------------------------- ;
align 4
StringConcatenate:
%define stradd [esp +  4]
%define str    [esp +  8]
        mov    esi, stradd
        or     ecx, -1
        mov    edi, esi
        xor    eax, eax
        repne scasb
        mov    edx, ecx
        mov    edi, str
        repne scasb
        dec    edi
        not    edx
        mov    eax, str
        mov    ecx, edx
        shr    ecx, 2
        and    edx, 3
        rep movsd
        mov    ecx, edx
        rep movsb
        ret    8
%undef stradd
%undef str
; ---------------------------------------------------------------------------- ;
align 4
DrawText:
%define x          [esp + 20]
%define y          [esp + 16]
%define text       [esp + 12]
%define fore_color [esp +  8]
%define back_color [esp +  4]
        mov    eax, 4
        mov    ecx, fore_color
        mov    edi, back_color
        mov    edx, text
        mov    ebx, x
        shl    ebx, 16
        or     ebx, y
        or     ecx, 0xD0000000
        int    64
        ret    20
%undef x
%undef y
%undef text
%undef fore_color
%undef back_color
; ---------------------------------------------------------------------------- ;
%IF 0
align 4
DrawTextBold:
%define x          [esp + 20]
%define y          [esp + 16]
%define text       [esp + 12]
%define fore_color [esp +  8]
%define back_color [esp +  4]
        mov    eax, 4
        mov    ecx, fore_color
        mov    edi, back_color
        mov    esi, 1 ; count
        mov    edx, text
        mov    ebx, x
        shl    ebx, 16
        or     ebx, y
align 4
.next:
        test   [edx], byte 0xFF
        jz     .done
        or     ecx, 0x50000000
        int    64
        add    ebx, (1 << 16)
        and    ecx, 0x10FFFFFF
        int    64
        add    ebx, (TEXT_WIDTH << 16)
        inc    edx
        jmp    .next
align 4
.done:
        ret    20
%undef x
%undef y
%undef text
%undef fore_color
%undef back_color
%ENDIF
; ---------------------------------------------------------------------------- ;
align 4
ChangeBackColor:
        mov    eax, [BackColors]
        sub    eax, 3
        neg    eax
        mov    [BackColors], eax
        mov    eax, [eax * 4 + BackColors]
        mov    [back_color], eax
        ret
; ---------------------------------------------------------------------------- ;
align 4
uint2str:
%define value  [esp + 4]
        push   dword value     ; value
        push   dword 10        ; base(decimal)
        push   dword tmpbuffer ; buffer
        call   ConvertToBase
        ret    4
%undef value
; ---------------------------------------------------------------------------- ;
align 4
ConvertToBase:
%define value  [esp + 12]        ; value treated as unsigned
%define base   [esp +  8]        ; 2 <= base <= 36
%define buffer [esp +  4]        ; SizeOf(buffer) => (32 + 1)
        mov    eax, value
        mov    ecx, base
        mov    esi, buffer
        mov    edi, esi          ;                    +0                             +31 +32
        add    esi, 32           ; base2(0xFFFFFFFF) = 11111111111111111111111111111111b
        mov    [esi], byte 0     ; end of string                                       byte 0
align 4
.next:
        xor    edx, edx
        div    ecx
        dec    esi
        mov    dl, [edx + .digits]; (put digit
        mov    [esi], dl          ;           to buffer)
        test   eax, eax
        jnz    .next
; shift result string to buffer beginning
        mov    eax, esi
        sub    eax, edi
        mov    ecx, 32 + 1
        sub    ecx, eax
        mov    eax, edi          ; return buffer
; yes, memory overlapped, but edi not above than esi
; hope that movsD faster than movsB in this case
; if you want only "rep movsb" because it shorter then remove five next lines before "rep movsb"
        mov    edx, ecx
        shr    ecx, 2
        and    edx, 3
        rep    movsd
        mov    ecx, edx
        rep    movsb
        ret    12
%undef value
%undef base
%undef buffer
align 4
.digits  db    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
; ---------------------------------------------------------------------------- ;
align 4
PadBuffSpaces:
%define maxlen [esp + 4]
        mov    edi, tmpbuffer
        or     ecx, -1
        xor    eax, eax
        repne scasb
        dec    edi
        sub    eax, ecx
        dec    eax
        mov    ecx, maxlen
        sub    ecx, eax
        mov    eax, "    "
        mov    edx, ecx
        shr    ecx, 2
        and    edx, 3
        rep    stosd
        mov    ecx, edx
        rep    stosb
        mov    [edi], byte 0
        ret    4
%undef maxlen
; ---------------------------------------------------------------------------- ;
align 4
END: