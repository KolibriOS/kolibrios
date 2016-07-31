use32
    org 0x0
    db  'MENUET01'
    dd  0x01,start,i_end,e_end,e_end,0,this_file_name

include '../../../../proc32.inc'
include '../../../../macros.inc'
include '../../../../dll.inc'
;include '../../../../debug.inc'
include 'libcrash.inc'

BUFFER_SIZE     = 4096


start:
        pushfd
        pop     eax
        or      eax, 1 SHL 18   ; Alignment Check flag
        push    eax
        popfd

        mcall   68, 11

        stdcall dll.Load, @IMPORT
        or      eax, eax
        jnz     quit

still:
        mcall   10
        dec     eax
        jz      redraw
        dec     eax
        jz      key

button:
        mcall   17
        shr     eax, 8

        cmp     eax, 1
        je      quit

redraw:
        mcall   12, 1
        mcall   0, <0,900>, <0,160>, 0x34000000, 0x80000000, window_title


        mcall   4, < 0,  0>, 0xc0ffffff, message, , 0

        mov     [hid], 0
        mov     [text_pos_y], 10
        mov     [hash_name], hash_strings
  .next_hash:
        mov     dword[f70_buf + 4], 0
        mov     dword[msglen], 0
        mov     edi, hex
        xor     eax, eax
        mov     ecx, 1024/4
        rep     stosd
        invoke  crash.hash, [hid], data_buffer, update_data_buffer, ctx
        invoke  crash.bin2hex, ctx, hex, [hid]
        mov     ebx, [text_pos_y]
        mcall   4, , 0xc0ffffff, [hash_name],    , 0
        mov     ebx, [text_pos_y]
        add     ebx, 66 SHL 16
        mcall   4, , 0x40ffffff, hex, 128, 0
        add     [text_pos_y], 10
        add     [hash_name], 12 ; lenght of hash_strings item
        inc     [hid]
        cmp     [hid], LIBCRASH_SHA3_512
        jng     .next_hash

        mcall   12, 2
        jmp     still

key:
        mcall   2
        jmp     still


quit:
        mcall   -1


proc update_data_buffer _left
        mov     eax, data_buffer
        add     eax, [_left]
        mov     dword[f70_buf + 16], eax
        mov     eax, BUFFER_SIZE
        sub     eax, [_left]
        mov     dword[f70_buf + 12], eax
        mcall   70, f70_buf
        mov     eax, ebx
        cmp     eax, -1
        jne     @f
        inc     eax
    @@:
        add     dword[f70_buf + 4], eax
        ret
endp


szZ window_title        , 'libcrash example'
szZ message             , 'hash sums of this file'

hash_strings:
                db 'crc32    : ',0
                db 'md4      : ',0
                db 'md5      : ',0
                db 'sha1     : ',0
                db 'sha224   : ',0
                db 'sha256   : ',0
                db 'sha384   : ',0
                db 'sha512   : ',0
                db 'sha3-224 : ',0
                db 'sha3-256 : ',0
                db 'sha3-384 : ',0
                db 'sha3-512 : ',0

f70_buf:
        funcnum dd 0
        src     dd 0
        res1    dd 0
        count   dd BUFFER_SIZE
        dst     dd data_buffer
        res2    db 0
        fname   dd this_file_name


align 4
@IMPORT:

library                           \
        libcrash, 'libcrash.obj'

import  libcrash                        , \
        libcrash.init  , 'lib_init'     , \
        crash.hash     , 'crash_hash'   , \
        crash.bin2hex  , 'crash_bin2hex'

i_end:
hash_name       rd 1
text_pos_y      rd 1
hash            rd 50
hid             rd 1    ; hash id
msglen          rd 1
hex             rb 1024
data_buffer     rb BUFFER_SIZE
this_file_name  rb 4096
rb 0x800                                        ;stack
align 16        ; should be enough
ctx             rb 0x1000
e_end:

