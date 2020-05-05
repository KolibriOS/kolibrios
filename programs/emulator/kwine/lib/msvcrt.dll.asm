; ------------------------------------------------------------- ;
; KWINE is a fork of program PELoad written by 0CodErr
; author of fork - rgimad
; ------------------------------------------------------------- ;
GLOBAL EXPORTS
section '.exprt' align 16
;**********************************************************************************
EXPORTS: ;/////////////////////////////////////////////////////////////////////////
;**********************************************************************************
dd sz__getch,     _getch
dd sz__kbhit,     _kbhit
dd sz_printf,     printf
dd sz_puts,       puts
dd sz_gets,       gets
dd sz_strlen,     strlen
dd 0
sz__getch         db "_getch",0
sz__kbhit         db "_kbhit",0
sz_printf         db "printf",0
sz_puts           db "puts",0
sz_gets           db "gets",0
sz_strlen         db "strlen",0

section '.code' align 16
align 16
;**********************************************************************************
_getch: ;//////////////////////////////////////////////////////////////////////////
;**********************************************************************************
        push   ebx
        push   esi
        push   edi

        call   load_console_lib
        pop ecx
        call   [con_getch]
        push ecx

        pop    edi
        pop    esi
        pop    ebx
        ret
align 16				
;**********************************************************************************
_kbhit: ;//////////////////////////////////////////////////////////////////////////
;**********************************************************************************
        push   ebx
        push   esi
        push   edi

        call   load_console_lib
        pop ecx
        call   [con_kbhit]
        push ecx

        pop    edi
        pop    esi
        pop    ebx
        ret
align 16
;**********************************************************************************
printf: ;//////////////////////////////////////////////////////////////////////////
;**********************************************************************************
        ;pushad
        ;push   ebx
        ;push   esi
        ;push   edi
        call   load_console_lib
	    ;popad
        pop ecx
        call   [con_printf]
        push ecx

        ;pop    edi
        ;pop    esi
        ;pop    ebx
				
        ret
align 16
;**********************************************************************************
puts: ;////////////////////////////////////////////////////////////////////////// cdecl
;**********************************************************************************
        ;push   ebx
        ;push   esi
        ;push   edi

        call   load_console_lib
        pop ecx ; pop return address
        call   [con_write_asciiz]
        push ecx ; push return address again

        ;pop    edi
        ;pop    esi
        ;pop    ebx
        ret
align 16
;**********************************************************************************
gets: ;////////////////////////////////////////////////////////////////////////// cdecl
;**********************************************************************************
        ;push   ebx
        ;push   esi
        ;push   edi

        call   load_console_lib
        pop ecx
        pop edx
        push   25 ;; second arg of con_gets assume by default
        push edx
        call   [con_gets]
        push ecx

        ;pop    edi
        ;pop    esi
        ;pop    ebx
        ret
align 16
;**********************************************************************************
strlen: ;//////////////////////////////////////////////////////////////////////////
;**********************************************************************************
        push   ebx
        push   esi
        push   edi

        call   load_console_lib
        xor eax,eax
        mov edi, dword [esp + 4 + 4 + 4 + 4]
        .while1:
            inc eax
            cmp byte [eax + edi], 0
            jnz .while1

        pop    edi
        pop    esi
        pop    ebx
        ret

				
load_console_lib: ;; stdcall ?
; if already loaded then do nothing
        cmp    [console], dword 0
        jne    .do_nothing
        push   sz_console
        call   load.library
        mov    [console], eax
        mov    ecx, eax
        mov    ebx, getprocaddress
        ;;
        push   ecx
        push   sz_con_init
        call   ebx
        mov    [con_init], eax
        ;;
        push   ecx
        push   sz_con_getch
        call   ebx
        mov    [con_getch], eax
        ;;
        push   ecx
        push   sz_con_kbhit
        call   ebx
        mov    [con_kbhit], eax
        ;;
        push   ecx
        push   sz_con_printf
        call   ebx
        mov    [con_printf], eax
        ;;
        push ecx
        push sz_con_write_asciiz
        call ebx
        mov [con_write_asciiz], eax
        ;;
        push   ecx
        push   sz_con_gets
        call   ebx
        mov    [con_gets], eax
				
        push   ecx
        push   sz_con_init
        call   ebx
        mov    [con_init], eax
        push   con_caption
        push   -1
        push   -1
        push   -1
        push   -1
        call   [con_init]
				
				
.do_nothing:
        ret

; ------------------------------------------------------------- ; stdcall
load.library:
        mov    eax, 68
        mov    ebx, 19
        mov    ecx, [esp + 4]
        int    64
        ret    4
; ------------------------------------------------------------- ;
getprocaddress:
        mov    edx, [esp + 8]
        xor    eax, eax
        test   edx, edx
        jz     .end
.next:
        xor    eax, eax
        cmp    [edx], dword 0
        jz     .end
        mov    esi, [edx]
        mov    edi, [esp + 4]
.next_:
        lodsb
        scasb
        jne    .fail
        or     al, al
        jnz    .next_
        jmp    .ok
.fail:
        add    edx, 8
        jmp    .next
.ok:
        mov    eax, [edx + 4]
.end:
        ret    8
; ------------------------------------------------------------- ;

section '.data' align 16

con_caption         db "test!",0
con_init            dd 0
con_getch           dd 0
con_kbhit           dd 0
con_printf          dd 0
con_write_asciiz dd 0
con_gets            dd 0
console             dd 0
sz_con_init         db "con_init",0
sz_con_getch        db "con_getch",0
sz_con_kbhit        db "con_kbhit",0
sz_con_printf       db "con_printf",0
sz_con_write_asciiz db "con_write_asciiz",0
sz_con_gets         db "con_gets",0
sz_console          db "/sys/lib/console.obj",0