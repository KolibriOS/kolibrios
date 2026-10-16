format binary as ""
use32
    org 0x0

    db 'MENUET01'
    dd 0x01, START, I_END
    dd P_END ; Memory
    dd P_END ; Stack pointer
    dd 0x0
    dd 0x0

include '../../../../proc32.inc'
include '../../../../macros.inc'
include '../../../../dll.inc'

START:
        stdcall dll.Load, import_table
        test    eax, eax
        jnz     EXIT         ; If not 0, jump to exit

        invoke  con_init, -1, -1, -1, -1, window_title
        invoke  con_write_asciiz, greet_text

;------------ testing the [test_combo]      --------------
        invoke  count_utf8_codepoints, test_combo
        cinvoke con_printf, fmt_codepoints, eax

        invoke  count_utf8_graphemes, test_combo
        cinvoke con_printf, fmt_graphemes, eax
;------------ testing ends for [test_combo] --------------

;------------ testing the [unitxt]          --------------
        invoke  count_utf8_codepoints, unitxt
        cinvoke con_printf, fmt_codepoints, eax

        invoke  count_utf8_graphemes, unitxt
        cinvoke con_printf, fmt_graphemes, eax
;------------ testing ends for [unitxt]     --------------

;------------ testing the [test_tech]       --------------
        invoke  count_utf8_codepoints, test_tech
        cinvoke con_printf, fmt_codepoints, eax

        invoke  count_utf8_graphemes, test_tech
        cinvoke con_printf, fmt_graphemes, eax
;------------ testing ends for [test_tech]  --------------

        invoke  con_exit, 0
EXIT:
        mcall   -1              ; Exit cleanly

; DATA SECTION

window_title    db 'Debug Console', 0
greet_text      db 'Console loaded successfully!', 10, 0
fmt_codepoints  db "Total Codepoints: %d", 10, 0
fmt_graphemes   db "Total  Graphemes: %d", 10, 0
unitxt          db 'AП👨‍👩‍👦qwerty', 0
test_tech       db 'c', 'a', 'f', 'e', 0xCC, 0x81, 0
test_combo      db 'A', 0xD0, 0x9F, 0xF0, 0x9F, 0x91, 0xA9, 0xE2, 0x80, 0x8D, 0xF0, 0x9F, 0x92, 0xBB, 'e', 0xCC, 0x81, 0

align 4
import_table:
library \
        console, 'console.obj', \
        unicode, '/cd1/1/libunicode.obj'    ; TODO: Change the destination to its default.

import  console,\
        con_init, 'con_init', con_write_asciiz, 'con_write_asciiz', \
        con_exit, 'con_exit', con_printf, 'con_printf'

import  unicode, \
        count_utf8_codepoints, 'utf8.count_codepoints', \
        count_utf8_graphemes, 'utf8.count_graphemes'
I_END:

mem     rb 0x1000   ; 4096 bytes

P_END:

