;    libcrash -- cryptographic hash functions
;
;    Copyright (C) 2012-2014,2016,2019 Ivan Baravy (dunkaist)
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
        oneshot dd ?
        len_out dd ?
ends

struct crash_ctx
        hash rb 1024    ; context starts with hash data
ends


CRC32_HASH_SIZE     = 4
CRC32_BLOCK_SIZE    = 1

MD4_HASH_SIZE       = 16
MD4_BLOCK_SIZE      = 64

MD5_HASH_SIZE       = 16
MD5_BLOCK_SIZE      = 64

SHA1_HASH_SIZE      = 20
SHA1_BLOCK_SIZE     = 64

SHA224_HASH_SIZE    = 28
SHA224_BLOCK_SIZE   = 64

SHA256_HASH_SIZE    = 32
SHA256_BLOCK_SIZE   = 64

SHA384_HASH_SIZE    = 48
SHA384_BLOCK_SIZE   = 128

SHA512_HASH_SIZE    = 64
SHA512_BLOCK_SIZE   = 128

SHA3_224_HASH_SIZE  = 28
SHA3_224_BLOCK_SIZE = 144

SHA3_256_HASH_SIZE  = 32
SHA3_256_BLOCK_SIZE = 136

SHA3_384_HASH_SIZE  = 48
SHA3_384_BLOCK_SIZE = 104

SHA3_512_HASH_SIZE  = 64
SHA3_512_BLOCK_SIZE = 72
