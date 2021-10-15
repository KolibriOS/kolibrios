use32
    org 0x0
    db  'MENUET01'
    dd  0x01,start,i_end,e_end,e_end,0,this_file_name

include 'proc32.inc'
include 'macros.inc'
include 'dll.inc'
;include 'debug-fdo.inc'
include 'libcrash.inc'

BUFFER_SIZE     = 0x1000


start:
        pushfd
        pop     eax
;        or      eax, 1 SHL 18   ; Alignment Check flag, FIXME in libcrash
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

        mov     [f70_buf.src], 0
        invoke  crash.hash, LIBCRASH_SHA2_256, read_data, 0, bin
        stdcall bin2hex, bin, SHA2_256_LEN, hex
        mcall   4, <0,0>, 0xc0ffffff, hex, 0, 0

        invoke  crash.hash_oneshot, LIBCRASH_SHA2_256, ctx, 0, i_end
        stdcall bin2hex, ctx, SHA2_256_LEN, hex
        mcall   4, <0,10>, 0xc0ffffff, hex, 0, 0

        invoke  sha2_256.oneshot, ctx, 0, i_end
        stdcall bin2hex, ctx, SHA2_256_LEN, hex
        mcall   4, <0,20>, 0xc0ffffff, hex, 0, 0

        invoke  sha2_256.init, ctx
        invoke  sha2_256.update, ctx, 0, 42
        invoke  sha2_256.update, ctx, 42, i_end-42
        invoke  sha2_256.finish, ctx
        stdcall bin2hex, ctx, SHA2_256_LEN, hex
        mcall   4, <0,30>, 0xc0ffffff, hex, 0, 0

        mcall   12, 2
        jmp     still

key:
        mcall   2
        jmp     still


quit:
        mcall   -1


proc bin2hex uses esi edi, _bin, _len, _hex
        mov     esi, [_bin]
        mov     edi, [_hex]
        mov     ecx, [_len]
.next_byte:
        movzx   eax, byte[esi]
        shr     al, 4
        cmp     al, 10
        sbb     al, 0x69
        das
        stosb
        lodsb
        and     al, 0x0f
        cmp     al, 10
        sbb     al, 0x69
        das
        stosb
        loop    .next_byte
        mov     byte[edi], 0
        ret
endp


proc read_data uses ebx, _user, _buf, _len
        mov     eax, [_buf]
        mov     [f70_buf.dst], eax
        mov     eax, [_len]
        mov     [f70_buf.count], eax
        mcall   70, f70_buf
        mov     eax, ebx
        cmp     eax, -1
        jnz     @f
        inc     eax
    @@:
        add     [f70_buf.src], eax
        ret
endp


sz window_title,  'libcrash example',0

f70_buf:
        .funcnum dd 0
        .src     dd 0
                 dd 0
        .count   dd BUFFER_SIZE
        .dst     dd data_buffer
                 db 0
        .fname   dd this_file_name


align 4
@IMPORT:

library                           \
        libcrash, 'libcrash.obj'

import  libcrash, \
        libcrash.init,         'lib_init', \
        crash.hash,            'crash_hash', \
        crash.hash_oneshot,    'crash_hash_oneshot', \
        crash.mac,             'crash_mac', \
        crash.mac_oneshot,     'crash_mac_oneshot', \
        crash.crypt,           'crash_crypt', \
        crash.crypt_oneshot,   'crash_crypt_oneshot', \
        sha2_256.init,         'sha2_256_init', \
        sha2_256.update,       'sha2_256_update', \
        sha2_256.finish,       'sha2_256_finish', \
        sha2_256.oneshot,      'sha2_256_oneshot', \
        hmac_sha2_256.init,    'hmac_sha2_256_init', \
        hmac_sha2_256.update,  'hmac_sha2_256_update', \
        hmac_sha2_256.finish,  'hmac_sha2_256_finish', \
        hmac_sha2_256.oneshot, 'hmac_sha2_256_oneshot', \
        aes256ctr.init,        'aes256ctr_init', \
        aes256ctr.update,      'aes256ctr_update', \
        aes256ctr.finish,      'aes256ctr_finish', \
        aes256ctr.oneshot,     'aes256ctr_oneshot'

i_end:
bin             rb MAX_HASH_LEN
hex             rb MAX_HASH_LEN*2+1
data_buffer     rb BUFFER_SIZE
this_file_name  rb 0x1000
align LIBCRASH_ALIGN
ctx             rb LIBCRASH_CTX_LEN
rb 0x1000       ;stack
e_end:
