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

LIBCRASH_ALIGN = 16     ; align your data for speed

; hash IDs
LIBCRASH_CRC32          = 0
LIBCRASH_MD5            = 1
LIBCRASH_SHA1           = 2
LIBCRASH_SHA2_224       = 3
LIBCRASH_SHA2_256       = 4
LIBCRASH_SHA2_384       = 5
LIBCRASH_SHA2_512       = 6
LIBCRASH_SHA3_224       = 7
LIBCRASH_SHA3_256       = 8
LIBCRASH_SHA3_384       = 9
LIBCRASH_SHA3_512       = 10

; mac IDs
LIBCRASH_POLY1305       = 0
LIBCRASH_HMAC_SHA2_256  = 1
LIBCRASH_HMAC_SHA2_512  = 2

; cipher IDs
LIBCRASH_CHACHA20       = 0
LIBCRASH_AES_256_CTR    = 1
LIBCRASH_AES_256_CBC    = 2

; cipher flags for crash_crypt
LIBCRASH_CIPHER_ENCRYPT = 0000b
LIBCRASH_CIPHER_DECRYPT = 0001b
LIBCRASH_CIPHER_PADDING = 0010b ; PKCS#5

; cipher output can be larger than input, e.g. for CBC mode with padding
CBC128_MAX_PAD_LEN = 128/8
LIBCRASH_MAX_PAD_LEN = CBC128_MAX_PAD_LEN

CRC32_LEN    = 4
MD5_LEN      = 16
SHA1_LEN     = 20
SHA2_224_LEN = 28
SHA2_256_LEN = 32
SHA2_384_LEN = 48
SHA2_512_LEN = 64
SHA3_224_LEN = 28
SHA3_256_LEN = 32
SHA3_384_LEN = 48
SHA3_512_LEN = 64
MAX_HASH_LEN = SHA3_512_LEN

POLY1305_LEN = 16
HMAC_SHA2_256_LEN = SHA2_256_LEN
HMAC_SHA2_512_LEN = SHA2_512_LEN

LIBCRASH_CTX_LEN = 0x500
