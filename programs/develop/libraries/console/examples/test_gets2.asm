
include 'proc32.inc'

DLL_ENTRY equ 1
DLL_EXIT  equ -1
REQ_DLL_VER equ 4

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

; yes! Now do some work (gets2() demo in this case).

        push    caption
        push    25
        push    80
        push    25
        push    80
        call    [con_init]

; C-equivalent of the following code:
; for (;;)
; {
;   con_write_asciiz("Enter string (empty for exit): ");
;   con_gets2(mycallback,s,256);
;   if (s[0] == '\n') break;
;   con_write_asciiz("You entered: ");
;   con_write_asciiz(s);
; }
mainloop:
        push    str1
        call    [con_write_asciiz]
        push    256
        push    s
        push    mycallback
        call    [con_gets2]
        cmp     [s], 10
        jz      done
        push    str2
        call    [con_write_asciiz]
        push    s
        call    [con_write_asciiz]
        jmp     mainloop
done:
        push    1
        call    [con_exit]
exit:
        or      eax, -1
        int     0x40

proc mycallback stdcall, keycode:dword, pstr:dword, pn:dword, ppos:dword
        mov     eax, [keycode]
        cmp     al, 0x0F
        jz      .tab
        cmp     al, 0x3B
        jz      .f1
        cmp     al, 0x48
        jz      .up
        cmp     al, 0x50
        jz      .down
        xor     eax, eax
        ret
.tab:
; Tab pressed - insert "[autocomplete]" to current position
        push    esi edi
        mov     eax, [ppos]
        mov     eax, [eax]
        mov     ecx, [pn]
        mov     ecx, [ecx]
        mov     esi, [pstr]
        mov     esi, [esi]
        add     ecx, esi
        add     esi, eax
        mov     edx, esi
@@:
        lodsb
        test    al, al
        jnz     @b
        lea     edi, [esi+str3.len]
        cmp     edi, ecx
        jbe     @f
        mov     edi, ecx
        lea     esi, [edi-str3.len]
@@:
        cmp     esi, edx
        jbe     @f
        dec     esi
        dec     edi
        mov     al, [esi]
        mov     [edi], al
        jmp     @b
@@:
        cmp     edi, ecx
        jb      @f
        dec     edi
@@:
        mov     ecx, edi
        sub     ecx, edx
        mov     edi, edx
        mov     esi, str3
        rep     movsb
        mov     eax, [pstr]
        sub     edi, [eax]
        mov     eax, [ppos]
        mov     [eax], edi
        pop     edi esi
        xor     eax, eax
        inc     eax
        ret
.f1:
; F1 pressed - say message
        push    str4
        call    [con_write_asciiz]
        push    str1
        call    [con_write_asciiz]
        push    2
        pop     eax
        ret
.up:
        push    esi
        mov     esi, str5
        mov     ecx, str5.len
        jmp     @f
.down:
        push    esi
        mov     esi, str6
        mov     ecx, str6.len
@@:
        push    edi
        mov     edi, [pstr]
        mov     edi, [edi]
        mov     eax, [ppos]
        mov     [eax], ecx
        rep     movsb
        xor     eax, eax
        stosb
        pop     edi esi
        inc     eax
        ret
endp

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
con_write_asciiz   dd szcon_write_asciiz
con_exit           dd szcon_exit
con_gets2          dd szcon_gets2
                   dd 0

szStart            db 'START',0
szVersion          db 'version',0
szcon_init         db 'con_init',0
szcon_write_asciiz db 'con_write_asciiz',0
szcon_exit         db 'con_exit',0
szcon_gets2        db 'con_gets2',0

dllname  db '/sys/lib/console.obj',0

caption            db 'Console test - gets2()',0
str1               db 'Enter string (empty for exit): ',0
str2               db 'You entered: ',0
str3               db '[autocomplete]'
str3.len = $ - str3
str4               db 13,10,'Help? What help do you need?',13,10,0
str5               db 'previous line in the history'
str5.len = $ - str5
str6               db 'next line in the history'
str6.len = $ - str6

i_end:

s rb 256

align 4
rb 2048 ; stack
mem:
