; SPDX-License-Identifier: GPL-2.0-only
; SPDX-FileCopyrightText: 2024 KolibriOS-NG Team

format binary as ""
use32
org     0
db      'MENUET01'    ; signature
dd      1             ; header version
dd      start         ; entry point
dd      _image_end    ; end of image
dd      _memory       ; required memory size
dd      _stacktop     ; address of stack top
dd      0             ; buffer for command line arguments
dd      0             ; buffer for path

include '../../macros.inc'
include '../../KOSfuncs.inc'

TEXT_WIDTH                        = 8
TEXT_HEIGHT                       = 16

MOUSE_LEFT_BUTTON_MASK            =         1b
MOUSE_RIGHT_BUTTON_MASK           =        10b
MOUSE_MIDDLE_BUTTON_MASK          =       100b

WINDOW_STYLE_SKINNED_FIXED        =  0x4000000
WINDOW_STYLE_COORD_CLIENT         = 0x20000000
WINDOW_STYLE_CAPTION              = 0x10000000

WINDOW_BORDER_SIZE                = 5
WINDOW_STYLE                      = (WINDOW_STYLE_SKINNED_FIXED or WINDOW_STYLE_COORD_CLIENT or WINDOW_STYLE_CAPTION)

MOUSE_BODY_COLOR                  = 0x007C7C96
MOUSE_LEFT_BUTTON_COLOR           = 0x008293A4
MOUSE_RIGHT_BUTTON_COLOR          = 0x008293A4
MOUSE_MIDDLE_BUTTON_COLOR         = 0x00A48293
MOUSE_LEFT_BUTTON_PRESSED_COLOR   = 0x00568EC7
MOUSE_RIGHT_BUTTON_PRESSED_COLOR  = 0x00568EC7
MOUSE_MIDDLE_BUTTON_PRESSED_COLOR = 0x00C7568E
WINDOW_BACK_COLOR                 = 0x00EFEFEF

MOUSE_WIDTH                       = 180
MOUSE_HEIGHT                      = 240
MOUSE_MARGIN                      = 4
BUTTONS_MARGIN                    = 2

MOUSE_LEFT                        = MOUSE_MARGIN
MOUSE_TOP                         = MOUSE_MARGIN
WINDOW_WIDTH                      = MOUSE_WIDTH + WINDOW_BORDER_SIZE * 2 + MOUSE_MARGIN * 2
MOUSE_BODY_HEIGHT                 = (MOUSE_HEIGHT - BUTTONS_MARGIN) / 2
MOUSE_BODY_TOP                    = MOUSE_HEIGHT  - MOUSE_BODY_HEIGHT + MOUSE_TOP
LEFT_BUTTON_HEIGHT                = MOUSE_HEIGHT - MOUSE_BODY_HEIGHT - BUTTONS_MARGIN
RIGHT_BUTTON_HEIGHT               = MOUSE_HEIGHT - MOUSE_BODY_HEIGHT - BUTTONS_MARGIN
LEFT_BUTTON_WIDTH                 = (MOUSE_WIDTH  - BUTTONS_MARGIN) / 2
RIGHT_BUTTON_WIDTH                = MOUSE_WIDTH  - LEFT_BUTTON_WIDTH - BUTTONS_MARGIN
LEFT_BUTTON_LEFT                  = MOUSE_LEFT
RIGHT_BUTTON_LEFT                 = LEFT_BUTTON_LEFT + LEFT_BUTTON_WIDTH + BUTTONS_MARGIN
MIDDLE_BUTTON_WIDTH               = MOUSE_WIDTH / 10
MIDDLE_BUTTON_HEIGHT              = MOUSE_HEIGHT / 6
MIDDLE_BUTTON_LEFT                = (MOUSE_WIDTH - MIDDLE_BUTTON_WIDTH) / 2 + MOUSE_LEFT
MIDDLE_BUTTON_TOP                 = (MOUSE_WIDTH / 2 - MIDDLE_BUTTON_WIDTH) / 2 + MOUSE_TOP

; data:
mouse_left_button_color           dd MOUSE_LEFT_BUTTON_COLOR
mouse_right_button_color          dd MOUSE_RIGHT_BUTTON_COLOR
mouse_middle_button_color         dd MOUSE_MIDDLE_BUTTON_COLOR

mouse_button                      dd 0
sz_caption                        db "MouseState", 0

sz_button_state                   db "Buttons state:", 0
STATE_VALUES_WIDTH                = ($ - sz_button_state - 1)*TEXT_WIDTH
sz_bin                            db "bin: ", 0
LEN_SZ_BIN                        = $ - sz_bin - 1
sz_hex                            db "hex: 0x", 0
LEN_SZ_HEX                        = $ - sz_hex - 1

STATE_VALUES_HEIGHT               = 3*TEXT_HEIGHT ; we have three lines of text
STATE_VALUES_TOP                  = ((MOUSE_BODY_HEIGHT - STATE_VALUES_HEIGHT) / 2 + MOUSE_BODY_TOP)
STATE_VALUES_LEFT                 = ((MOUSE_WIDTH - STATE_VALUES_WIDTH) / 2 + MOUSE_LEFT)



align 4
draw_mouse_buttons:
        mov     eax, SF_DRAW_RECT

        mov     ebx, (LEFT_BUTTON_LEFT shl 16) or LEFT_BUTTON_WIDTH
        mov     ecx, (MOUSE_TOP shl 16) or LEFT_BUTTON_HEIGHT
        mov     edx, [mouse_left_button_color]
        mcall

        mov     ebx, (RIGHT_BUTTON_LEFT shl 16) or RIGHT_BUTTON_WIDTH
        mov     ecx, (MOUSE_TOP shl 16) or RIGHT_BUTTON_HEIGHT
        mov     edx, [mouse_right_button_color]
        mcall

        mov     ebx, (MIDDLE_BUTTON_LEFT shl 16) or MIDDLE_BUTTON_WIDTH
        mov     ecx, (MIDDLE_BUTTON_TOP shl 16) or MIDDLE_BUTTON_HEIGHT
        mov     edx, [mouse_middle_button_color]
        mcall
; draw state values
        mov     eax, SF_DRAW_NUMBER
        mov     esi, (0101b shl 28) or WINDOW_BACK_COLOR
        mov     ecx, [mouse_button]
        mov     edi, MOUSE_BODY_COLOR

        mov     ebx, (10 shl 16) or (2 shl 8) ; 10 digits, base2
        mov     edx, ((LEN_SZ_BIN * TEXT_WIDTH + STATE_VALUES_LEFT) shl 16) or (STATE_VALUES_TOP + TEXT_HEIGHT)
        mcall

        mov     ebx, (8 shl 16) or (1 shl 8) ; 8 digits, base16
        mov     edx, ((LEN_SZ_HEX * TEXT_WIDTH + STATE_VALUES_LEFT) shl 16) or (STATE_VALUES_TOP + TEXT_HEIGHT * 2)
        mcall
        ret


align 4
start:
        mcall   SF_GET_GRAPHICAL_PARAMS, SSF_SCREEN_SIZE   
        mov     edx, eax
        movzx   ecx, ax
        shr     edx, 16

        mcall   SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
        add     eax, MOUSE_HEIGHT + WINDOW_BORDER_SIZE + MOUSE_MARGIN * 2 - 1
        mov     esi, eax
        sub     edx, (WINDOW_WIDTH - 1)
        sub     ecx, eax
        shr     edx, 1
        shr     ecx, 1

        mov     eax, SF_SET_EVENTS_MASK
        mov     ebx, EVM_REDRAW or EVM_BUTTON or EVM_MOUSE
        mcall


align 4
on_redraw:
        mcall   SF_REDRAW, SSF_BEGIN_DRAW
        xor     eax, eax ; SF_CREATE_WINDOW
        mov     ebx, edx ; window.left
; ecx = window.top
        shl     ebx, 16
        shl     ecx, 16
        or      ebx, (WINDOW_WIDTH - 1)
        or      ecx, esi ; window.height
        mov     edx, WINDOW_STYLE or WINDOW_BACK_COLOR
        mov     edi, sz_caption
        xor     esi, esi
        mcall

        mcall   SF_REDRAW, SSF_END_DRAW
; draw mouse body:
        mov     eax, SF_DRAW_RECT
        mov     ebx, (MOUSE_LEFT shl 16) or MOUSE_WIDTH
        mov     ecx, (MOUSE_BODY_TOP shl 16) or MOUSE_BODY_HEIGHT
        mov     edx, MOUSE_BODY_COLOR
        mcall

        mov     eax, SF_DRAW_TEXT
        mov     ecx, (1101b shl 28) or WINDOW_BACK_COLOR
        mov     edi, MOUSE_BODY_COLOR

        mov     ebx, (STATE_VALUES_LEFT shl 16) or STATE_VALUES_TOP
        mov     edx, sz_button_state
        mcall

        add     ebx, TEXT_HEIGHT
        mov     edx, sz_bin
        mcall

        add     ebx, TEXT_HEIGHT
        mov     edx, sz_hex
        mcall
        call    draw_mouse_buttons


align 4
wait_event:
        mcall   SF_WAIT_EVENT
        cmp     eax, EV_REDRAW
        je      on_redraw
        cmp     eax, EV_BUTTON
        je      on_button
        ; otherwise mouse:

        mcall   SF_MOUSE_GET, SSF_BUTTON
        cmp     [mouse_button], eax ; if equal, then no need update
        je      wait_event
        mov     ebx, dword MOUSE_LEFT_BUTTON_COLOR
        mov     ecx, dword MOUSE_RIGHT_BUTTON_COLOR
        mov     edx, dword MOUSE_MIDDLE_BUTTON_COLOR
.left:
        test    eax, MOUSE_LEFT_BUTTON_MASK
        jz      .right
        mov     ebx, dword MOUSE_LEFT_BUTTON_PRESSED_COLOR
.right:
        test    eax, MOUSE_RIGHT_BUTTON_MASK
        jz      .middle
        mov     ecx, dword MOUSE_RIGHT_BUTTON_PRESSED_COLOR
.middle:
        test    eax, MOUSE_MIDDLE_BUTTON_MASK
        jz      .other
        mov     edx, dword MOUSE_MIDDLE_BUTTON_PRESSED_COLOR
.other:
        mov     [mouse_left_button_color], ebx
        mov     [mouse_right_button_color], ecx
        mov     [mouse_middle_button_color], edx
        mov     [mouse_button], eax
        call    draw_mouse_buttons
        jmp     wait_event

align 4
on_button:
; terminate because we have only one button (close)
        mcall   SF_TERMINATE_PROCESS

align 16
_image_end:

; reserve for stack:
        rb      256
align 16
_stacktop:
_memory:

