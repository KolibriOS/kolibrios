;    modexp.asm - Modular exponentiation test suite
;
;    Copyright (C) 2015-2017 Jeffrey Amelynck
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

__DEBUG__       = 1
__DEBUG_LEVEL__ = 1

MAX_BITS        = 512

use32

        db      'MENUET01'      ; signature
        dd      1               ; header version
        dd      start           ; entry point
        dd      i_end           ; initialized size
        dd      mem+65536       ; required memory
        dd      mem+65536       ; stack pointer
        dd      0               ; parameters
        dd      0               ; path

include '../../../macros.inc'
purge mov,add,sub
include '../../../proc32.inc'
include '../../../debug-fdo.inc'

include '../mpint.inc'

start:

        DEBUGF  1, "ModExp Test suite\n"

        DEBUGF  1, "mpint_print(0x0)\n"
        mov     dword[mpint_A+00], 0
        stdcall mpint_print, mpint_A

        mov     dword[mpint_A+00], 32
        mov     dword[mpint_A+04], 0xCAFEBABE
        mov     dword[mpint_A+08], 0xDEADBEEF
        mov     dword[mpint_A+12], 0xCAFEBABE
        mov     dword[mpint_A+16], 0xDEADBEEF
        mov     dword[mpint_A+20], 0xCAFEBABE
        mov     dword[mpint_A+24], 0xDEADBEEF
        mov     dword[mpint_A+28], 0xCAFEBABE
        mov     dword[mpint_A+32], 0xDEADBEEF
        DEBUGF  1, "mpint_print(0xCAFEBABEDEADBEEF...)\n"
        stdcall mpint_print, mpint_A

        DEBUGF  1, "mpint_shl(A, 3)\n"
        stdcall mpint_shl, mpint_A, 3
        stdcall mpint_print, mpint_A

        DEBUGF  1, "mpint_shl(A, 29)\n"
        stdcall mpint_shl, mpint_A, 29
        stdcall mpint_print, mpint_A

        DEBUGF  1, "8 times mpint_shl1\n"
        stdcall mpint_shl1, mpint_A
        stdcall mpint_print, mpint_A
        stdcall mpint_shl1, mpint_A
        stdcall mpint_print, mpint_A
        stdcall mpint_shl1, mpint_A
        stdcall mpint_print, mpint_A
        stdcall mpint_shl1, mpint_A
        stdcall mpint_print, mpint_A
        stdcall mpint_shl1, mpint_A
        stdcall mpint_print, mpint_A
        stdcall mpint_shl1, mpint_A
        stdcall mpint_print, mpint_A
        stdcall mpint_shl1, mpint_A
        stdcall mpint_print, mpint_A
        stdcall mpint_shl1, mpint_A
        stdcall mpint_print, mpint_A

        mov     dword[mpint_B+00], 32
        mov     dword[mpint_B+04], 0xCAFEBABE
        mov     dword[mpint_B+08], 0xDEADBEEF
        mov     dword[mpint_B+12], 0xCAFEBABE
        mov     dword[mpint_B+16], 0xDEADBEEF
        mov     dword[mpint_B+20], 0xCAFEBABE
        mov     dword[mpint_B+24], 0xDEADBEEF
        mov     dword[mpint_B+28], 0xCAFEBABE
        mov     dword[mpint_B+32], 0xDEADBEEF
        DEBUGF  1, "mpint_print(A)\n"
        stdcall mpint_print, mpint_A
        DEBUGF  1, "mpint_print(B)\n"
        stdcall mpint_print, mpint_B
        DEBUGF  1, "mpint_add(B, A)\n"
        stdcall mpint_add, mpint_B, mpint_A
        stdcall mpint_print, mpint_B
        DEBUGF  1, "mpint_sub(B, A)\n"
        stdcall mpint_sub, mpint_B, mpint_A
        stdcall mpint_print, mpint_B

        mov     dword[mpint_B+00], 24
        mov     dword[mpint_B+04], 0xCAFEBABE
        mov     dword[mpint_B+08], 0xDEADBEEF
        mov     dword[mpint_B+12], 0xCAFEBABE
        mov     dword[mpint_B+16], 0xDEADBEEF
        mov     dword[mpint_B+20], 0xCAFEBABE
        mov     dword[mpint_B+24], 0xDEADBEEF
        DEBUGF  1, "mpint_print(A)\n"
        stdcall mpint_print, mpint_A
        DEBUGF  1, "mpint_print(B)\n"
        stdcall mpint_print, mpint_B
        DEBUGF  1, "mpint_mod(A, B)\n"
        stdcall mpint_mod, mpint_A, mpint_B
        stdcall mpint_print, mpint_A

        mov     dword[mpint_A+0], 2
        mov     dword[mpint_A+4], 1936
        mov     dword[mpint_B+0], 2
        mov     dword[mpint_B+4], 497
        stdcall mpint_cmp, mpint_A, mpint_B
        stdcall mpint_mod, mpint_A, mpint_B
        DEBUGF  1, "mpint_mod(1936, 497)\n"
        stdcall mpint_print, mpint_A

        mov     dword[mpint_A+00], 32
        mov     dword[mpint_A+04], 0x11111111
        mov     dword[mpint_A+08], 0x22222222
        mov     dword[mpint_A+12], 0x33333333
        mov     dword[mpint_A+16], 0x44444444
        mov     dword[mpint_A+20], 0x55555555
        mov     dword[mpint_A+24], 0x88888888
        mov     dword[mpint_A+28], 0xAAAAAAAA
        mov     dword[mpint_A+32], 0xCCCCCCCC

        mov     dword[mpint_B+0], 2
        mov     dword[mpint_B+4], 0x0100
        DEBUGF  1, "mpint_print(A)\n"
        stdcall mpint_print, mpint_A
        DEBUGF  1, "mpint_print(B)\n"
        stdcall mpint_print, mpint_B
        DEBUGF  1, "mpint_mul(C, A, B)\n"
        stdcall mpint_mul, mpint_C, mpint_A, mpint_B
        stdcall mpint_print, mpint_C
        stdcall mpint_print, mpint_A
        stdcall mpint_print, mpint_B
        DEBUGF  1, "mpint_mul(C, B, A)\n"
        stdcall mpint_mul, mpint_C, mpint_B, mpint_A
        stdcall mpint_print, mpint_C

        stdcall mpint_bits, mpint_C
        DEBUGF  1, "mpint_bits(C): %u\n", eax

        mov     dword[mpint_A+0], 1
        mov     dword[mpint_A+4], 3
        mov     dword[mpint_B+0], 1
        mov     dword[mpint_B+4], 4
        mov     dword[mpint_C+0], 2
        mov     dword[mpint_C+4], 5
        stdcall mpint_modexp, mpint_D, mpint_A, mpint_B, mpint_C
        DEBUGF  1, "mpint_modexp(3, 4, 5)\n"
        stdcall mpint_print, mpint_D

        mcall   -1

i_end:

mpint_A         rb MPINT_MAX_LEN+4
mpint_B         rb MPINT_MAX_LEN+4
mpint_C         rb MPINT_MAX_LEN+4
mpint_D         rb MPINT_MAX_LEN+4

mpint_tmp       rb MPINT_MAX_LEN+4

include_debug_strings

mem: