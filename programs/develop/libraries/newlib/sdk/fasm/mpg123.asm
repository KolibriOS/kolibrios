
STACK_SIZE  equ 4096

include "include/app.inc"

align 8
main:
.argc   equ     ebp+8
.argv   equ     ebp+12
.envp   equ     ebp+16

     ;   int3
        push    ebp
        mov     ebp, esp
        push    ebx

        mov     eax, [.argc]
        cmp     eax, 2
        jae     @F

        call    _get_moviefile
        mov     [input_file], eax

@@:
        call    [mpg123_init]
        test    eax, eax
        jz      @F

        push    eax
        call    [mpg123_plain_strerror]

        mov     [esp], eax
        push    msg_init_fail
        call    [_printf]
        add     esp, 4
        jmp     .fail
@@:
        push    dword error
        push    0
        call    [mpg123_new]
        add     esp, 8

        mov     [mh], eax
        mov     ebx, eax
        test    eax, eax
        jz      .err_1

        push    [input_file]
        push    eax
        call    [mpg123_open]
        add     esp, 8

        test    eax, eax
        jnz     .err_1

        push    encoding
        push    channels
        push    rate
        push    ebx
        call    [mpg123_getformat]
        add     esp, 16

        test    eax, eax
        jnz     .err_1

        push    ebx
        call    [mpg123_scan]
        test    eax, eax
        jz      @F

        call    [mpg123_strerror]
        mov     [esp], eax
        push    msg_print
        call    [_printf]
        add     esp, 8
        jmp     .fail

@@:
        call    [mpg123_format_none]
        mov     ecx, [encoding]
        mov     [esp], ecx
        push    [channels]
        push    [rate]
        push    ebx
        call    [mpg123_format]
        add     esp, 16

        push    error
        stdcall InitSound
        test    eax, eax
        jz      @F

        cinvoke _printf, msg_sound
        jmp     .fail
@@:
        mov     eax, [rate]
        mov     ecx, [channels]

        mov     [whdr.riff_id], 0x46464952
        mov     [whdr.riff_format], 0x45564157
        mov     [whdr.wFormatTag], 0x01
        mov     [whdr.nSamplesPerSec], eax
        mov     [whdr.nChannels], cx
        mov     [whdr.wBitsPerSample], 16

        stdcall test_wav, whdr
        stdcall CreateBuffer, eax, 0, hBuff
        test    eax, eax
        jz      @F

        cinvoke _printf, msg_buffer
        jmp     .fail
@@:
        mov     eax, [hBuff]
        push    esi
        push    edi

        mov     ecx, 0x40000
        mov     eax, 68
        mov     ebx, 12
        int     0x40

        push    eax                  ;buffer esp+16
        push    count                ;&count esp+12
        push    0x40000              ;remain esp+8
        push    eax                  ;outPtr esp+4
        push    [mh]                 ;mh     esp

        xor     ebx, ebx             ;totalcount

.inner:
    ;    int3
        mov     [count], 0
        call    [mpg123_read]
        mov     ebx, [count]
        test    eax, eax
        jz      @F
        test    ebx, ebx
        jz      .done
@@:
.write_out:
        add     ebx, 4095
        and     ebx, -4096
        mov     esi, [esp+16]
        stdcall WaveOut, [hBuff], esi, ebx
        mov     [esp+8], dword 0x40000
        mov     edi, [esp+16]
        mov     [esp+4], edi
        jmp     .inner
.done:
        mov     edi, [esp+16]
        mov     ecx, 4096
        xor     eax, eax
        rep     stosd
        mov     esi, [esp+16]
        stdcall WaveOut, [hBuff], esi, 16384
        add     esp, 20
        pop     edi
        pop     esi

        xor     eax, eax
        pop     ebx
        mov     esp, ebp
        pop     ebp
        ret

.err_1:
        test    ebx, ebx
        jnz     @F

        push    [error]
        call    [mpg123_plain_strerror]
        jmp     .err_2
@@:
        push    ebx
        call    [mpg123_strerror]
.err_2:
        mov     [esp], eax
        push    msg_trouble
        call    [_printf]
        add     esp, 8
.fail:
        mov     eax, -1
        pop     ebx
        mov     esp, ebp
        pop     ebp
        ret

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


SRV_GETVERSION      equ 0
SND_CREATE_BUFF     equ 1
SND_DESTROY_BUFF    equ 2
SND_SETFORMAT       equ 3
SND_GETFORMAT       equ 4
SND_RESET           equ 5
SND_SETPOS          equ 6
SND_GETPOS          equ 7
SND_SETBUFF         equ 8
SND_OUT             equ 9
SND_PLAY            equ 10
SND_STOP            equ 11
SND_SETVOLUME       equ 12
SND_GETVOLUME       equ 13
SND_SETPAN          equ 14
SND_GETPAN          equ 15
SND_GETBUFFSIZE     equ 16
SND_GETFREESPACE    equ 17
SND_SETTIMEBASE     equ 18
SND_GETTIMESTAMP    equ 19


align 4
InitSound:       ;p_ver:dword

           push ebx
           push ecx

           mov eax, 68
           mov ebx, 16
           mov ecx, szInfinity
           int 0x40
           mov [hSound], eax
           test eax, eax
           jz .fail

           mov eax, 68
           mov ebx, 16
           mov ecx, szSound
           int 0x40
           mov [hrdwSound], eax

           lea eax, [esp+12]   ;p_ver
           xor ebx, ebx

           push 4              ;.out_size
           push eax            ;.output
           push ebx            ;.inp_size
           push ebx            ;.input
           push SRV_GETVERSION ;.code
           push [hSound]       ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp        ;[handle]
           int 0x40
           add esp, 24
           pop ecx
           pop ebx
           ret 4
.fail:
           or eax, -1
           pop ecx
           pop ebx
           ret 4


align 4
CreateBuffer:   ;format:dword,size:dword,p_str:dword

           push ebx
           push ecx
           lea eax, [esp+20]   ;p_str
           lea ebx, [esp+12]   ;format

           push 4              ;.out_size
           push eax            ;.output
           push 8              ;.inp_size
           push ebx            ;.input
           push SND_CREATE_BUFF;.code
           push [hSound]       ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp
           int 0x40
           add esp, 24       ;io_cintrol
           pop ecx
           pop ebx
           ret 12

align 4
proc test_wav stdcall, hdr:dword

           mov eax, [hdr]
           cmp dword [eax], 0x46464952
           jne .fail

           cmp dword [eax+8], 0x45564157
           jne .fail

           cmp word [eax+20], 1
           jne .fail

           mov ecx, dword [eax+24]
           mov edx, 22050
           cmp ecx, edx
           ja .high
           je .l_22

           cmp ecx, 8000
           je .l_8

           cmp ecx, 11025
           je .l_11

           cmp ecx, 12000
           je .l_12

           cmp ecx, 16000
           je .l_16
.fail:
           xor eax, eax
           ret
.high:
           cmp ecx, 24000
           je .LN56
           cmp ecx, 32000
           je .LN65
           cmp ecx, 44100
           je .LN74
           cmp ecx, 48000
           jne .fail

           movzx ecx, word [eax+22]
           dec ecx
           je .LN79
           dec ecx
           jne .LN74

           mov edx, 19
           jmp .done
.LN79:
           mov edx, 20
           jmp .done
.LN74:
           movzx ecx, word [eax+22]
           dec ecx
           je .LN70
           dec ecx
           jne .LN65

           mov edx, 21
           jmp .done
.LN70:
           mov edx, 22
           jmp .done
.LN65:
           movzx ecx, word [eax+22]
           dec ecx
           je .LN61
           dec ecx
           jne .LN56

           mov edx, 23
           jmp .done
.LN61:
           mov edx, 24
           jmp .done
.LN56:
           movzx ecx, word [eax+22]
           dec ecx
           je .LN52
           dec ecx
           je .LN50
.l_22:
           movzx ecx, word [eax+22]
           dec ecx
           je .LN43
           dec ecx
           je .LN41
.l_16:
           movzx ecx, word [eax+22]
           dec ecx
           je .LN34
           dec ecx
           je .LN32
.l_12:
           movzx ecx, word [eax+22]
           dec ecx
           je .LN25
           dec ecx
           je .LN23
.l_11:
           movzx ecx, word [eax+22]
           dec ecx
           je .LN16
           dec ecx
           je .LN14
.l_8:
           movzx ecx, word [eax+22]
           dec ecx
           je .LN7
           dec ecx
           jne .fail

           mov edx, 35
           jmp .done
.LN7:
           mov edx, 36
           jmp .done
.LN14:
           mov edx, 33
           jmp .done
.LN16:
           mov edx, 34
           jmp .done
.LN23:
           mov edx, 31
           jmp .done
.LN25:
           mov edx, 32
           jmp .done
.LN32:
           mov edx, 29
           jmp .done
.LN34:
           mov edx, 30
           jmp .done
.LN41:
           mov edx, 27
           jmp .done
.LN43:
           mov edx, 28
           jmp .done
.LN50:
           mov edx, 25
           jmp .done
.LN52:
           mov edx, 26
.done:
           xor ecx, ecx
           cmp word [eax+34], 16
           setne cl
           dec ecx
           and ecx, -18
           add ecx, edx
           mov eax, ecx
           ret
endp

align 4
WaveOut:        ;str:dword, src:dword, size:dword
           push ebx
           push ecx

           xor eax, eax
           lea ebx, [esp+12]   ;[stream]

           push eax            ;.out_size
           push eax            ;.output
           push 12             ;.inp_size
           push ebx            ;.input
           push SND_OUT        ;.code
           push dword [hSound] ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp
           int 0x40
           add esp, 24
           pop ecx
           pop ebx
           ret 12

align 4
hSound      dd ?
hrdwSound   dd ?

szInfinity  db 'INFINITY',0
szSound     db 'SOUND',0

align 4
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
    db 'mp3',0
;    db 'flv',0
;    db 'mov',0
;    db 'mpg',0
;    db 'mpeg',0
;    db 'mkv',0
;    db 'mp4',0
;    db 'webm',0
;    db 'wmv',0
.end:
    db 0




sz_proc_lib         db "/rd/1/lib/proc_lib.obj",0
sz_OpenDialog_init  db "OpenDialog_init",0
sz_OpenDialog_start db "OpenDialog_start",0
sz_com_area_name    db "FFFFFFFF_open_dialog",0
sz_dir_default_path db "/rd/1",0
sz_start_path       db "/rd/1/File managers/opendial",0

msg_print       db '%s',0x0D,0x0A,0
msg_init_fail   db 'Cannot initialize mpg123 library: %s', 0x0D,0x0A,0
msg_trouble     db 'Trouble with mpg123: %s', 0x0D,0x0A,0
msg_sound       db 'Sound service not installed', 0x0D,0x0A,0
msg_buffer      db 'Unable to create a sound buffer',0x0D,0x0A,0

align 16
__idata_start:

  library libc,'libc.dll',      \
          libmpg123, 'libmpg123.dll'

include 'include/libc.inc'
include 'include/libmpg123.inc'

__idata_end:
__iend:

align 4
whdr:
    .riff_id            rd 1
    .riff_size          rd 1
    .riff_format        rd 1

    .fmt_id             rd 1
    .fmt_size           rd 1

    .wFormatTag         rw 1
    .nChannels          rw 1
    .nSamplesPerSec     rd 1
    .nAvgBytesPerSec    rd 1
    .nBlockAlign        rw 1
    .wBitsPerSample     rw 1
    .data_id            rd 1
    .data_size          rd 1

proclib          rd 1
opendialog_init  rd 1
opendialog_start rd 1

input_file rd 1

mh         rd 1
encoding   rd 1
channels   rd 1
rate       rd 1

hBuff      rd 1
count      rd 1

done       rd 1

error      rd 1


__cmdline: rb 256
__pgmname: rb 1024
           rb 16
__stack:
__bssend:


