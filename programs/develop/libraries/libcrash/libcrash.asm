; libcrash -- cryptographic hash (and other) functions
;
; Copyright (C) <2012-2014,2016,2019,2021> Ivan Baravy
;
; SPDX-License-Identifier: GPL-2.0-or-later
;
; This program is free software: you can redistribute it and/or modify it under
; the terms of the GNU General Public License as published by the Free Software
; Foundation, either version 2 of the License, or (at your option) any later
; version.
;
; This program is distributed in the hope that it will be useful, but WITHOUT
; ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
; FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License along with
; this program. If not, see <http://www.gnu.org/licenses/>.

format MS COFF

public @EXPORT as 'EXPORTS'

include 'proc32.inc'
include 'struct.inc'
include 'macros.inc'
include 'kglobals.inc'

purge section,mov,add,sub
section '.flat' code readable align 16

include 'libcrash.inc'
include 'hash/crc32.asm'
include 'hash/md5.asm'
include 'hash/sha1.asm'
include 'hash/sha2_224_256.asm'
include 'hash/sha2_384_512.asm'
include 'hash/sha3.asm'
include 'mac/poly1305.asm'
include 'mac/hmac.asm'
include 'cipher/chacha20.asm'
include 'cipher/mode/ctr.asm'
include 'cipher/mode/cbc.asm'
include 'cipher/aes.asm'
include 'cipher/aes_ctr.asm'
include 'cipher/aes_cbc.asm'

LIBCRASH_BUF_SIZE = 0x1000

struct hash_item
        init    dd ?
        update  dd ?
        finish  dd ?
        oneshot dd ?
        ctx_size dd ?
        out_size dd ?
ends

struct mac_item
        init    dd ?
        update  dd ?
        finish  dd ?
        oneshot dd ?
        ctx_size dd ?
        out_size dd ?
ends

struct cipher_item      ; FIXME merge *_item, why not
        init    dd ?
        update  dd ?
        finish  dd ?
        oneshot dd ?
        ctx_size dd ?
                 dd ?   ; placeholder for out_size
ends

; Initialize the library.
; This must be the first called function of the library.
; Parameters:
; eax = function pointer to allocate memory:
;       stdcall void *alloc(size_t size)
; ebx = function pointer to free memory:
;       stdcall void free(void *ptr)
; Return value: none
proc crash.init
        mov     [mem.alloc], eax
        mov     [mem.free], ebx
        ret
endp

; Hash data read by a callback read function.
; The function calls the read callback until it returns 0, and hashes the read
; data with the specified algorithm. The result is written to the specified
; buffer in binary format.
; Parameters:
; [_id] = ID of a hash function to use.
; [_clbk_read] = function pointer to read data:
;                stdcall ssize_t clbk_read(void *user, void *buf, size_t size)
; [_user] = pointer to user-specified data passed to the read callback as is.
; [_out] = buffer pointer where a hash value is to be written to.
; Return value:
; binary hash value in [_out] buffer.
proc crash.hash uses ebx esi edi, _id, _clbk_read, _user, _out
locals
        .ctx dd ?
        .buf dd ?
endl
        ; alloc buf
        mov     eax, [io_buf_size]
        add     eax, LIBCRASH_MAX_PAD_LEN
        stdcall [mem.alloc], eax
        test    eax, eax
        jz      .quit
        mov     [.buf], eax
        ; alloc ctx
        mov     eax, [_id]
        imul    eax, sizeof.hash_item
        lea     ebx, [crash._.hash_table+eax]
        stdcall [mem.alloc], [ebx+hash_item.ctx_size]
        test    eax, eax
        jz      .quit_free_buf
        mov     [.ctx], eax

        stdcall [ebx+hash_item.init], [.ctx]
.update:
        stdcall [_clbk_read], [_user], [.buf], [io_buf_size]
        test    eax, eax
        jz      .finish
        stdcall [ebx+hash_item.update], [.ctx], [.buf], eax
        jmp     .update
.finish:
        stdcall [ebx+hash_item.finish], [.ctx]
        mov     esi, [.ctx]
        mov     edi, [_out]
        mov     ecx, [ebx+hash_item.out_size]
        rep movsd
.quit_free_buf_ctx:
        stdcall [mem.free], [.ctx]
.quit_free_buf:
        stdcall [mem.free], [.buf]
.quit:
        ret
endp

; Calculate MAC of data read by a callback read function.
; The function calls the read callback until it returns 0, and calculates a MAC
; using a specified algorithm and a key. The result is written to the specified
; buffer.
; Parameters:
; [_id] = ID of a MAC function to use.
; [_key] = key pointer, no NULL terminator is needed
; [_key_len] = length of the [_key] data, in bytes
; [_clbk_read] = function pointer to read data:
;                stdcall ssize_t clbk_read(void *user, void *buf, size_t size)
; [_user] = pointer to user-specified data passed to the read callback as is.
; [_out] = buffer pointer where a MAC value is to be written to.
; Return value:
; Binary MAC value in [_out] buffer.
proc crash.mac uses ebx esi edi, _id, _key, _key_len, _clbk_read, _user, _out
locals
        .ctx dd ?
        .buf dd ?
endl
        ; alloc buf
        mov     eax, [io_buf_size]
        add     eax, LIBCRASH_MAX_PAD_LEN
        stdcall [mem.alloc], eax
        test    eax, eax
        jz      .quit
        mov     [.buf], eax
        ; alloc ctx
        mov     eax, [_id]
        imul    eax, sizeof.mac_item
        lea     ebx, [crash._.mac_table+eax]
        stdcall [mem.alloc], [ebx+mac_item.ctx_size]
        test    eax, eax
        jz      .quit_free_buf
        mov     [.ctx], eax

        stdcall [ebx+mac_item.init], [.ctx], [_key], [_key_len]
.update:
        stdcall [_clbk_read], [_user], [.buf], [io_buf_size]
        test    eax, eax
        jz      .finish
        stdcall [ebx+mac_item.update], [.ctx], [.buf], eax
        jmp     .update
.finish:
        stdcall [ebx+mac_item.finish], [.ctx]
        mov     esi, [.ctx]
        mov     edi, [_out]
        mov     ecx, [ebx+mac_item.out_size]
        rep movsd
.quit_free_buf_ctx:
        stdcall [mem.free], [.ctx]
.quit_free_buf:
        stdcall [mem.free], [.buf]
.quit:
        ret
endp

; Encrypt or decrypt data read by a callback read function.
; The function calls the read callback until it returns 0, and encrypts or
; decrypts the data using a specified algorithm, a key and an input vector.
; The result is passed to the write callback function.
; * The maximum difference in input/output data lengths is LIBCRASH_MAX_PAD_LEN.
; * The input and output buffers can sometimes be the same buffer depending on
; the cipher. If unsure, use different buffers.
; Parameters:
; [_id] = ID of a MAC function to use.
; [_flags] = see LIBCRASH_CIPHER_* in libcrash.inc
; [_key] = key pointer, NULL terminated
; [_iv] = input vector pointer, no NULL terminator is needed
; [_clbk_read] = function pointer to read data:
;                stdcall ssize_t clbk_read(void *user, void *buf, size_t size)
; [_user_read] = pointer to user-specified data passed to the read callback.
; [_clbk_write] = function pointer to write data:
;                 stdcall void clbk_write(void *user, void *buf, size_t size)
; [_user_write] = pointer to user-specified data passed to the write callback.
; Return value: none
proc crash.crypt uses ebx esi edi, _id, _flags, _key, _iv, _clbk_read, \
                                   _user_read, _clbk_write, _user_write
locals
        .ctx dd ?
        .buf dd ?
        .buf_in dd ?
        .buf_out dd ?
endl
        ; alloc buf
        mov     eax, [io_buf_size]
        mov     [.buf_out], eax
        shl     eax, 1
        add     eax, LIBCRASH_MAX_PAD_LEN
        stdcall [mem.alloc], eax
        test    eax, eax
        jz      .quit
        mov     [.buf], eax
        mov     [.buf_in], eax
        add     [.buf_out], eax
        ; alloc ctx
        mov     eax, [_id]
        imul    eax, sizeof.cipher_item
        lea     ebx, [crash._.cipher_table+eax]
        stdcall [mem.alloc], [ebx+cipher_item.ctx_size]
        test    eax, eax
        jz      .quit_free_buf
        mov     [.ctx], eax

        stdcall [ebx+cipher_item.init], [.ctx], [_key], [_iv], [_flags]
.update:
        stdcall [_clbk_read], [_user_read], [.buf_in], [io_buf_size]
        test    eax, eax
        jz      .finish
        stdcall [ebx+cipher_item.update], [.ctx], [.buf_in], eax, [.buf_out]
        stdcall [_clbk_write], [_user_write], [.buf_out], eax
        jmp     .update
.finish:
        stdcall [ebx+cipher_item.finish], [.ctx], [.buf_out]
        stdcall [_clbk_write], [_user_write], [.buf_out], eax
.quit_free_buf_ctx:
        stdcall [mem.free], [.ctx]
.quit_free_buf:
        stdcall [mem.free], [.buf]
.quit:
        ret
endp

; These crash.*_oneshot functions below are wrappers to <hash_name>.oneshot,
; <mac_name>.oneshot and <cipher_name>.oneshot functions. The functions pop
; [_id] argument from the stack and jump to the oneshot function of the
; corresponding algorithm with all the other arguments in place.
; You can also call <hash_name/mac_name/cipher_name>.oneshot functions directly.

; Hash data in a buffer.
; The function hashes data in the specified buffer with the specified algorithm.
; The result is written to the very beginning of the specified context buffer in
; binary format.
; Parameters:
; [_id] = ID of a hash function to use.
; [_ctx] = buffer pointer for internal use, LIBCRASH_CTX_LEN bytes is enough.
; [_in] = pointer to input data
; [_len] = length of input data
; Return value:
; binary hash value in [_ctx] buffer.
crash.hash_oneshot:     ; _id, _ctx, _in, _len
        pop     eax
        xchg    eax, [esp]
        imul    eax, sizeof.hash_item
        lea     eax, [crash._.hash_table+eax]
        jmp     [eax+hash_item.oneshot]

; Calculate MAC of data in the buffer.
; The function calculates a MAC of data in the specified buffer with the
; specified algorithm and key. The result is written to the very beginning of
; the specified context buffer in binary format.
; Parameters:
; [_id] = ID of a hash function to use.
; [_ctx] = buffer pointer for internal use, LIBCRASH_CTX_LEN bytes is enough.
; [_in] = pointer to input data
; [_len] = length of input data
; [_key] = key pointer, no NULL terminator is needed
; [_key_len] = length of the [_key] data, in bytes
; Return value:
; binary MAC value in [_ctx] buffer.
crash.mac_oneshot:      ; _id, _ctx, _in, _len, _key, _key_len
        pop     eax
        xchg    eax, [esp]
        imul    eax, sizeof.mac_item
        lea     eax, [crash._.mac_table+eax]
        jmp     [eax+mac_item.oneshot]

; Encrypt or decrypt data in buffer.
; The function encrypts or decrypts data in the specified buffer using a
; specified algorithm, a key and an input vector. The result is written to
; another specified buffer.
; * The input and output buffers can sometimes be the same buffer depending on
; the cipher. If unsure, use different buffers.
; * The maximum difference in input/output data lengths is LIBCRASH_MAX_PAD_LEN.
; Parameters:
; [_id] = ID of a MAC function to use.
; [_ctx] = buffer pointer for internal use, LIBCRASH_CTX_LEN bytes is enough.
; [_key] = key pointer, NULL terminated
; [_iv] = input vector pointer, no NULL terminator is needed
; [_flags] = see LIBCRASH_CIPHER_* in libcrash.inc
; [_in] = pointer to input data
; [_len] = length of input data
; [_out] = pointer to output data
; Return value: none
crash.crypt_oneshot:    ; _id, _ctx, _key, _iv, _flags, _in, _len, _out
        pop     eax
        xchg    eax, [esp]
        imul    eax, sizeof.cipher_item
        lea     eax, [crash._.cipher_table+eax]
        jmp     [eax+cipher_item.oneshot]

section '.data' writeable align 16
mem.alloc dd ?
mem.free dd ?
io_buf_size dd LIBCRASH_BUF_SIZE
; FIXME: IDs shouldn't be indexes, should they?
align 4
crash._.hash_table dd \
        crc32.init,    crc32.update,    crc32.finish,    crc32.oneshot, \
        sizeof.ctx_crc32,       CRC32_LEN/4, \
        md5.init,      md5.update,      md5.finish,      md5.oneshot, \
        sizeof.ctx_md5,         MD5_LEN/4, \
        sha1.init,     sha1.update,     sha1.finish,     sha1.oneshot, \
        sizeof.ctx_sha1,        SHA1_LEN/4, \
        sha2_224.init, sha2_224.update, sha2_224.finish, sha2_224.oneshot, \
        sizeof.ctx_sha2_224256, SHA2_224_LEN/4, \
        sha2_256.init, sha2_256.update, sha2_256.finish, sha2_256.oneshot, \
        sizeof.ctx_sha2_224256, SHA2_256_LEN/4, \
        sha2_384.init, sha2_384.update, sha2_384.finish, sha2_384.oneshot, \
        sizeof.ctx_sha2_384512, SHA2_384_LEN/4, \
        sha2_512.init, sha2_512.update, sha2_512.finish, sha2_512.oneshot, \
        sizeof.ctx_sha2_384512, SHA2_512_LEN/4, \
        sha3_224.init, sha3.update,     sha3.finish,     sha3_224.oneshot, \
        sizeof.ctx_sha3,        SHA3_224_LEN/4, \
        sha3_256.init, sha3.update,     sha3.finish,     sha3_256.oneshot, \
        sizeof.ctx_sha3,        SHA3_256_LEN/4, \
        sha3_384.init, sha3.update,     sha3.finish,     sha3_384.oneshot, \
        sizeof.ctx_sha3,        SHA3_384_LEN/4, \
        sha3_512.init, sha3.update,     sha3.finish,     sha3_512.oneshot, \
        sizeof.ctx_sha3,        SHA3_512_LEN/4, \
        0

align 4
crash._.mac_table dd \
        poly1305.init,      poly1305.update,      poly1305.finish, \
        poly1305.oneshot,      sizeof.ctx_poly1305, POLY1305_LEN/4, \
        hmac_sha2_256.init, hmac_sha2_256.update, hmac_sha2_256.finish, \
        hmac_sha2_256.oneshot, sizeof.ctx_hmac,     HMAC_SHA2_256_LEN/4, \
        hmac_sha2_512.init, hmac_sha2_512.update, hmac_sha2_512.finish, \
        hmac_sha2_512.oneshot, sizeof.ctx_hmac,     HMAC_SHA2_512_LEN/4, \
        0

align 4
crash._.cipher_table dd \
        chacha20.init,  chacha20.update,  chacha20.finish,  chacha20.oneshot, \
        sizeof.ctx_chacha20, 0, \
        aes256ctr.init, aes256ctr.update, aes256ctr.finish, aes256ctr.oneshot, \
        sizeof.ctx_aes_ctr,  0, \
        aes256cbc.init, aes256cbc.update, aes256cbc.finish, aes256cbc.oneshot, \
        sizeof.ctx_aes_cbc,  0, \
        0

IncludeIGlobals

align 4
@EXPORT:

export \
        crash.init, "lib_init", \
        crash.hash, "crash_hash", \
        crash.mac, "crash_mac", \
        crash.crypt, "crash_crypt", \
        crash.hash_oneshot, "crash_hash_oneshot", \
        crash.mac_oneshot, "crash_mac_oneshot", \
        crash.crypt_oneshot, "crash_crypt_oneshot", \
        \
        crc32.init, "crc32_init", \
        crc32.update, "crc32_update", \
        crc32.finish, "crc32_finish", \
        crc32.oneshot, "crc32_oneshot", \
        md5.init, "md5_init", \
        md5.update, "md5_update", \
        md5.finish, "md5_finish", \
        md5.oneshot, "md5_oneshot", \
        sha1.init, "sha1_init", \
        sha1.update, "sha1_update", \
        sha1.finish, "sha1_finish", \
        sha1.oneshot, "sha1_oneshot", \
        sha2_224.init, "sha2_224_init", \
        sha2_224.update, "sha2_224_update", \
        sha2_224.finish, "sha2_224_finish", \
        sha2_224.oneshot, "sha2_224_oneshot", \
        sha2_256.init, "sha2_256_init", \
        sha2_256.update, "sha2_256_update", \
        sha2_256.finish, "sha2_256_finish", \
        sha2_256.oneshot, "sha2_256_oneshot", \
        sha2_384.init, "sha2_384_init", \
        sha2_384.update, "sha2_384_update", \
        sha2_384.finish, "sha2_384_finish", \
        sha2_384.oneshot, "sha2_384_oneshot", \
        sha2_512.init, "sha2_512_init", \
        sha2_512.update, "sha2_512_update", \
        sha2_512.finish, "sha2_512_finish", \
        sha2_512.oneshot, "sha2_512_oneshot", \
        sha3_224.init, "sha3_224_init", \
        sha3_224.update, "sha3_224_update", \
        sha3_224.finish, "sha3_224_finish", \
        sha3_224.oneshot, "sha3_224_oneshot", \
        sha3_256.init, "sha3_256_init", \
        sha3_256.update, "sha3_256_update", \
        sha3_256.finish, "sha3_256_finish", \
        sha3_256.oneshot, "sha3_256_oneshot", \
        sha3_384.init, "sha3_384_init", \
        sha3_384.update, "sha3_384_update", \
        sha3_384.finish, "sha3_384_finish", \
        sha3_384.oneshot, "sha3_384_oneshot", \
        sha3_512.init, "sha3_512_init", \
        sha3_512.update, "sha3_512_update", \
        sha3_512.finish, "sha3_512_finish", \
        sha3_512.oneshot, "sha3_512_oneshot", \
        \
        poly1305.init, "poly1305_init", \
        poly1305.update, "poly1305_update", \
        poly1305.finish, "poly1305_finish", \
        poly1305.oneshot, "poly1305_oneshot", \
        hmac_sha2_256.init, "hmac_sha2_256_init", \
        hmac_sha2_256.update, "hmac_sha2_256_update", \
        hmac_sha2_256.finish, "hmac_sha2_256_finish", \
        hmac_sha2_256.oneshot, "hmac_sha2_256_oneshot", \
        hmac_sha2_512.init, "hmac_sha2_512_init", \
        hmac_sha2_512.update, "hmac_sha2_512_update", \
        hmac_sha2_512.finish, "hmac_sha2_512_finish", \
        hmac_sha2_512.oneshot, "hmac_sha2_512_oneshot", \
        \
        chacha20.init, "chacha20_init", \
        chacha20.update, "chacha20_update", \
        chacha20.finish, "chacha20_finish", \
        chacha20.oneshot, "chacha20_oneshot", \
        aes256ctr.init, "aes256ctr_init", \
        aes256ctr.update, "aes256ctr_update", \
        aes256ctr.finish, "aes256ctr_finish", \
        aes256ctr.oneshot, "aes256ctr_oneshot", \
        aes256cbc.init, "aes256cbc_init", \
        aes256cbc.update, "aes256cbc_update", \
        aes256cbc.finish, "aes256cbc_finish", \
        aes256cbc.oneshot, "aes256cbc_oneshot"
