;    mpint.asm - Multi Precision INTeger routines test suite
;
;    Copyright (C) 2015-2021 Jeffrey Amelynck
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
__DEBUG_LEVEL__ = 2

MAX_BITS    = 4096

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

cmptestctr = 0x10000000

macro cmptesteq arg1, arg2 {
        stdcall mpint_cmp, arg1, arg2
        je      @f
        mov     eax, cmptestctr
        int3
  @@:
cmptestctr = cmptestctr + 1
}

macro cmptesta arg1, arg2 {
        stdcall mpint_cmp, arg1, arg2
        ja      @f
        mov     eax, cmptestctr
        int3
  @@:
cmptestctr = cmptestctr + 1
        stdcall mpint_cmp, arg2, arg1
        jb      @f
        mov     eax, cmptestctr
        int3
  @@:
cmptestctr = cmptestctr + 1
}

start:

        DEBUGF  3, "MPINT Test suite\n"

; First, do some checks on the compare routine
        cmptesteq mpint_0_0, mpint_0_0
        cmptesteq mpint_0_0, mpint_0_1
        cmptesteq mpint_0_1, mpint_0_0
        cmptesteq mpint_0_1, mpint_0_1
        cmptesteq mpint_0_5, mpint_0_0
        cmptesteq mpint_0_0, mpint_0_5
        cmptesteq mpint_0_5, mpint_0_1
        cmptesteq mpint_0_1, mpint_0_5

        cmptesteq mpint_1_1, mpint_1_1
        cmptesteq mpint_1_1, mpint_1_2
        cmptesteq mpint_1_2, mpint_1_1
        cmptesteq mpint_1_2, mpint_1_2
        cmptesteq mpint_1_5, mpint_1_1
        cmptesteq mpint_1_1, mpint_1_5
        cmptesteq mpint_1_5, mpint_1_2
        cmptesteq mpint_1_2, mpint_1_5

        cmptesteq mpint_2_1, mpint_2_1
        cmptesteq mpint_2_1, mpint_2_2
        cmptesteq mpint_2_2, mpint_2_1
        cmptesteq mpint_2_2, mpint_2_2
        cmptesteq mpint_2_5, mpint_2_1
        cmptesteq mpint_2_1, mpint_2_5
        cmptesteq mpint_2_5, mpint_2_2
        cmptesteq mpint_2_2, mpint_2_5

        cmptesta mpint_1_1, mpint_0_0
        cmptesta mpint_1_1, mpint_0_1
        cmptesta mpint_1_1, mpint_0_5

        cmptesta mpint_1_5, mpint_0_0
        cmptesta mpint_1_5, mpint_0_1
        cmptesta mpint_1_5, mpint_0_5

        cmptesta mpint_2_1, mpint_1_1
        cmptesta mpint_2_1, mpint_1_2
        cmptesta mpint_2_1, mpint_1_5

        cmptesta mpint_2_2, mpint_1_1
        cmptesta mpint_2_2, mpint_1_2
        cmptesta mpint_2_2, mpint_1_5

        cmptesta mpint_2_5, mpint_1_1
        cmptesta mpint_2_5, mpint_1_2
        cmptesta mpint_2_5, mpint_1_5

        cmptesta mpint_100, mpint_ff
        cmptesta mpint_10000, mpint_ff00
        cmptesta mpint_100000000, mpint_ff000000


iglobal
mpint_0_0 dd 0
          rb MPINT_MAX_LEN

mpint_0_1 dd 1
          db 0
          rb MPINT_MAX_LEN - 1

mpint_0_2 dd 2
          db 0, 0
          rb MPINT_MAX_LEN - 2

mpint_0_3 dd 3
          db 0, 0, 0
          rb MPINT_MAX_LEN - 3

mpint_0_4 dd 4
          db 0, 0, 0, 0
          rb MPINT_MAX_LEN - 4

mpint_0_5 dd 5
          db 0, 0, 0, 0, 0
          rb MPINT_MAX_LEN - 5

mpint_1_1 dd 1
          db 1
          rb MPINT_MAX_LEN - 1

mpint_1_2 dd 2
          db 1, 0
          rb MPINT_MAX_LEN - 2

mpint_1_3 dd 3
          db 1, 0, 0
          rb MPINT_MAX_LEN - 3

mpint_1_4 dd 4
          db 1, 0, 0, 0
          rb MPINT_MAX_LEN - 4

mpint_1_5 dd 5
          db 1, 0, 0, 0, 0
          rb MPINT_MAX_LEN - 5

mpint_2_1 dd 1
          db 2
          rb MPINT_MAX_LEN - 1

mpint_2_2 dd 2
          db 2, 0
          rb MPINT_MAX_LEN - 2

mpint_2_3 dd 3
          db 2, 0, 0
          rb MPINT_MAX_LEN - 3

mpint_2_4 dd 4
          db 2, 0, 0, 0
          rb MPINT_MAX_LEN - 4

mpint_2_5 dd 5
          db 2, 0, 0, 0, 0
          rb MPINT_MAX_LEN - 5

mpint_ff  dd 2
          db 0xff, 0
          rb MPINT_MAX_LEN - 2

mpint_100 dd 2
          db 0, 1
          rb MPINT_MAX_LEN - 2

mpint_ff00  dd 3
          db 0, 0xff, 0
          rb MPINT_MAX_LEN - 3

mpint_10000 dd 3
          db 0, 0, 1
          rb MPINT_MAX_LEN - 3

mpint_ff000000  dd 5
          db 0, 0, 0, 0xff, 0
          rb MPINT_MAX_LEN - 5

mpint_100000000 dd 5
          db 0, 0, 0, 0, 1
          rb MPINT_MAX_LEN - 5


endg

include "tests.inc"

        DEBUGF  3, "All tests completed\n"

        mcall   -1

IncludeIGlobals

i_end:

starttime dq ?

mpint_tmp       rb MPINT_MAX_LEN+4

include_debug_strings

mem: