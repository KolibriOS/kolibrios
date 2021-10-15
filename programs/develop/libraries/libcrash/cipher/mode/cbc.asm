; libcrash -- cryptographic hash (and other) functions
;
; Copyright (C) <2021> Ivan Baravy
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

CBC128_BLOCK_SIZE = 128/8

struct ctx_cbc
        vector rd CBC128_BLOCK_SIZE/4
        block rd CBC128_BLOCK_SIZE/4
        has_data dd ?
ends

; ebx = context
proc cbc.init uses esi edi, _iv

        mov     esi, [_iv]
        lea     edi, [ebx+ctx_ctr.block_counter]
        mov     ecx, CTR128_BLOCK_SIZE/4
        rep movsd
        mov     [ebx+ctx_ctr.partial_cnt], 0
        ret
endp
