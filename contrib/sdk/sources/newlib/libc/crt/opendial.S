format MS COFF

public _get_moviefile

section '.text' align 16

align 4
getprocaddress:
        mov     edx, [esp + 8] ; hlib
        xor     eax, eax
        test    edx, edx ; If hlib = 0 then goto .end
        jz      .end

.next:
        cmp     [edx], dword 0 ; If end of export table then goto .end
        jz      .end

        xor     eax, eax
        mov     esi, [edx]
        mov     edi, [esp + 4] ; name

.next_:
        lodsb
        scasb
        jne     .fail
        or      al, al
        jnz     .next_
        jmp     .ok
.fail:
        add     edx, 8
        jmp     .next

.ok:                  ; return address
        mov eax, [edx + 4]
.end:
        ret 8



align 8
_get_moviefile:

        pushad
        mov     eax, 68
        mov     ebx, 19
        mov     ecx, sz_proc_lib
        int     0x40
        mov     [proclib], eax
        test    eax, eax
        jz      .fail

        push    [proclib]
        push    sz_OpenDialog_init
        call    getprocaddress
        mov     [opendialog_init], eax

        push    dword[proclib]
        push    sz_OpenDialog_start
        call    getprocaddress
        mov     [opendialog_start], eax

        mov     eax, 68
        mov     ebx, 12
        mov     ecx, 4096*3
        int     0x40

        mov     [od.procinfo], eax

        add     eax, 1024
        mov     [od.filename_area], eax

        add     eax, 3072
        mov     [od.opendir_path], eax

        add     eax, 4096
        mov     [od.openfile_path], eax

        push    od
        call    [opendialog_init]

        mov     eax, [od.openfile_path]
        mov     [eax], byte 0          ; end of ASCIIZ-string(may be don't need?)

        push    od
        call    [opendialog_start]

        popad
        mov     eax, [od.openfile_path]; selected filePath

        ret
.fail:
        xor     eax, eax
        ret

align 4
fake_on_redraw:
        ret

section '.rdata' align 16

sz_proc_lib         db "/rd/1/lib/proc_lib.obj",0
sz_OpenDialog_init  db "OpenDialog_init",0
sz_OpenDialog_start db "OpenDialog_start",0
sz_com_area_name    db "FFFFFFFF_open_dialog",0
sz_dir_default_path db "/rd/1",0
sz_start_path       db "/rd/1/File managers/opendial",0


section '.data' align 16

od:
    .mode             dd 0
    .procinfo         dd 0
    .com_area_name    dd sz_com_area_name
    .com_area         dd 0
    .opendir_path     dd 0
    .dir_default_path dd sz_dir_default_path
    .start_path       dd sz_start_path
    .draw_window      dd fake_on_redraw
    .status           dd 0
    .openfile_path    dd 0
    .filename_area    dd 0
    .filter_area      dd filefilter
    .x_size           dw 512
    .x_start          dw 512
    .y_size           dw 512
    .y_start          dw 512

filefilter:
dd filefilter.end - filefilter
    db 'avi',0
    db 'flv',0
    db 'mov',0
    db 'mpg',0
    db 'mpeg',0
    db 'mkv',0
    db 'mp4',0
    db 'webm',0
    db 'wmv',0
.end:
    db 0


section '.bss' align 16

proclib          dd ?
opendialog_init  dd ?
opendialog_start dd ?

