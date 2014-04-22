
use32

LIBC_VERSION     =  1
DLL_ENTRY        =  1

db 'MENUET02'
dd 1
dd start
dd i_end
dd mem
dd mem
dd cmdline
dd path
dd 0

align 4
start:
           mov eax, LIBC_VERSION
           mov ecx, sz_libc
           mov edx, libc
           call load_library
           test eax, eax
           jz .fail

           push dword 0                ; no environment
           push cmdline
           push path
           push (my_app_end - my_app)
           push my_app
           call [libc.imp_exec]

           ret
.fail:
           or eax, -1
           int 0x40

align 4
load_library:    ;eax=VERSION ecx=library path edx=import section

           sub esp, 16
           mov [esp+8], edx
           mov [esp+12], eax

           mov  eax, 68
           mov  ebx, 19
           int  0x40
           test eax, eax
           jz   .fail

           mov [esp+4], eax
           mov  esi, edx                ;import section
           mov  edx, eax                ;export section
.import_loop:
           lodsd
           test eax, eax
           jz   .import_done
.import_find:
           mov ebx, [edx]
           test ebx, ebx
           jz .fail                     ;import_not_found

           mov [esp], eax               ;import name
@@:
           mov cl, [eax]
           cmp cl, [ebx]
           jnz .import_find_next

           test cl, cl
           jz .import_found

           inc eax
           inc ebx
           jmp @b

.import_find_next:
           mov eax, [esp]
           add edx, 8
           jmp .import_find

.import_found:
           mov eax, [edx+4]
           mov [esi-4], eax
           mov edx, [esp+4]
           jmp .import_loop
.import_done:

           mov edx, [esp+8]
           mov eax, [esp+12]

           cmp word [edx+4], ax
           jb .fail
           cmp word [edx+6], ax
           ja .fail

           push DLL_ENTRY
           call dword [edx]
.fail:
           add esp, 16
           ret


sz_libc    db  '/sys/lib/libc.obj',0

szStart    db  'START',0
szVersion  db  'version',0
szExec     db  'exec',0

libc:

.imp_start      dd  szStart
.imp_ver        dd  szVersion
.imp_exec       dd  szExec
                dd  0

; keep this aligned
align 16
my_app:
           file 'myapp.exe'
my_app_end:


; keep this aligned
align 4
i_end:

path       rb 1024
cmdline    rb 256
           rb 128   ;required stack
; keep this aligned
align 4096
mem:

