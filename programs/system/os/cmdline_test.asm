; Just a test of cmdline.inc.
; Checks that parsing of some predefined command lines
; gives some predefined command arguments.
; Nothing to see here.
format PE console 4.0
entry start

include 'win32a.inc'
include '../../struct.inc'
include '../../proc32.inc'

start:
        stdcall run_test, empty_cmdline, empty_args
        stdcall run_test, spaces_tabs_cmdline, empty_args
        stdcall run_test, fancy_quotes_cmdline, fancy_quotes_args
        stdcall run_test, fancy_slashes_cmdline, fancy_slashes_args
        stdcall run_test, unmatched_quote_cmdline, unmatched_quote_args
        xor     eax, eax
        ret

proc run_test
        mov     esi, [esp+4]
        xor     edi, edi
        xor     edx, edx
        call    parse_cmdline
        mov     eax, [esp+8]
        cmp     ebx, [eax]
        jnz     .invalid_argc
        test    edx, edx
        jnz     .invalid_edx
        mov     eax, [esp+4]
@@:
        inc     eax
        cmp     byte [eax-1], 0
        jnz     @b
        cmp     esi, eax
        jnz     .invalid_esi
        mov     esi, [esp+4]
        mov     edi, data_area
        mov     edx, argv_area
        call    parse_cmdline
        mov     eax, [esp+4]
@@:
        inc     eax
        cmp     byte [eax-1], 0
        jnz     @b
        cmp     esi, eax
        jnz     .invalid_esi
        mov     eax, [esp+8]
        cmp     ebx, [eax]
        jnz     .invalid_argc
        lea     ecx, [argv_area+ebx*4]
        cmp     edx, ecx
        jnz     .invalid_edx
        lea     esi, [eax+4]
        mov     edi, data_area
        mov     edx, argv_area
        test    ebx, ebx
        jz      .args_done
.args_check:
        cmp     [edx], edi
        jnz     .invalid_argv
        add     edx, 4
@@:
        cmpsb
        jnz     .invalid_argv
        cmp     byte [esi-1], 0
        jnz     @b
        dec     ebx
        jnz     .args_check
.args_done:
        ret     8
.invalid_argc:
        mov     eax, 1
        int3
        jmp     $
.invalid_edx:
        mov     eax, 2
        int3
        jmp     $
.invalid_esi:
        mov     eax, 3
        int3
        jmp     $
.invalid_argv:
        mov     eax, 4
        int3
        jmp     $
endp

include 'cmdline.inc'

empty_cmdline   db      0
spaces_tabs_cmdline     db      '  ',9,' ',9,0
empty_args      dd      0

fancy_quotes_cmdline    db      'begin"quoted mode"end\ \"escaped" "quotes" "1\" "" """escaped quotes 2"""',0
fancy_quotes_args       dd      4
        db      'beginquoted modeend\',0
        db      '"escaped quotes 1"',0
        db      0
        db      '"escaped quotes 2"',0

fancy_slashes_cmdline   db      'arg\\" "1\\x "arg 2\\x" arg3\" arg4\\\"',9,'"arg 5\"" "arg6\\\"" "arg 7\\"',0
fancy_slashes_args      dd      7
        db      'arg\ 1\\x',0
        db      'arg 2\\x',0
        db      'arg3"',0
        db      'arg4\"',0
        db      'arg 5"',0
        db      'arg6\"',0
        db      'arg 7\',0

unmatched_quote_cmdline db      'some string"test',0
unmatched_quote_args    dd      2
        db      'some',0
        db      'stringtest',0

align 4
data_area       rb      1024
argv_area       rd      256
