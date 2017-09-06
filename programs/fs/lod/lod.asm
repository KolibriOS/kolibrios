; Программа запускает OpenDialog с указанным фильтром и после выбора файла
; запускает указанную программу, передавая в качестве параметра выбранный путь.
; Например:
; LOD *pdf,xps*/hd0/1/mupdf
; LOD *mp3*/hd0/1/minimp3
; LOD *asm,inc,mac*/sys/tinypad

; Author 0CodErr
; http://board.kolibrios.org/viewtopic.php?f=9&t=2486

ORG 0
BITS 32
; ---------------------------------------------------------------------------- ;
PARAMS_SIZE        equ 256
STACK_SIZE         equ 256
; ---------------------------------------------------------------------------- ;
PROCINFO_SIZE      equ 1024
FILENAME_AREA_SIZE equ 256
OPENDIR_PATH_SIZE  equ 4096
OPENFILE_PATH_SIZE equ 4096
FILTER_AREA_SIZE   equ 256
FILTER_BRACKET     equ "*" ; and for example: LOD *bmp,png,jpeg*/sys/media/kiv
; ---------------------------------------------------------------------------- ;
MENUET01           db 'MENUET01'
version            dd 1
program.start      dd START
program.end        dd END
program.memory     dd END + PARAMS_SIZE + STACK_SIZE
program.stack      dd END + PARAMS_SIZE + STACK_SIZE
program.params     dd END
program.path       dd 0
; ---------------------------------------------------------------------------- ;
START:
		mov    edi,[program.params]
		cmp    [edi],dword 0
		je     terminate
		
        call   FakeDrawWindow
        call   OpenDialogInit
        call   OpenDialogSetFilter
        call   OpenDialogOpen
        cmp    [od.status], dword 0
        je     terminate
        mov    [file_info.params], eax
launch_program:
        mov    eax, 70
        mov    ebx, file_info
        int    64
terminate:
        or     eax, -1
        int    64
; ---------------------------------------------------------------------------- ;
OpenDialogInit:
; load.library
        mov    eax, 68
        mov    ebx, 19
        mov    ecx, sz_proc_lib
        int    64
        mov    [proclib], eax

        push   dword[proclib]
        push   sz_OpenDialog_init
        call   GetProcAddress
        mov    [opendialog_init], eax

        push   dword[proclib]
        push   sz_OpenDialog_start
        call   GetProcAddress
        mov    [opendialog_start], eax
; memory.allocate
        mov    eax, 68
        mov    ebx, 12
        mov    ecx, PROCINFO_SIZE + FILENAME_AREA_SIZE + OPENDIR_PATH_SIZE + OPENFILE_PATH_SIZE
        int    64

        mov    [od.procinfo], eax
        add    eax, PROCINFO_SIZE
        mov    [od.filename_area], eax
        add    eax, FILENAME_AREA_SIZE
        mov    [od.opendir_path], eax
        add    eax, OPENDIR_PATH_SIZE
        mov    [od.openfile_path], eax

        push   od
        call   [opendialog_init]
        ret
; ---------------------------------------------------------------------------- ;
OpenDialogOpen:
        mov    eax, [od.openfile_path]
        mov    [eax], byte 0
        push   od
        call   [opendialog_start]
        mov    eax, [od.openfile_path]
        ret
; ---------------------------------------------------------------------------- ;
GetProcAddress:
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
; ---------------------------------------------------------------------------- ;
FakeDrawWindow:
; redraw.start
        mov    eax, 12
        mov    ebx, 1
        int    64
; get.screen.size
        mov    eax, 61
        mov    ebx, 1
        int    64
        shr    eax, 1
        and    eax, 0x7FFF7FFF
; draw.window        
        movzx  ecx, ax
        shl    ecx, 16
        shr    eax, 16
        movzx  ebx, ax
        shl    ebx, 16
        mov    edx, 0x01000000
        xor    eax, eax
        int    64                         
; redraw.finish
        mov    eax, 12
        mov    ebx, 2
        int    64
        ret
; ---------------------------------------------------------------------------- ;
OpenDialogSetFilter:
        mov    edi, [program.params]
        mov    esi, filefilter + 4
; skip spaces
        or     ecx, -1
        mov    al, " "
        repe scasb
        dec    edi
        cmp    [edi], byte FILTER_BRACKET
        xchg   esi, edi
        jne    .no_filter
        inc    esi
        mov    ecx, FILTER_AREA_SIZE
; copy filter string to filter area
; and replace commas with zeroes
.next:
        lodsb
        test   al, al
        jnz    .bracket?
        stosb
        jmp    .done
.bracket?:
        cmp    al, FILTER_BRACKET
        jne    .comma?
        xor    al, al
        stosb
        jmp    .done
.comma?:
        cmp    al, ","
        jne    .not_comma
        xor    al, al
.not_comma:
        stosb
        loop   .next
.done:
        sub    edi, filefilter
        mov    [filefilter], edi
.no_filter:
        mov    edi, esi
; skip spaces
        or     ecx, -1
        mov    al, " "
        repe scasb
        dec    edi
        mov    [file_info.file_path], edi
        ret    4
; ---------------------------------------------------------------------------- ;
LaunchProgram:
        mov    eax, [od.openfile_path]
        mov    [file_info.params], eax
        mov    eax, 70
        mov    ebx, file_info
        int    64
        ret
; ---------------------------------------------------------------------------- ;
file_info:
                    dd 7
                    dd 0
.params             dd 0
                    dd 0
                    dd 0
                    db 0
.file_path          dd 0
; ---------------------------------------------------------------------------- ;
filefilter          dd 0
                    resb FILTER_AREA_SIZE
.end                db 0
; ---------------------------------------------------------------------------- ;
od:
.mode               dd 0
.procinfo           dd 0
.com_area_name      dd sz_com_area_name
.com_area           dd 0
.opendir_path       dd 0
.dir_default_path   dd sz_dir_default_path
.start_path         dd sz_start_path
.draw_window        dd FakeDrawWindow
.status             dd 0
.openfile_path      dd 0
.filename_area      dd 0
.filter_area        dd filefilter
.x_size             dw 444
.x_start            dw 0
.y_size             dw 444
.y_start            dw 0
; ---------------------------------------------------------------------------- ;
sz_proc_lib         db "/sys/lib/proc_lib.obj",0
sz_OpenDialog_init  db "OpenDialog_init",0
sz_OpenDialog_start db "OpenDialog_start",0
sz_com_area_name    db "FFFFFFFF_open_dialog",0
sz_dir_default_path db "/sys",0
sz_start_path       db "/sys/File managers/opendial",0
; ---------------------------------------------------------------------------- ;
proclib             dd 0
opendialog_init     dd 0
opendialog_start    dd 0
; ---------------------------------------------------------------------------- ;
END: