;    libcrash -- cryptographic hash functions
;
;    Copyright (C) 2012-2014,2016 Ivan Baravy (dunkaist)
;
;    This program is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation, either version 3 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program.  If not, see <http://www.gnu.org/licenses/>.

format MS COFF

public @EXPORT as 'EXPORTS'

include '../../../../struct.inc'
include '../../../../proc32.inc'
include '../../../../macros.inc'
include '../../../../config.inc'
;include '../../../../debug.inc'

purge section,mov,add,sub
section '.flat' code readable align 16

include 'libcrash.inc'
include 'crc32.asm'
include 'md4.asm'
include 'md5.asm'
include 'sha1.asm'
include 'sha224_256.asm'
include 'sha384_512.asm'
include 'sha3.asm'


proc lib_init
	ret
endp


proc crash.hash  _hid, _data, _callback, _ctx
locals
        size dd ?
endl
        mov     [size], 0
        mov     eax, [_hid]
        imul    eax, sizeof.crash_item
        lea     edx, [crash._.table + eax]
        mov     ebx, [_ctx]

        stdcall [edx + crash_item.init], [_ctx]

  .hash:
        mov     esi, [_data]
        push    edx
        stdcall [edx + crash_item.update], [_ctx], [_data], [size]
        mov     [size], 0
        pop     edx

        mov     eax, [_callback]
        test    eax, eax
        jz      .quit
        push    edx
        stdcall [_callback], [size]
        pop     edx
        mov     [size], eax
        test    eax, eax
        jnz     .hash

        stdcall [edx + crash_item.final], [_ctx]
  .quit:
        ret
endp


proc crash.bin2hex _bin, _hex, _hid
        mov     eax, [_hid]
        imul    eax, sizeof.crash_item
        mov     ecx, [crash._.table + eax + crash_item.len_out]
        mov     ebx, crash._.bin2hex_table
        mov     esi, [_bin]
        mov     edi, [_hex]
  .next_byte:
        xor     eax, eax
        lodsb
        shl     eax, 4
        shr     al, 4
        xlatb
        xchg    al, ah
        xlatb
        stosw
        dec     ecx
        jnz     .next_byte
        xor     al, al
        stosb
        ret
endp


section '.data' data readable align 16
crash._.bin2hex_table   db      '0123456789abcdef'

crash._.table   dd \
        crc32.init,   crc32.update,     crc32.final,     CRC32_HASH_SIZE,  \
        md4.init,     md4.update,       md4.final,       MD4_HASH_SIZE,    \
        md5.init,     md5.update,       md5.final,       MD5_HASH_SIZE,    \
        sha1.init,    sha1.update,      sha1.final,      SHA1_HASH_SIZE,   \
        sha224.init,  sha224256.update, sha224256.final, SHA224_HASH_SIZE, \
        sha256.init,  sha224256.update, sha224256.final, SHA256_HASH_SIZE, \
        sha384.init,  sha384512.update, sha384512.final, SHA384_HASH_SIZE, \
        sha512.init,  sha384512.update, sha384512.final, SHA512_HASH_SIZE, \
        sha3224.init, sha3.update,      sha3.final,      SHA3224_HASH_SIZE,\
        sha3256.init, sha3.update,      sha3.final,      SHA3256_HASH_SIZE,\
        sha3384.init, sha3.update,      sha3.final,      SHA3384_HASH_SIZE,\
        sha3512.init, sha3.update,      sha3.final,      SHA3512_HASH_SIZE

align 4
@EXPORT:

export                                   \
    lib_init        , 'lib_init'       , \
    crash.hash      , 'crash_hash'     , \
    crash.bin2hex   , 'crash_bin2hex'

