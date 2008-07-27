use32
        db      'MENUET01'
        dd      1
        dd      start
        dd      i_end
        dd      mem
        dd      mem
        dd      0
        dd      0

REQ_DLL_VER = 2
DLL_ENTRY = 1

start:
; First 3 steps are intended to load/init console DLL
; and are identical for all console programs

; load DLL
        mov     eax, 68
        mov     ebx, 19
        mov     ecx, dll_name
        int     0x40
        test    eax, eax
        jz      exit

; initialize import
        mov	edx, eax
        mov     esi, myimport
import_loop:
        lodsd
        test    eax, eax
        jz      import_done
        push    edx
import_find:
        mov     ebx, [edx]
        test    ebx, ebx
        jz      exit;import_not_found
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
        pop     edx
        jmp     import_loop
import_done:

; check version
        cmp     word [dll_ver], REQ_DLL_VER
        jb      exit
        cmp     word [dll_ver+2], REQ_DLL_VER
        ja      exit
        push    DLL_ENTRY
        call    [dll_start]

; yes! Now do some work (say helloworld in this case).
        push    caption
        push    -1
        push    -1
        push    -1
        push    -1
        call    [con_init]
        push    aHelloWorld
        call    [con_write_asciiz]
        push    0
        call    [con_exit]
exit:
        or      eax, -1
        int     0x40

dll_name db '/sys/lib/console.obj',0
caption db 'Console test',0
aHelloWorld     db      'Hello, World!',10,0

align 4
myimport:
dll_start       dd      aStart
dll_ver         dd      aVersion
con_init        dd      aConInit
con_write_asciiz dd     aConWriteAsciiz
con_exit        dd      aConExit
                dd      0

aStart          db      'START',0
aVersion        db      'version',0
aConInit        db      'con_init',0
aConWriteAsciiz db      'con_write_asciiz',0
aConExit        db      'con_exit',0

i_end:

align 4
rb 2048 ; stack
mem:
