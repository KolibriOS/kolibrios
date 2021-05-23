; Author: rgimad (2021)
; License: GNU GPL v2

format binary as ""
use32
org     0
db      'MENUET01'    ; signature
dd      1             ; header version
dd      start         ; entry point
dd      _i_end         ; end of image
dd      _mem          ; required memory size
dd      _stacktop      ; address of stack top
dd      cmdline       ; buffer for command line arguments
dd      0             ; buffer for path

include 'constants.inc'

; application's entry point
align 4
start:
        mcall   68, 11
        mcall   68, 12, IMGBUF_SIZE
        mov     dword [imgbuf_ptr], eax

        stdcall chip8_init ; initialize

        DEBUGF  DBG_INFO, "app started, args = %s\n", cmdline
        DEBUGF  DBG_INFO, "MAX_GAME_SIZE = %x = %u\n", MAX_GAME_SIZE, MAX_GAME_SIZE

;        xor     ecx, ecx
; @@:
;        mov     al, byte [chip8_fontset + ecx]
;        DEBUGF  DBG_INFO, "%x ", al
;        cmp     ecx, 79
;        je      @f
;        inc     ecx
;        jmp     @b
; @@:

        mov     dword [fread_struct.filename], cmdline
        stdcall chip8_loadgame, fread_struct
        jz      .file_not_found

        DEBUGF  DBG_INFO, "file was read. bytes: %x %x %x..\n", [memory + 0x200], [memory + 0x200 + 4], [memory + 0x200 + 8]
        
        ; mov     byte [gfx + 5], 1
        ; mov     byte [gfx + 64], 1
        ; mov     byte [gfx + 65], 1
        ; mov     byte [gfx + 64*2 + 3], 1

.event_loop:
        mcall   23, CLOCK_RATE ; wait for event with CLOCK_RATE timeout 
        ;DEBUGF  DBG_INFO, "evenp loop iter i\n"

        cmp     eax, 1
        je      .event_redraw

        cmp     eax, 2
        je      .event_key

        cmp     eax, 3
        je      .event_button

        jmp     .event_default

        .event_redraw:
                stdcall draw_main_window
                jmp     .event_default

        .event_key:
                mcall   2
                stdcall keyboard_update, eax
                DEBUGF  DBG_INFO, "key scancode %x\n", eax:4
                jmp     .event_default

        .event_button:
                mcall   17
                cmp     ah, 1
                jne     .event_default
                mcall   -1

        .event_default:
                stdcall chip8_emulatecycle
                cmp     byte [chip8_draw_flag], 0
                jz      @f   

                ; mov     byte [gfx + 64*2 + 3], 1 ; DBG

                stdcall draw_screen
                mov     byte [chip8_draw_flag], 0
        @@:
                stdcall chip8_tick
        jmp     .event_loop

.file_not_found:
        DEBUGF  DBG_ERR, "Unable to open game file! eax = %u\n", eax
        jmp     .exit

.exit:
        mov     eax, -1
        int     0x40

;;;;;;;;;;;;;;;;;;;;;;;


include 'gui.inc'

include 'emu.inc'

include 'utils.inc'

include 'data.inc'
        rb      4096 ; reserve for main thread stack
align 16
_stacktop:

_mem:
