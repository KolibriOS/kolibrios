;   This program shows  state of mouse buttons   ;
;  to compile: nasm -f bin mstate.asm -o mstate  ;
ORG 0
BITS 32
; ---------------------------------------------------------------------------- ;
PATH_SIZE                         equ 256
PARAMS_SIZE                       equ 256
STACK_SIZE                        equ 256
; ---------------------------------------------------------------------------- ;
TEXT_WIDTH                        equ 6
TEXT_HEIGHT                       equ 9
; ---------------------------------------------------------------------------- ;
MOUSE_LEFT_BUTTON_MASK            equ         1b
MOUSE_RIGHT_BUTTON_MASK           equ        10b
MOUSE_MIDDLE_BUTTON_MASK          equ       100b
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
WINDOW_STYLE_SKINNED_FIXED        equ 0x4000000
WINDOW_STYLE_COORD_CLIENT         equ 0x20000000
WINDOW_STYLE_CAPTION              equ 0x10000000
; ---------------------------------------------------------------------------- ;
WINDOW_BORDER_SIZE                equ 5
; ---------------------------------------------------------------------------- ;
WINDOW_STYLE                      equ WINDOW_STYLE_SKINNED_FIXED | WINDOW_STYLE_COORD_CLIENT | WINDOW_STYLE_CAPTION
; ---------------------------------------------------------------------------- ;
MOUSE_BODY_COLOR                  equ 0x007C7C96
MOUSE_LEFT_BUTTON_COLOR           equ 0x008293A4
MOUSE_RIGHT_BUTTON_COLOR          equ 0x008293A4
MOUSE_MIDDLE_BUTTON_COLOR         equ 0x00A48293
MOUSE_LEFT_BUTTON_PRESSED_COLOR   equ 0x00568EC7
MOUSE_RIGHT_BUTTON_PRESSED_COLOR  equ 0x00568EC7
MOUSE_MIDDLE_BUTTON_PRESSED_COLOR equ 0x00C7568E
WINDOW_BACK_COLOR                 equ 0x00EFEFEF
; ---------------------------------------------------------------------------- ;
MOUSE_WIDTH                       equ 120
MOUSE_HEIGHT                      equ 240
MOUSE_MARGIN                      equ 4
BUTTONS_MARGIN                    equ 2
; ---------------------------------------------------------------------------- ;
MOUSE_LEFT                        equ MOUSE_MARGIN
MOUSE_TOP                         equ MOUSE_MARGIN
WINDOW_WIDTH                      equ MOUSE_WIDTH + WINDOW_BORDER_SIZE * 2 + MOUSE_MARGIN * 2
MOUSE_BODY_HEIGHT                 equ (MOUSE_HEIGHT - BUTTONS_MARGIN) / 2
MOUSE_BODY_TOP                    equ MOUSE_HEIGHT  - MOUSE_BODY_HEIGHT + MOUSE_TOP
LEFT_BUTTON_HEIGHT                equ MOUSE_HEIGHT - MOUSE_BODY_HEIGHT - BUTTONS_MARGIN
RIGHT_BUTTON_HEIGHT               equ MOUSE_HEIGHT - MOUSE_BODY_HEIGHT - BUTTONS_MARGIN
LEFT_BUTTON_WIDTH                 equ (MOUSE_WIDTH  - BUTTONS_MARGIN) / 2
RIGHT_BUTTON_WIDTH                equ MOUSE_WIDTH  - LEFT_BUTTON_WIDTH - BUTTONS_MARGIN
LEFT_BUTTON_LEFT                  equ MOUSE_LEFT
RIGHT_BUTTON_LEFT                 equ LEFT_BUTTON_LEFT + LEFT_BUTTON_WIDTH + BUTTONS_MARGIN
MIDDLE_BUTTON_WIDTH               equ MOUSE_WIDTH / 10
MIDDLE_BUTTON_HEIGHT              equ MOUSE_HEIGHT / 6
MIDDLE_BUTTON_LEFT                equ (MOUSE_WIDTH - MIDDLE_BUTTON_WIDTH) / 2 + MOUSE_LEFT
MIDDLE_BUTTON_TOP                 equ (MOUSE_WIDTH / 2 - MIDDLE_BUTTON_WIDTH) / 2 + MOUSE_TOP
; ---------------------------------------------------------------------------- ;
%define SZ_BUTTONS_STATE "Buttons state:"
%define SZ_BIN "bin:"
%define SZ_HEX "hex:0x"
%strlen LEN_SZ_BUTTONS_STATE SZ_BUTTONS_STATE
%strlen LEN_SZ_BIN SZ_BIN
%strlen LEN_SZ_HEX SZ_HEX
; ---------------------------------------------------------------------------- ;
STATE_VALUES_HEIGHT               equ 3 * TEXT_HEIGHT ; we have three lines of text
STATE_VALUES_WIDTH                equ LEN_SZ_BUTTONS_STATE * TEXT_WIDTH
STATE_VALUES_TOP                  equ (MOUSE_BODY_HEIGHT - STATE_VALUES_HEIGHT) / 2 + MOUSE_BODY_TOP
STATE_VALUES_LEFT                 equ (MOUSE_WIDTH - STATE_VALUES_WIDTH) / 2 + MOUSE_LEFT
; ---------------------------------------------------------------------------- ;
MENUET01                          db 'MENUET01'
version                           dd 1
program.start                     dd START
program.end                       dd _END
program.memory                    dd _END + PATH_SIZE + PARAMS_SIZE + STACK_SIZE
program.stack                     dd _END + PATH_SIZE + PARAMS_SIZE + STACK_SIZE
program.params                    dd _END + PATH_SIZE
program.path                      dd _END
; ---------------------------------------------------------------------------- ;
mouse_body_color                  dd MOUSE_BODY_COLOR
mouse_left_button_color           dd MOUSE_LEFT_BUTTON_COLOR
mouse_right_button_color          dd MOUSE_RIGHT_BUTTON_COLOR
mouse_middle_button_color         dd MOUSE_MIDDLE_BUTTON_COLOR
; ---------------------------------------------------------------------------- ;
mouse.button                      dd 0
; ---------------------------------------------------------------------------- ;
sz_caption                        db "MouseState",0
; ---------------------------------------------------------------------------- ;
sz_button_state                   db SZ_BUTTONS_STATE,0
sz_bin                            db SZ_BIN,0
sz_hex                            db SZ_HEX,0
; ---------------------------------------------------------------------------- ;
%macro DrawMouseBody 0
; draw.rectangle
        mov    eax, 13
        mov    ebx, MOUSE_LEFT         << 16 | MOUSE_WIDTH
        mov    ecx, MOUSE_BODY_TOP     << 16 | MOUSE_BODY_HEIGHT
        mov    edx, [mouse_body_color]
        int    64
; texts
        mov    eax, 4
        mov    ecx, 1100b << 28 | WINDOW_BACK_COLOR
        mov    edi, [mouse_body_color]
; draw.text
        mov    ebx, (STATE_VALUES_LEFT << 16) | STATE_VALUES_TOP
        mov    edx, sz_button_state
        int    64
; draw.text
        add    ebx, TEXT_HEIGHT
        mov    edx, sz_bin
        int    64
; draw.text
        add    ebx, TEXT_HEIGHT
        mov    edx, sz_hex
        int    64
%endmacro
; ---------------------------------------------------------------------------- ;
align 4
DrawMouseButtons:
        mov    eax, 13
; draw.rectangle
        mov    ebx, LEFT_BUTTON_LEFT   << 16 | LEFT_BUTTON_WIDTH
        mov    ecx, MOUSE_TOP          << 16 | LEFT_BUTTON_HEIGHT
        mov    edx, [mouse_left_button_color]
        int    64
; draw.rectangle
        mov    ebx, RIGHT_BUTTON_LEFT  << 16 | RIGHT_BUTTON_WIDTH
        mov    ecx, MOUSE_TOP          << 16 | RIGHT_BUTTON_HEIGHT
        mov    edx, [mouse_right_button_color]
        int    64
; draw.rectangle
        mov    ebx, MIDDLE_BUTTON_LEFT << 16 | MIDDLE_BUTTON_WIDTH
        mov    ecx, MIDDLE_BUTTON_TOP  << 16 | MIDDLE_BUTTON_HEIGHT
        mov    edx, [mouse_middle_button_color]
        int    64
; Draw State Values
        mov    eax, 47
        mov    esi, 0100b << 28 | WINDOW_BACK_COLOR
        mov    ecx, [mouse.button]
        mov    edi, [mouse_body_color]
; draw.number
        mov    ebx, (10 << 16) | (2 << 8) ; 10 digits, base2
        mov    edx, (LEN_SZ_BIN * TEXT_WIDTH + STATE_VALUES_LEFT) << 16 | (STATE_VALUES_TOP + TEXT_HEIGHT)
        int    64
; draw.number
        mov    ebx, (8 << 16) | (1 << 8) ; 8 digits, base16
        mov    edx, (LEN_SZ_HEX * TEXT_WIDTH + STATE_VALUES_LEFT) << 16 | (STATE_VALUES_TOP + TEXT_HEIGHT * 2)
        int    64
        ret
; ---------------------------------------------------------------------------- ;
align 4
START:
; get.screen.size
        mov    eax, 61
        mov    ebx, 1
        int    64        
        mov    edx, eax
        movzx  ecx, ax
        shr    edx, 16
; skin.height
        mov    eax, 48
        mov    ebx, 4
        int    64
        add    eax, MOUSE_HEIGHT + WINDOW_BORDER_SIZE + MOUSE_MARGIN * 2 - 1
        mov    esi, eax
        sub    edx, (WINDOW_WIDTH - 1)
        sub    ecx, eax
        shr    edx, 1
        shr    ecx, 1
; set.event
        mov    eax, 40
        mov    ebx, EM_REDRAW | EM_BUTTON | EM_MOUSE
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
        mov    ebx, edx ; window.left
; ecx = window.top
        shl    ebx, 16
        shl    ecx, 16
        or     ebx, (WINDOW_WIDTH - 1)
        or     ecx, esi ; window.height
        mov    edx, WINDOW_STYLE | WINDOW_BACK_COLOR
        mov    edi, sz_caption
        xor    esi, esi
        int    64
; redraw.finish
        mov    eax, 12
        mov    ebx, 2
        int    64
        DrawMouseBody
        call   DrawMouseButtons
align 4
wait.event:
        mov    eax, 10    ; redraw = 001b; 001b & 110b = 000b
        int    64         ; button = 011b; 011b & 110b = 010b
        test   eax, 110b  ; mouse  = 110b; 110b & 110b = 110b
        jz     on_redraw
        jnp    on_button
; get.mouse.button
        mov    eax, 37
        mov    ebx, 2
        int    64
        cmp    [mouse.button], eax ;      if equal
        je     wait.event          ; then no need update
        mov    ebx, dword MOUSE_LEFT_BUTTON_COLOR
        mov    ecx, dword MOUSE_RIGHT_BUTTON_COLOR
        mov    edx, dword MOUSE_MIDDLE_BUTTON_COLOR
.left:
        test   eax, MOUSE_LEFT_BUTTON_MASK
        jz     .right
        mov    ebx, dword MOUSE_LEFT_BUTTON_PRESSED_COLOR
.right:
        test   eax, MOUSE_RIGHT_BUTTON_MASK
        jz     .middle
        mov    ecx, dword MOUSE_RIGHT_BUTTON_PRESSED_COLOR
.middle:
        test   eax, MOUSE_MIDDLE_BUTTON_MASK
        jz     .other
        mov    edx, dword MOUSE_MIDDLE_BUTTON_PRESSED_COLOR
.other:
        mov    [mouse_left_button_color], ebx
        mov    [mouse_right_button_color], ecx
        mov    [mouse_middle_button_color], edx
        mov    [mouse.button], eax
        call   DrawMouseButtons
        jmp    wait.event
align 4
on_button: ; terminate because we have only one button(close button)
        or     eax, -1
        int    64
; ---------------------------------------------------------------------------- ;
align 4
_END:
