;=========================================
; initialized data
        include_debug_strings ; for debug-fdo

        chip8_fontset db  \
            0xF0, 0x90, 0x90, 0x90, 0xF0, \  ; 0
            0x20, 0x60, 0x20, 0x20, 0x70, \  ; 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, \  ; 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, \  ; 3
            0x90, 0x90, 0xF0, 0x10, 0x10, \  ; 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, \  ; 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, \  ; 6
            0xF0, 0x10, 0x20, 0x40, 0x40, \  ; 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, \  ; 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, \  ; 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, \  ; A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, \  ; B
            0xF0, 0x80, 0x80, 0x80, 0xF0, \  ; C
            0xE0, 0x90, 0x90, 0x90, 0xE0, \  ; D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, \  ; E
            0xF0, 0x80, 0xF0, 0x80, 0x80     ; F 

        opcode  dw 0          ; operation code
        V       db 16 dup(0)  ; 16 8-bit registers
        I       dw 0          ; additional register (usually used for storing addresses)
        P_C     dw 0          ; program counter
        S_P     dw 0          ; stack pointer
        delay_timer db 0
        sound_timer db 0
        stackmem    dw STACK_SIZE dup(0)  ; stack memory
        key         db KEY_SIZE dup (0) ; keyboard
        chip8_draw_flag db 0
        next_rand       dd 1

        align 4
        fread_struct:
            .subfunction    dd 0               ; + 0
            .offset_low     dd 0               ; + 4
            .offset_high    dd 0               ; + 8
            .size           dd MAX_GAME_SIZE   ; + 12
            .buffer         dd memory + 0x200  ; + 16
                            db 0               ; + 20
            .filename:      dd 0               ; + 24

        sys_colors system_colors
        main_window_title db 'CHIP-8 Emulator',0

        imgbuf_ptr  dd ? ; pointer to in-memory image buffer

;=========================================
align 16
_i_end:
; uninitialized data
        cmdline rb 1024 ; reserve for command line arguments

        memory  rb MEM_SIZE
        gfx     rb GFX_SIZE

        ;tmp_buf rb 128
