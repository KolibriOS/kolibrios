
include 'proc32.inc'

DLL_ENTRY equ 1
DLL_EXIT  equ -1
REQ_DLL_VER equ 3

use32
        db      'MENUET01'
        dd      1
        dd      start
        dd      i_end
        dd      mem
        dd      mem
        dd      0
        dd      0

start:
        stdcall load_dll_and_import, dllname, imports
        test    eax, eax
        jz      exit

; check version
        cmp     word [dll_ver], REQ_DLL_VER
        jb      exit
        cmp     word [dll_ver+2], REQ_DLL_VER
        ja      exit
        push    DLL_ENTRY
        call    [dll_start]

; yes! Now do some work (getch() demo in this case).

        push    caption
        push    -1
        push    -1
        push    -1
        push    -1
        call    [con_init]

; C-equivalent of the following code:
; con_printf(start_string);
; int c;
; while ((c=con_getch())!=27) // Esc=exit
; {
;   if (c)
;     con_printf("normal character with code %d=0x%02X\n",c,c);
;   else
;   {
;     c=con_getch();
;     con_printf("extended character with code %d=0x%02X\n",c,c);
;   }
; }
        push    start_string
        call    [con_printf]
        pop     ecx
mainloop:
        call    [con_getch]
        cmp     al, 27
        jz      done
        test    eax, eax
        jz      extended
        push    eax
        push    eax
        push    string_normal
@@:
        call    [con_printf]
        add     esp, 12
        jmp     mainloop
extended:
        call    [con_getch]
        push    eax
        push    eax
        push    string_extended
        jmp     @b
done:
        push    1
        call    [con_exit]
exit:
        or      eax, -1
        int     0x40

proc load_dll_and_import stdcall, _dllname:dword, _imports:dword
        pushad
; load DLL
        push    68
        pop     eax
        push    19
        pop     ebx
        mov     ecx, [_dllname]
        int     0x40
        test    eax, eax
        jz      import_fail

; initialize import
        mov     edi, eax
        mov     esi, [_imports]
import_loop:
        lodsd
        test    eax, eax
        jz      import_done
        mov     edx, edi
import_find:
        mov     ebx, [edx]
        test    ebx, ebx
        jz      import_not_found
        push    eax
@@:
        mov     cl, [eax]
        cmp     cl, [ebx]
        jnz     import_find_next
        test    cl, cl
        jz      import_found
        inc     eax
        inc     ebx
        jmp     @b
import_find_next:
        pop     eax
        add     edx, 8
        jmp     import_find
import_found:
        pop     eax
        mov     eax, [edx+4]
        mov     [esi-4], eax
        jmp     import_loop
import_not_found:
import_fail:
        popad
        xor     eax, eax
        ret
import_done:
        popad
        xor     eax, eax
        inc     eax
        ret
endp

align 4

imports:
dll_start          dd szStart
dll_ver            dd szVersion
con_init           dd szcon_init
con_printf         dd szcon_printf
con_exit           dd szcon_exit
con_getch          dd szcon_getch
                   dd 0

szStart            db 'START',0
szVersion          db 'version',0
szcon_init         db 'con_init',0
szcon_printf       db 'con_printf',0
szcon_exit         db 'con_exit',0
szcon_getch        db 'con_getch',0

dllname  db '/sys/lib/console.obj',0

caption            db 'Console test - getch()',0
start_string       db 'Press any key to see its code, or Esc to exit',10,0
string_normal      db 'normal character with code %d=0x%02X',10,0
string_extended    db 'extended character with code %d=0x%02X',10,0

i_end:

align 4
rb 2048 ; stack
mem:
