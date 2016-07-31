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


LIBCRASH_CRC32          = 0
LIBCRASH_MD4            = 1
LIBCRASH_MD5            = 2
LIBCRASH_SHA1           = 3
LIBCRASH_SHA224         = 4
LIBCRASH_SHA256         = 5
LIBCRASH_SHA384         = 6
LIBCRASH_SHA512         = 7
LIBCRASH_SHA3_224       = 8
LIBCRASH_SHA3_256       = 9
LIBCRASH_SHA3_384       = 10
LIBCRASH_SHA3_512       = 11
LIBCRASH_LAST           = 11


struct crash_item
        init    dd ?
        update  dd ?
        final   dd ?
        len_out dd ?
ends

; CRC32

CRC32_HASH_SIZE = 4
CRC32_ALIGN = 4
CRC32_ALIGN_MASK = CRC32_ALIGN - 1

struct ctx_crc32
        hash    rd 1
ends

; MD4

MD4_BLOCK_SIZE = 64
MD4_HASH_SIZE  = 16
MD4_ALIGN      = 4
MD4_ALIGN_MASK = MD4_ALIGN - 1

struct ctx_md4
        hash            rb MD4_HASH_SIZE
        block           rb MD4_BLOCK_SIZE
        index           rd 1
        msglen_0        rd 1
        msglen_1        rd 1
ends

; MD5

MD5_BLOCK_SIZE = 64
MD5_HASH_SIZE  = 16
MD5_ALIGN      = 4
MD5_ALIGN_MASK = MD5_ALIGN - 1

struct ctx_md5
        hash            rb MD5_HASH_SIZE
        block           rb MD5_BLOCK_SIZE
        index           rd 1
        msglen_0        rd 1
        msglen_1        rd 1
ends

; SHA1

SHA1_BLOCK_SIZE = 64
SHA1_HASH_SIZE  = 20
SHA1_ALIGN      = 4
SHA1_ALIGN_MASK = SHA1_ALIGN - 1

struct ctx_sha1
        hash            rb SHA1_HASH_SIZE
        block           rb SHA1_BLOCK_SIZE
        index           rd 1
        msglen_0        rd 1
        msglen_1        rd 1
ends

; SHA2

SHA224256_BLOCK_SIZE = 64
SHA224256_INIT_SIZE  = 32
SHA224_HASH_SIZE     = 28
SHA256_HASH_SIZE     = 32
SHA224256_ALIGN      = 4
SHA224256_ALIGN_MASK = SHA224256_ALIGN - 1

struct ctx_sha224256
        hash            rb SHA224256_INIT_SIZE
        block           rb SHA224256_BLOCK_SIZE
        index           rd 1
        msglen_0        rd 1
        msglen_1        rd 1
ends

SHA384512_BLOCK_SIZE = 128
SHA384512_INIT_SIZE  = 64

SHA384_HASH_SIZE     = 48
SHA512_HASH_SIZE     = 64

SHA384512_ALIGN      = 16
SHA384512_ALIGN_MASK = SHA384512_ALIGN - 1

struct ctx_sha384512
        hash            rb SHA384512_INIT_SIZE
        block           rb SHA384512_BLOCK_SIZE
        index           rd 1
        msglen_0        rd 1
        msglen_1        rd 1
        msglen_2        rd 1
        msglen_3        rd 1
                        rd 3    ; align
        ; tmp vars
        w               rq 80
        A               rq 1
        B               rq 1
        C               rq 1
        D               rq 1
        E               rq 1
        F               rq 1
        G               rq 1
        H               rq 1
        temp            rq 1
ends

; SHA3

SHA3224_BLOCK_SIZE      = 144
SHA3256_BLOCK_SIZE      = 136
SHA3384_BLOCK_SIZE      = 104
SHA3512_BLOCK_SIZE      = 72
SHA3MAX_BLOCK_SIZE      = SHA3224_BLOCK_SIZE

SHA3_INIT_SIZE          = 200

SHA3224_HASH_SIZE       = 28
SHA3256_HASH_SIZE       = 32
SHA3384_HASH_SIZE       = 48
SHA3512_HASH_SIZE       = 64

SHA3_ALIGN              = 16
SHA3_ALIGN_MASK         = SHA3_ALIGN-1

struct ctx_sha3
        hash            rb SHA3_INIT_SIZE
                        rb SHA3_ALIGN - (SHA3_INIT_SIZE mod SHA3_ALIGN)
        block           rb SHA3MAX_BLOCK_SIZE
                        rb SHA3_ALIGN - (SHA3MAX_BLOCK_SIZE mod SHA3_ALIGN)
        index           rd 1
        block_size      rd 1
        rounds_cnt      rd 1
                        rd 1    ; align
        ; tmp vars
        C               rq 5
        D               rq 5
ends

