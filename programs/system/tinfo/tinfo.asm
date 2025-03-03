;   This program shows  information about thread   ;
;        usage:  tinfo [slot of the thread]        ;
; if slot number is omitted then assume  slot = -1 ;
;    to compile: nasm -f bin tinfo.asm -o tinfo    ;

use32
	org 0
	db 'MENUET01'
version dd 1
	dd program.start
	dd program.end
	dd program.memory
	dd program.stack
	dd program.params
	dd program.path

include '../../macros.inc'
include '../../proc32.inc'
include '../../KOSfuncs.inc'
include 'lang.inc'

; ---------------------------------------------------------------------------- ;
TEXT_WIDTH                        = 8
BOLD_TEXT_WIDTH                   = TEXT_WIDTH + 1
; ---------------------------------------------------------------------------- ;
if lang eq ru_RU
COLUMN1_MAX_COUNT                 = 19
COLUMN2_MAX_COUNT                 = 12
COLUMN3_MAX_COUNT                 = 26
else
COLUMN1_MAX_COUNT                 = 13
COLUMN2_MAX_COUNT                 = 12
COLUMN3_MAX_COUNT                 = 26
end if
; ---------------------------------------------------------------------------- ;
COLUMN_PADDING                    = 3
COLUMN1_PADDING                   = COLUMN_PADDING
COLUMN2_PADDING                   = COLUMN_PADDING
COLUMN3_PADDING                   = COLUMN_PADDING
; ---------------------------------------------------------------------------- ;
ITEM_HEIGHT                       = 22
TEXT_HEIGHT                       = 16
;COLUMN1_ITEM_WIDTH                = COLUMN1_MAX_COUNT * BOLD_TEXT_WIDTH + COLUMN1_PADDING * 2
;COLUMN2_ITEM_WIDTH                = COLUMN2_MAX_COUNT * BOLD_TEXT_WIDTH + COLUMN2_PADDING * 2
;COLUMN3_ITEM_WIDTH                = COLUMN3_MAX_COUNT * BOLD_TEXT_WIDTH + COLUMN3_PADDING * 2
COLUMN1_ITEM_WIDTH                = COLUMN1_MAX_COUNT * TEXT_WIDTH + COLUMN1_PADDING * 2
COLUMN2_ITEM_WIDTH                = COLUMN2_MAX_COUNT * TEXT_WIDTH + COLUMN2_PADDING * 2
COLUMN3_ITEM_WIDTH                = COLUMN3_MAX_COUNT * TEXT_WIDTH + COLUMN3_PADDING * 2
ITEM_MARGIN                       = 6
ITEM_BACK_COLOR_1                 = 0x00EAEAEA
ITEM_BACK_COLOR_2                 = 0x00F4F4F4
ITEM_COUNT                        = 18 ; at current time we have 18 items
; ---------------------------------------------------------------------------- ;
COLUMN_Y                          = 10
COLUMN1_X                         = 10
COLUMN2_X                         = COLUMN1_X + COLUMN1_ITEM_WIDTH + ITEM_MARGIN
COLUMN3_X                         = COLUMN2_X + COLUMN2_ITEM_WIDTH + ITEM_MARGIN
; ---------------------------------------------------------------------------- ;
COLUMN1_TEXT_X                    = COLUMN1_X + COLUMN1_PADDING
COLUMN2_TEXT_X                    = COLUMN2_X + COLUMN2_PADDING
COLUMN3_TEXT_X                    = COLUMN3_X + COLUMN3_PADDING
; ---------------------------------------------------------------------------- ;
FRAME_TOP                         = COLUMN_Y - ITEM_MARGIN / 2
FRAME_BOTTOM                      = COLUMN_Y + ITEM_HEIGHT * ITEM_COUNT + ITEM_MARGIN / 2 - 1
; ---------------------------------------------------------------------------- ;
FRAME1_LEFT                       = COLUMN1_X - ITEM_MARGIN / 2
FRAME1_RIGHT                      = COLUMN1_X + COLUMN1_ITEM_WIDTH + ITEM_MARGIN / 2 - 1
; ---------------------------------------------------------------------------- ;
FRAME2_LEFT                       = COLUMN2_X - ITEM_MARGIN / 2
FRAME2_RIGHT                      = COLUMN2_X + COLUMN2_ITEM_WIDTH + ITEM_MARGIN / 2 - 1
; ---------------------------------------------------------------------------- ;
FRAME3_LEFT                       = COLUMN3_X - ITEM_MARGIN / 2
FRAME3_RIGHT                      = COLUMN3_X + COLUMN3_ITEM_WIDTH + ITEM_MARGIN / 2 - 1
; ---------------------------------------------------------------------------- ;
WINDOW_STYLE_SKINNED_FIXED        = 0x4000000
WINDOW_STYLE_COORD_CLIENT         = 0x20000000
WINDOW_STYLE_CAPTION              = 0x10000000
; ---------------------------------------------------------------------------- ;
WINDOW_BORDER_SIZE                = 5
WINDOW_WIDTH                      = FRAME3_RIGHT + FRAME1_LEFT + WINDOW_BORDER_SIZE * 2
WINDOW_STYLE                      = WINDOW_STYLE_SKINNED_FIXED or WINDOW_STYLE_COORD_CLIENT or WINDOW_STYLE_CAPTION
WINDOW_BACK_COLOR                 = 0x00FFFFFF
; ---------------------------------------------------------------------------- ;
INDICATOR_WIDTH                   = 3
INDICATOR_HEIGHT                  = 3
INDICATOR_LEFT                    = COLUMN3_X + COLUMN3_ITEM_WIDTH + ITEM_MARGIN / 2
INDICATOR_TOP                     = COLUMN_Y  - ITEM_MARGIN / 2 - INDICATOR_HEIGHT
; ---------------------------------------------------------------------------- ;
UPDATE_TIME                       = 28
; ---------------------------------------------------------------------------- ;
KEYBOARD_MODE_ASCII               = 0
KEYBOARD_MODE_SCAN                = 1
; ---------------------------------------------------------------------------- ;
WINDOW_STATE_MAXIMIZED            = 1
WINDOW_STATE_MINIMIZED            = 2
WINDOW_STATE_ROLLED_UP            = 4
; ---------------------------------------------------------------------------- ;
THREAD_STATE_RUNNING              = 0
THREAD_STATE_SUSPENDED            = 1
THREAD_STATE_SUSPENDED_WAIT_EVENT = 2
THREAD_STATE_NORMAL_TERMINATING   = 3
THREAD_STATE_EXCEPT_TERMINATING   = 4
THREAD_STATE_WAIT_EVENT           = 5
THREAD_STATE_SLOT_IS_FREE         = 9
; ---------------------------------------------------------------------------- ;


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
align 4
Pos:
.x                                dd COLUMN1_X
.y                                dd COLUMN_Y
; ---------------------------------------------------------------------------- ;
sz_caption                        db "ThreadInfo",0
; ---------------------------------------------------------------------------- ;
if lang eq ru_RU
sz_cpu_usage                      db "Использ. процессора",0
sz_win_stack_pos                  db "Win stack pos",0
sz_name                           db "Имя",0
sz_mem_address                    db "Адрес в памяти",0
sz_mem_usage                      db "Использовано памяти",0
sz_identifier                     db "Идентификатор",0
sz_x                              db "X",0
sz_y                              db "Y",0
sz_size_x                         db "Размер X",0
sz_size_y                         db "Размер Y",0
sz_thread_state                   db "Состояние процесса",0
sz_client_x                       db "Client X",0
sz_client_y                       db "Client Y",0
sz_client_size_x                  db "Client Size X",0
sz_client_size_y                  db "Client Size Y",0
sz_window_state                   db "Состояние окна",0
sz_event_mask                     db "Маска событий",0
sz_keyboard_mode                  db "Режим клавиатуры",0
else
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
end if
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
macro DrawCpuUsage {
; sz_cpu_usage
        stdcall   DrawText, [back_color],[fore_color],sz_cpu_usage,[Pos.y],COLUMN1_TEXT_X
; [cpu_usage]
        stdcall   uint2str, [thread_info.cpu_usage]
        stdcall   PadBuffSpaces, COLUMN2_MAX_COUNT
        stdcall   DrawText, [back_color],[fore_color],tmpbuffer,[Pos.y],COLUMN2_TEXT_X
        call   ChangeBackColor
        add    [Pos.y], dword ITEM_HEIGHT
}
; **************************************************************************** ;
macro DrawWinStackPos {
; sz_win_stack_pos
        stdcall   DrawText, [back_color],[fore_color],sz_win_stack_pos,[Pos.y],COLUMN1_TEXT_X
; [win_stack_pos]
        movzx  eax, word [thread_info.window_stack_position]
        stdcall   uint2str, eax
        stdcall   PadBuffSpaces, COLUMN2_MAX_COUNT
        stdcall   DrawText, [back_color],[fore_color],tmpbuffer,[Pos.y],COLUMN2_TEXT_X
        call   ChangeBackColor
        add    [Pos.y], dword ITEM_HEIGHT
}
; **************************************************************************** ;
macro DrawName {
; sz_name
        stdcall   DrawText, [back_color],[fore_color],sz_name,[Pos.y],COLUMN1_TEXT_X
; name
        stdcall   DrawText, [back_color],[fore_color],(thread_info.process_name),[Pos.y],COLUMN2_TEXT_X
        call   ChangeBackColor
        add    [Pos.y], dword ITEM_HEIGHT
}
; **************************************************************************** ;
macro DrawMemAddress {
; sz_mem_address
        stdcall   DrawText, [back_color],[fore_color],sz_mem_address,[Pos.y],COLUMN1_TEXT_X
; [mem_address]
        stdcall   uint2str, [thread_info.memory_start]
        stdcall   PadBuffSpaces, COLUMN2_MAX_COUNT
        stdcall   DrawText, [back_color],[fore_color],tmpbuffer,[Pos.y],COLUMN2_TEXT_X
        call   ChangeBackColor
        add    [Pos.y], dword ITEM_HEIGHT
}
; **************************************************************************** ;
macro DrawMemUsage {
; sz_mem_usage
        stdcall   DrawText, [back_color],[fore_color],sz_mem_usage,[Pos.y],COLUMN1_TEXT_X
; [mem_usage]
        stdcall   uint2str, [thread_info.used_memory]
        stdcall   PadBuffSpaces, COLUMN2_MAX_COUNT
        stdcall   DrawText, [back_color],[fore_color],tmpbuffer,[Pos.y],COLUMN2_TEXT_X
        call   ChangeBackColor
        add    [Pos.y], dword ITEM_HEIGHT
}
; **************************************************************************** ;
macro DrawIdentifier {
; sz_identifier
        stdcall   DrawText, [back_color],[fore_color],sz_identifier,[Pos.y],COLUMN1_TEXT_X
; [identifier]
        stdcall   uint2str, [thread_info.PID]
        stdcall   PadBuffSpaces, COLUMN2_MAX_COUNT
        stdcall   DrawText, [back_color],[fore_color],tmpbuffer,[Pos.y],COLUMN2_TEXT_X
        call   ChangeBackColor
        add    [Pos.y], dword ITEM_HEIGHT
}
; **************************************************************************** ;
macro DrawWindowX {
; sz_x
        stdcall   DrawText, [back_color],[fore_color],sz_x,[Pos.y],COLUMN1_TEXT_X
; [x]
        stdcall   uint2str, [thread_info.box.left]
        stdcall   PadBuffSpaces, COLUMN2_MAX_COUNT
        stdcall   DrawText, [back_color],[fore_color],tmpbuffer,[Pos.y],COLUMN2_TEXT_X
        call   ChangeBackColor
        add    [Pos.y], dword ITEM_HEIGHT
}
; **************************************************************************** ;
macro DrawWindowY {
; sz_y
        stdcall   DrawText, [back_color],[fore_color],sz_y,[Pos.y],COLUMN1_TEXT_X
; [y]
        stdcall   uint2str, [thread_info.box.top]
        stdcall   PadBuffSpaces, COLUMN2_MAX_COUNT
        stdcall   DrawText, [back_color],[fore_color],tmpbuffer,[Pos.y],COLUMN2_TEXT_X
        call   ChangeBackColor
        add    [Pos.y], dword ITEM_HEIGHT
}
; **************************************************************************** ;
macro DrawWindowSizeX {
; sz_size_x
        stdcall   DrawText, [back_color],[fore_color],sz_size_x,[Pos.y],COLUMN1_TEXT_X
; [size_x]
        stdcall   uint2str, [thread_info.box.width]
        stdcall   PadBuffSpaces, COLUMN2_MAX_COUNT
        stdcall   DrawText, [back_color],[fore_color],tmpbuffer,[Pos.y],COLUMN2_TEXT_X
        call   ChangeBackColor
        add    [Pos.y], dword ITEM_HEIGHT
}
; **************************************************************************** ;
macro DrawWindowSizeY {
; sz_size_y
        stdcall   DrawText, [back_color],[fore_color],sz_size_y,[Pos.y],COLUMN1_TEXT_X
; [size_y]
        stdcall   uint2str, [thread_info.box.height]
        stdcall   PadBuffSpaces, COLUMN2_MAX_COUNT
        stdcall   DrawText, [back_color],[fore_color],tmpbuffer,[Pos.y],COLUMN2_TEXT_X
        call   ChangeBackColor
        add    [Pos.y], dword ITEM_HEIGHT
}
; **************************************************************************** ;
macro DrawThreadState {
; sz_thread_state
        stdcall   DrawText, [back_color],[fore_color],sz_thread_state,[Pos.y],COLUMN1_TEXT_X
; decoded_thread_state & [thread_state]
        movzx  eax, word [thread_info.slot_state]
        push   eax ; for "call uint2str" below

        cmp    eax, THREAD_STATE_RUNNING
        jne    @f
        mov    eax, sz_running
        jmp    %%draw_decoded_thread_state
@@:
        cmp    eax, THREAD_STATE_SUSPENDED
        jne    @f
        mov    eax, sz_suspended
        jmp    %%draw_decoded_thread_state
@@:
        cmp    eax, THREAD_STATE_SUSPENDED_WAIT_EVENT
        jne    @f
        mov    eax, sz_suspended_wait_event
        jmp    %%draw_decoded_thread_state
@@:
        cmp    eax, THREAD_STATE_NORMAL_TERMINATING
        jne    @f
        mov    eax, sz_normal_terminating
        jmp    %%draw_decoded_thread_state
@@:
        cmp    eax, THREAD_STATE_EXCEPT_TERMINATING
        jne    @f
        mov    eax, sz_except_terminating
        jmp    %%draw_decoded_thread_state
@@:
        cmp    eax, THREAD_STATE_WAIT_EVENT
        jne    @f
        mov    eax, sz_wait_event
        jmp    %%draw_decoded_thread_state
@@:
        cmp    eax, THREAD_STATE_SLOT_IS_FREE
        jne    @f
        mov    eax, sz_slot_is_free
        jmp    %%draw_decoded_thread_state
@@:
        mov    eax, sz_undefined
%%draw_decoded_thread_state:
        stdcall   DrawText, [back_color],[fore_color],eax,[Pos.y],COLUMN3_TEXT_X
        call   uint2str
        stdcall   PadBuffSpaces, COLUMN2_MAX_COUNT
        stdcall   DrawText, [back_color],[fore_color],tmpbuffer,[Pos.y],COLUMN2_TEXT_X
        call   ChangeBackColor
        add    [Pos.y], dword ITEM_HEIGHT
}
; **************************************************************************** ;
macro DrawClientX {
; sz_client_x
        stdcall   DrawText, [back_color],[fore_color],sz_client_x,[Pos.y],COLUMN1_TEXT_X
; [client_x]
        stdcall   uint2str, [thread_info.client_box.left]
        stdcall   PadBuffSpaces, COLUMN2_MAX_COUNT
        stdcall   DrawText, [back_color],[fore_color],tmpbuffer,[Pos.y],COLUMN2_TEXT_X
        call   ChangeBackColor
        add    [Pos.y], dword ITEM_HEIGHT
}
; **************************************************************************** ;
macro DrawClientY {
; sz_client_y
        stdcall   DrawText, [back_color],[fore_color],sz_client_y,[Pos.y],COLUMN1_TEXT_X
; [client_y]
        stdcall   uint2str, [thread_info.client_box.top]
        stdcall   PadBuffSpaces, COLUMN2_MAX_COUNT
        stdcall   DrawText, [back_color],[fore_color],tmpbuffer,[Pos.y],COLUMN2_TEXT_X
        call   ChangeBackColor
        add    [Pos.y], dword ITEM_HEIGHT
}
; **************************************************************************** ;
macro DrawClientSizeX {
; sz_client_size_x
        stdcall   DrawText, [back_color],[fore_color],sz_client_size_x,[Pos.y],COLUMN1_TEXT_X
; [client_size_x]
        stdcall   uint2str, [thread_info.client_box.width]
        stdcall   PadBuffSpaces, COLUMN2_MAX_COUNT
        stdcall   DrawText, [back_color],[fore_color],tmpbuffer,[Pos.y],COLUMN2_TEXT_X
        call   ChangeBackColor
        add    [Pos.y], dword ITEM_HEIGHT
}
; **************************************************************************** ;
macro DrawClientSizeY {
; sz_client_size_y
        stdcall   DrawText, [back_color],[fore_color],sz_client_size_y,[Pos.y],COLUMN1_TEXT_X
; [client_size_y]
        stdcall   uint2str, [thread_info.client_box.height]
        stdcall   PadBuffSpaces, COLUMN2_MAX_COUNT
        stdcall   DrawText, [back_color],[fore_color],tmpbuffer,[Pos.y],COLUMN2_TEXT_X
        call   ChangeBackColor
        add    [Pos.y], dword ITEM_HEIGHT
}
; **************************************************************************** ;
macro DrawWindowState {
; sz_window_state
        stdcall   DrawText, [back_color],[fore_color],sz_window_state,[Pos.y],COLUMN1_TEXT_X
; decoded_window_state & [window_state]
        movzx  eax, byte [thread_info.wnd_state]
        push   eax ; for "call uint2str" below
        mov    ebx, eax
        mov    [tmpbuffer], byte 0

        test   ebx, WINDOW_STATE_MAXIMIZED
        jz     @f
        stdcall   StringConcatenate, sz_maximized,tmpbuffer
@@:
        test   ebx, WINDOW_STATE_MINIMIZED
        jz     @f
        stdcall   StringConcatenate, sz_minimized,tmpbuffer
@@:
        test   ebx, WINDOW_STATE_ROLLED_UP
        jz     @f
        stdcall   StringConcatenate, sz_rolled_up,tmpbuffer
@@:
        stdcall   PadBuffSpaces, COLUMN3_MAX_COUNT
        stdcall   DrawText, [back_color],[fore_color],tmpbuffer,[Pos.y],COLUMN3_TEXT_X
        call   uint2str
        stdcall   PadBuffSpaces, COLUMN2_MAX_COUNT
        stdcall   DrawText, [back_color],[fore_color],tmpbuffer,[Pos.y],COLUMN2_TEXT_X
        call   ChangeBackColor
        add    [Pos.y], dword ITEM_HEIGHT
}
; **************************************************************************** ;
macro DrawEventMask {
; sz_event_mask
        stdcall   DrawText, [back_color],[fore_color],sz_event_mask,[Pos.y],COLUMN1_TEXT_X
; decoded_event_mask & [event_mask]
        mov    eax, [thread_info.event_mask]
        push   eax ; for "call uint2str" below
        mov    ebx, eax
        mov    [tmpbuffer], byte 0
        test   ebx, EVM_REDRAW
        jz     @f
        stdcall   StringConcatenate, sz_redraw,tmpbuffer
@@:
        test   ebx, EVM_KEY
        jz     @f
        stdcall   StringConcatenate, sz_key,tmpbuffer
@@:
        test   ebx, EVM_BUTTON
        jz     @f
        stdcall   StringConcatenate, sz_button,tmpbuffer
@@:
        test   ebx, EVM_EXIT
        jz     @f
        stdcall   StringConcatenate, sz_reserved0,tmpbuffer
@@:
        test   ebx, EVM_BACKGROUND
        jz     @f
        stdcall   StringConcatenate, sz_redraw_background,tmpbuffer
@@:
        test   ebx, EVM_MOUSE
        jz     @f
        stdcall   StringConcatenate, sz_mouse,tmpbuffer
@@:
        test   ebx, EVM_IPC
        jz     @f
        stdcall   StringConcatenate, sz_ipc,tmpbuffer
@@:
        test   ebx, EVM_STACK
        jz     @f
        stdcall   StringConcatenate, sz_network,tmpbuffer
@@:
        test   ebx, EVM_DEBUG
        jz     @f
        stdcall   StringConcatenate, sz_debug,tmpbuffer
@@:
        stdcall   PadBuffSpaces, COLUMN3_MAX_COUNT
        stdcall   DrawText, [back_color],[fore_color],tmpbuffer,[Pos.y],COLUMN3_TEXT_X
        call   uint2str
        stdcall   PadBuffSpaces, COLUMN2_MAX_COUNT
        stdcall   DrawText, [back_color],[fore_color],tmpbuffer,[Pos.y],COLUMN2_TEXT_X
        call   ChangeBackColor
        add    [Pos.y], dword ITEM_HEIGHT
}
; **************************************************************************** ;
macro DrawKeyboardMode {
        stdcall   PadBuffSpaces, COLUMN2_MAX_COUNT
; sz_keyboard_mode
        stdcall   DrawText, [back_color],[fore_color],sz_keyboard_mode,[Pos.y],COLUMN1_TEXT_X
; decoded_keyboard_mode & [keyboard_mode]
        movzx  eax, byte [thread_info.keyboard_mode]
        push   eax ; for "call uint2str" below

        cmp    eax, KEYBOARD_MODE_ASCII
        jne    @f
        mov    eax, sz_ascii
        jmp    %%draw_decoded_keyboard_mode
@@:
        cmp    eax, KEYBOARD_MODE_SCAN
        jne    @f
        mov    eax, sz_scan
        jmp    %%draw_decoded_keyboard_mode
@@:
        mov    eax, sz_undefined
%%draw_decoded_keyboard_mode:
        stdcall   DrawText, [back_color],[fore_color],eax,[Pos.y],COLUMN3_TEXT_X
        call   uint2str
        stdcall   DrawText, [back_color],[fore_color],tmpbuffer,[Pos.y],COLUMN2_TEXT_X
        call   ChangeBackColor
        add    [Pos.y], dword ITEM_HEIGHT
}
; **************************************************************************** ;
macro DrawUpdateIndicator {
        mov    eax, [IndicatorColors]
        sub    eax, 3
        neg    eax
        mov    [IndicatorColors], eax
; draw.rectangle
        mov    edx, [eax * 4 + IndicatorColors]
        mcall SF_DRAW_RECT, (INDICATOR_LEFT shl 16) or INDICATOR_WIDTH, (INDICATOR_TOP shl 16) or INDICATOR_HEIGHT
}
; **************************************************************************** ;
macro DrawTable {
; DrawFrames
        stdcall   DrawFrame, [frame_color],FRAME_BOTTOM,FRAME1_RIGHT,FRAME_TOP,FRAME1_LEFT
        stdcall   DrawFrame, [frame_color],FRAME_BOTTOM,FRAME2_RIGHT,FRAME_TOP,FRAME2_LEFT
        stdcall   DrawFrame, [frame_color],FRAME_BOTTOM,FRAME3_RIGHT,FRAME_TOP,FRAME3_LEFT
; DrawItems
        mov    esi, COLUMN_Y
        xor    edi, edi
@@:
        cmp    edi, ITEM_COUNT
        jnl    @f
        mov    ecx, esi
        shl    ecx, 16
        or     ecx, ITEM_HEIGHT
; draw rectangles
        mcall SF_DRAW_RECT, (COLUMN1_X shl 16) or COLUMN1_ITEM_WIDTH,, [back_color]
        mcall , (COLUMN2_X shl 16) or COLUMN2_ITEM_WIDTH
        mcall , (COLUMN3_X shl 16) or COLUMN3_ITEM_WIDTH
        call   ChangeBackColor
        add    esi, ITEM_HEIGHT
        inc    edi
        jmp    @b
@@:
}
; ---------------------------------------------------------------------------- ;
align 4
program.start:
        mov    esi, program.params
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
        mcall SF_GET_GRAPHICAL_PARAMS,SSF_SCREEN_SIZE
        mov    [screen], eax
; skin.height
        mcall SF_STYLE_SETTINGS,SSF_GET_SKIN_HEIGHT
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
        mcall SF_SET_EVENTS_MASK, EVM_REDRAW or EVM_BUTTON
; ---------------------------------------------------------------------------- ;
align 4
on_redraw:
; redraw.start
        mcall SF_REDRAW,SSF_BEGIN_DRAW
; draw.window
        mov    ebx, [window.left]
        mov    ecx, [window.top]
        shl    ebx, 16
        shl    ecx, 16
        or     ebx, [window.width]
        or     ecx, [window.height]
        mov    edx, WINDOW_STYLE or WINDOW_BACK_COLOR
        mov    edi, sz_caption
        xor    esi, esi
        mcall SF_CREATE_WINDOW
; redraw.finish
        mcall SF_REDRAW,SSF_END_DRAW
        DrawTable
        call   UpdateThreadInfo
align 4
wait.event.by.time:
        mcall SF_WAIT_EVENT_TIMEOUT,UPDATE_TIME
        dec    eax
        jz     on_redraw        ; IF      eax = 1 THEN   redraw
        jns    on_button        ; ELSEIF  eax = 2 THEN   button
        call   UpdateThreadInfo ; ELSE no event -> update thread info
        jmp    wait.event.by.time
align 4
on_button: ; terminate because we have only one button(close button)
        mcall SF_TERMINATE_PROCESS
; ---------------------------------------------------------------------------- ;
align 4
UpdateThreadInfo:
; get.thread.info
        mcall SF_THREAD_INFO, thread_info, [slot]

        mov    [Pos.x], dword COLUMN1_X
        mov    [Pos.y], dword COLUMN_Y + (ITEM_HEIGHT - TEXT_HEIGHT) / 2
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
Color  equ [esp +  4 +4]
Bottom equ [esp +  8 +4]
Top    equ [esp + 16 +4]
Right  equ [esp + 12 +4]
Left   equ [esp + 20 +4]
        push   ebp
		mov    edi, Top
        mov    ebp, Right
        mov    ebx, Left
        shl    ebx, 16
        mov    bx, bp
        shrd   ecx, edi, 16
        mov    cx, di
        mcall SF_DRAW_LINE,,, Color, Bottom
        shrd   ecx, esi, 16
        mov    cx, si
        mcall
        shld   esi, ebx, 16
        mov    bx, si
        shrd   ecx, edi, 16
        mcall
        shrd   ebx, ebp, 16
        mov    bx, bp
        mcall
        pop    ebp
        ret    20
purge Color
purge Bottom
purge Top
purge Right
purge Left
; ---------------------------------------------------------------------------- ;
align 4
proc StringConcatenate, stradd:dword, str1:dword
        mov    esi, [stradd]
        or     ecx, -1
        mov    edi, esi
        xor    eax, eax
        repne scasb
        mov    edx, ecx
        mov    edi, [str1]
        repne scasb
        dec    edi
        not    edx
        mov    eax, [str1]
        mov    ecx, edx
        shr    ecx, 2
        and    edx, 3
        rep movsd
        mov    ecx, edx
        rep movsb
        ret
endp
; ---------------------------------------------------------------------------- ;
align 4
proc DrawText, b_color:dword, f_color:dword, text:dword, y:dword, x:dword
        mov    ebx, [x]
        shl    ebx, 16
        or     ebx, [y]
        mov    ecx, [f_color]
        or     ecx, 0xD0000000
        mcall SF_DRAW_TEXT,,, [text],, [b_color]
        ret
endp
; ---------------------------------------------------------------------------- ;
align 4
proc DrawTextBold, b_color:dword, f_color:dword, text:dword, y:dword, x:dword
        mov    ecx, [f_color]
        mov    edi, [b_color]
        mov    esi, 1 ; count
        mov    edx, [text]
        mov    ebx, [x]
        shl    ebx, 16
        or     ebx, [y]
align 4
@@:
        test   [edx], byte 0xFF
        jz     @f
        or     ecx, 0x50000000
        mcall SF_DRAW_TEXT
        add    ebx, (1 shl 16)
        and    ecx, 0x10FFFFFF
        mcall
        add    ebx, (TEXT_WIDTH shl 16)
        inc    edx
        jmp    @b
align 4
@@:
        ret
endp
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
        stdcall   ConvertToBase, tmpbuffer,10,[esp + 4]
        ret    4
; ---------------------------------------------------------------------------- ;
align 4
ConvertToBase:
value  equ [esp + 12]        ; value treated as unsigned
base   equ [esp +  8]        ; 2 <= base <= 36
buffer equ [esp +  4]        ; SizeOf(buffer) => (32 + 1)
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
purge value
purge base
purge buffer
align 4
.digits  db    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
; ---------------------------------------------------------------------------- ;
align 4
PadBuffSpaces:
maxlen equ [esp + 4]
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
purge maxlen
; ---------------------------------------------------------------------------- ;
align 4
program.end:
	program.path rb 1024
	program.params rb 256
thread_info process_information

tmpbuffer rb 64
	rb 512
align 16
program.stack:
program.memory: