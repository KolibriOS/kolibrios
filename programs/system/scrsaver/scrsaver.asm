use32
        org 0
        db  'MENUET01'
        dd  0x01,start,i_end,e_end,e_end,0,0

include 'proc32.inc'
include 'macros.inc'
include 'dll.inc'
include 'debug-fdo.inc'

__DEBUG__ = 1
__DEBUG_LEVEL__ = 1

DEFAULT_TIMEOUT_MINS = 15

start:
        mcall   68, 11
        mcall   40, EVM_KEY + EVM_BACKGROUND + EVM_MOUSE

        stdcall dll.Load,@IMPORT
        test    eax, eax
        jnz     exit

        invoke  ini.get_str, ini_file, ini_section, ini_key_program, ini_program_buf, ini_program_buf.size, 0
        cmp     [ini_program_buf], 0          ; if nothing set then exit 
        je      exit

        invoke  ini.get_int, ini_file, ini_section, ini_key_timeout, DEFAULT_TIMEOUT_MINS
        imul    eax, 60*100     ; cs
        mov     [timeout], eax

        ; r1647 by Nasarus
;        mcall   66, 4, 57, 0    ; hot key for {Space}
;        mcall   66, 4, 28, 0    ; hot key for {Enter}

still:
        mcall   23, [timeout]
        test    eax, eax
        jz      run_saver
        cmp     eax, 2  ; key
        jnz     still
        mcall
        ; r1647 by Nasarus
;        cmp     al, 2           ; hot key?
;        jnz     still           ; no hotkey, evenets handling go on
;        movzx   edx, ah
;        mcall   72, 1, 2        ; transfer key code to active window after interception
        jmp     still
run_saver:
        invoke  ini.get_str, ini_file, ini_section, ini_key_program, ini_program_buf, ini_program_buf.size, ini_program_default
        ; run actual screensaver
        mcall   70, f70
        cmp     eax, 0
        jg      exit
        neg     eax
        DEBUGF 1, 'Screen saver not found: %d: %s\n', eax, ini_program_buf
exit:
        mcall   -1


sz ini_file, '/sys/settings/system.ini',0
sz ini_section, 'screensaver',0
sz ini_key_timeout, 'timeout',0
timeout dd ?
sz ini_key_program, 'program',0
sz ini_program_default, '/sys/demos/spiral',0
sz program_params, '@ss',0

f70:    ; run
        dd 7, 0, program_params, 0, 0
        db 0
        dd ini_program_buf

align 4
@IMPORT:

library \
        libini , 'libini.obj'

import  libini, \
        ini.get_str, 'ini_get_str', \
        ini.get_int, 'ini_get_int'

include_debug_strings
i_end:

align 4
sz ini_program_buf, 1024 dup(?)
rb 0x100
e_end:
