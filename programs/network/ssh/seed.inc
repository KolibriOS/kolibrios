;    seed.inc - Collect some entropy from KolibriOS system
;
;    Copyright (C) 2021 Jeffrey Amelynck
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


align 4
create_seed:

        push    ebx edx

        mcall   3                       ; System time
        xor     edx, eax
        rol     edx, 1
        mcall   14                      ; Screen size
        xor     edx, eax
        rol     edx, 1
        mcall   18, 4                   ; Idle time counter
        xor     edx, eax
        rol     edx, 1
        mcall   18, 5                   ; CPU clock rate
        xor     edx, eax
        rol     edx, 1
        mcall   18, 7                   ; Active window slot
        xor     edx, eax
        rol     edx, 1
        mcall   18, 16                  ; Free RAM space
        xor     edx, eax
        rol     edx, 1
        mcall   18, 17                  ; Total RAM space
        xor     edx, eax
        rol     edx, 1
        mcall   18, 21                  ; Active window slot
        xor     edx, eax
        rol     edx, 1
        mcall   26, 10                  ; High precision time counter
        xor     edx, eax
        rol     edx, 1
        xor     edx, ebx
        rol     edx, 1
        mcall   37, 0                   ; Screen coordinates of the cursor
        xor     edx, eax
        rol     edx, 1
        mcall   54, 0                   ; Number of slots on the clipboard
        xor     edx, eax
        rol     edx, 1
        mcall   66, 3                   ; Status of the keyboard control keys
        xor     edx, eax
        rol     edx, 1
        mcall   68, 0                   ; Task switch counter
        xor     edx, eax
        rol     edx, 1
        mcall   74, 0x108               ; Network interface 1 TX bytes counter
        xor     edx, eax
        rol     edx, 1
        mcall   74, 0x109               ; Network interface 1 RX bytes counter
        xor     edx, eax
        rol     edx, 1
        mcall   76, 0x100               ; Network interface 1 MAC address
        xor     eax, ebx
        xor     eax, edx

        pop     edx ebx

        ret