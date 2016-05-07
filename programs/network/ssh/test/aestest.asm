;    aestest.inc - AES test suite
;
;    Copyright (C) 2016 Ivan Baravy (dunkaist)
;    Copyright (C) 2016 Jeffrey Amelynck
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

format binary as ""

use32
    org 0x0
    db  'MENUET01'
    dd  0x01,start,i_end,e_end,e_end,0,0

__DEBUG__       = 1
__DEBUG_LEVEL__ = 1

include '../../../proc32.inc'
include '../../../macros.inc'
include '../../../dll.inc'
include '../../../debug-fdo.inc'
;include 'libcrash.inc'
include '../aes256.inc'
include '../aes256-ctr.inc'
include '../aes256-cbc.inc'

; Test vectors are taken from the very end of sp800-38a.pdf


start:
        mcall   68, 11

DEBUGF 1,'===========================================\n'
DEBUGF 1,'AES256_CTR_CRYPT\n'
        DEBUGF  1,'\n'
        DEBUGF  1,'key    : '
        stdcall dump_128bit_hex, key
        DEBUGF  1,'\n'
        DEBUGF  1,'         '
        stdcall dump_128bit_hex, key+16
        DEBUGF  1,'\n'
        DEBUGF  1,'\n'
        DEBUGF  1,'counter: '
        stdcall dump_128bit_hex, counter
        DEBUGF  1,'\n'
        DEBUGF  1,'\n'

        stdcall aes256_ctr_init, counter
        ; returns context, save it to ebx
        mov     ebx, eax
        stdcall aes256_set_encrypt_key, ebx, key

        mov     esi, plain
        mov     edi, cipher
        mov     ecx, 4
  @@:
        push    ecx
        stdcall aes256_ctr_crypt, ebx, esi, edi
        pop     ecx
        add     esi, 16
        add     edi, 16
        loop    @r

DEBUGF 1,'===========================================\n'
DEBUGF 1,'AES256_CBC_ENCRYPT\n'
        DEBUGF  1,'\n'
        DEBUGF  1,'key    : '
        stdcall dump_128bit_hex, key
        DEBUGF  1,'\n'
        DEBUGF  1,'         '
        stdcall dump_128bit_hex, key+16
        DEBUGF  1,'\n'
        DEBUGF  1,'\n'
        DEBUGF  1,'IV     : '
        stdcall dump_128bit_hex, iv
        DEBUGF  1,'\n'
        DEBUGF  1,'\n'

        stdcall aes256_cbc_init, iv
        ; returns context, save it to ebx
        mov     ebx, eax
        stdcall aes256_set_encrypt_key, ebx, key

        mov     esi, plain
        mov     edi, cipher
        mov     ecx, 4
  @@:
        push    ecx
        stdcall aes256_cbc_encrypt, ebx, esi, edi
        pop     ecx
        add     esi, 16
        add     edi, 16
        loop    @r

DEBUGF 1,'===========================================\n'
DEBUGF 1,'AES256_CBC_DECRYPT\n'
        DEBUGF  1,'\n'
        DEBUGF  1,'key    : '
        stdcall dump_128bit_hex, key
        DEBUGF  1,'\n'
        DEBUGF  1,'         '
        stdcall dump_128bit_hex, key+16
        DEBUGF  1,'\n'
        DEBUGF  1,'\n'
        DEBUGF  1,'IV     : '
        stdcall dump_128bit_hex, iv
        DEBUGF  1,'\n'
        DEBUGF  1,'\n'

        stdcall aes256_cbc_init, iv
        ; returns context, save it to ebx
        mov     ebx, eax
        stdcall aes256_set_decrypt_key, ebx, key

        mov     esi, cipher
        mov     edi, plain
        mov     ecx, 4
  @@:
        push    ecx
        stdcall aes256_cbc_decrypt, ebx, esi, edi
        pop     ecx
        add     esi, 16
        add     edi, 16
        loop    @r

quit:
DEBUGF 1,'===========================================\n'
        mcall   -1


key     db      0x60,0x3d,0xeb,0x10,0x15,0xca,0x71,0xbe,0x2b,0x73,0xae,0xf0,\
                0x85,0x7d,0x77,0x81,0x1f,0x35,0x2c,0x07,0x3b,0x61,0x08,0xd7,\
                0x2d,0x98,0x10,0xa3,0x09,0x14,0xdf,0xf4

iv      db      0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,\
                0x0c,0x0d,0x0e,0x0f

counter db      0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,\
                0xfc,0xfd,0xfe,0xff

plain   db      0x6b,0xc1,0xbe,0xe2,0x2e,0x40,0x9f,0x96,0xe9,0x3d,0x7e,0x11,\
                0x73,0x93,0x17,0x2a,0xae,0x2d,0x8a,0x57,0x1e,0x03,0xac,0x9c,\
                0x9e,0xb7,0x6f,0xac,0x45,0xaf,0x8e,0x51,0x30,0xc8,0x1c,0x46,\
                0xa3,0x5c,0xe4,0x11,0xe5,0xfb,0xc1,0x19,0x1a,0x0a,0x52,0xef,\
                0xf6,0x9f,0x24,0x45,0xdf,0x4f,0x9b,0x17,0xad,0x2b,0x41,0x7b,\
                0xe6,0x6c,0x37,0x10

cipher  rd      16

; CTR-AES256.Encrypt
;
; Key           603deb1015ca71be2b73aef0857d7781
;               1f352c073b6108d72d9810a30914dff4
; Init. Counter f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff
;
;
; Block #1 
;
; Input Block   f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff
; Output Block  0bdf7df1591716335e9a8b15c860c502
; Plaintext     6bc1bee22e409f96e93d7e117393172a
; Ciphertext    601ec313775789a5b7a7f504bbf3d228
;
;
; Block #2 
;
; Input Block   f0f1f2f3f4f5f6f7f8f9fafbfcfdff00
; Output Block  5a6e699d536119065433863c8f657b94
; Plaintext     ae2d8a571e03ac9c9eb76fac45af8e51
; Ciphertext    f443e3ca4d62b59aca84e990cacaf5c5
;
;
; Block #3 
;
; Input Block   f0f1f2f3f4f5f6f7f8f9fafbfcfdff01
; Output Block  1bc12c9c01610d5d0d8bd6a3378eca62
; Plaintext     30c81c46a35ce411e5fbc1191a0a52ef
; Ciphertext    2b0930daa23de94ce87017ba2d84988d
;
;
; Block #4 
;
; Input Block   f0f1f2f3f4f5f6f7f8f9fafbfcfdff02
; Output Block  2956e1c8693536b1bee99c73a31576b6
; Plaintext     f69f2445df4f9b17ad2b417be66c3710
; Ciphertext    dfc9c58db67aada613c2dd08457941a6


; CBC-AES256.Encrypt
; Key           603deb1015ca71be2b73aef0857d7781
;               1f352c073b6108d72d9810a30914dff4
; IV            000102030405060708090a0b0c0d0e0f
;
; Block #1
;
; Plaintext     6bc1bee22e409f96e93d7e117393172a
; Input Block   6bc0bce12a459991e134741a7f9e1925
; Output Block  f58c4c04d6e5f1ba779eabfb5f7bfbd6
; Ciphertext    f58c4c04d6e5f1ba779eabfb5f7bfbd6
;
; Block #2
;
; Plaintext     ae2d8a571e03ac9c9eb76fac45af8e51
; Input Block   5ba1c653c8e65d26e929c4571ad47587
; Output Block  9cfc4e967edb808d679f777bc6702c7d
; Ciphertext    9cfc4e967edb808d679f777bc6702c7d
;
; Block #3
;
; Plaintext     30c81c46a35ce411e5fbc1191a0a52ef
; Input Block   ac3452d0dd87649c8264b662dc7a7e92
; Output Block  39f23369a9d9bacfa530e26304231461
; Ciphertext    39f23369a9d9bacfa530e26304231461
;
; Block #4
;
; Plaintext     f69f2445df4f9b17ad2b417be66c3710
; Input Block   cf6d172c769621d8081ba318e24f2371
; Output Block  b2eb05e2c39be9fcda6c19078c6a9d1b
; Ciphertext    b2eb05e2c39be9fcda6c19078c6a9d1b


; CBC-AES256.Decrypt
; Key           603deb1015ca71be2b73aef0857d7781
;               1f352c073b6108d72d9810a30914dff4
; IV            000102030405060708090a0b0c0d0e0f
;
; Block #1
;
; Ciphertext    f58c4c04d6e5f1ba779eabfb5f7bfbd6
; Input Block   f58c4c04d6e5f1ba779eabfb5f7bfbd6
; Output Block  6bc0bce12a459991e134741a7f9e1925
; Plaintext     6bc1bee22e409f96e93d7e117393172a
;
; Block #2
;
; Ciphertext    9cfc4e967edb808d679f777bc6702c7d
; Input Block   9cfc4e967edb808d679f777bc6702c7d
; Output Block  5ba1c653c8e65d26e929c4571ad47587
; Plaintext     ae2d8a571e03ac9c9eb76fac45af8e51
;
; Block #3
;
; Ciphertext    39f23369a9d9bacfa530e26304231461
; Input Block   39f23369a9d9bacfa530e26304231461
; Output Block  ac3452d0dd87649c8264b662dc7a7e92
; Plaintext     30c81c46a35ce411e5fbc1191a0a52ef
;
; Block #4
;
; Ciphertext    b2eb05e2c39be9fcda6c19078c6a9d1b
; Input Block   b2eb05e2c39be9fcda6c19078c6a9d1b
; Output Block  cf6d172c769621d8081ba318e24f2371
; Plaintext     f69f2445df4f9b17ad2b417be66c3710



include_debug_strings

IncludeIGlobals

i_end:

IncludeUGlobals

rb 0x1000       ;stack
e_end:

