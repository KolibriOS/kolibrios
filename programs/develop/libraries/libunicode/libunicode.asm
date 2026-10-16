; libunicode -- KolibriOS Unicode Library
;
; Copyright (C) <2026> KolibriOS.org Team
; Author:
;           1. Swarnadeep Paul <swarnadeep@mail.com>
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

include "../../../macros.inc"
include "../../../proc32.inc"

section '.flat' code readable align 16

;=============================================================
;   Valid UTF-8 character checking routine
;   Parameters:
;       _str_ptr = pointer to the start of the character
;                  should be part of null terminated string
;_next_max_bytes = Give the total number of possible bytes that
;                  can be read from the current position.
;   Return values:
;       eax = 1,2,3,4 <- if Valid
;             0 <- if not Valid
;   Examples:
;       ; Here 5 means that counting from the current byte,
;       ; this function can read five bytes at max.
;       invoke  is_valid_utf8_char, my_string, 5
;       test    eax, eax        ; Check if valid
;       jz      .handle_error   ; error handling by user
;       add     esi, eax        ; Advance the pointer by the length!
;=============================================================
;   PRIVATE:
;       eax = pointer to str
;       ebx = internal variable
;       ecx = load the bytes for testing
;=============================================================

proc is_valid_utf8_char uses ebx, _str_ptr, _next_max_bytes
        cmp     [_next_max_bytes], 1  ; If less than 1 no valid reading bytes
        jb      .is_invalid

        mov     eax, [_str_ptr]     ; Move the pointer to eax
        movzx   ecx, byte [eax]     ; Move the first byte to ecx

        cmp     cl, 0x80            ; If less than 0x80 than valid ascii
        jb      .is_valid_byte

        cmp     cl, 0xC2
        jb      .is_invalid         ; rejects 0x80–0xC1 (continution and overlong)

; 0xE0 is start of 3 byte so check if smaller
        cmp     cl, 0xE0
        jb      .check_valid_2_bytes

; 0xF0 is start of 4 byte so check if smaller
        cmp     cl, 0xF0
        jb      .check_valid_3_bytes

; 0xF4 is upper limit of unicode first byte as of 20-March-2026
; The limit is U+10FFFF
        cmp     cl, 0xF4
        ja      .is_invalid

; since checking of 1 byte, 2 byte, 3 byte is finished
; we will start to check for 4 byte sequence
.check_valid_4_bytes:
        cmp     [_next_max_bytes], 4  ; If it is four bytes then max bytes should be at least four.
        jb      .is_invalid

        ; cl contains the first byte
        movzx   ebx, byte [eax+1]

; If it is 0x00 then invalid
        test    ebx, ebx
        jz      .is_invalid

; For cl == 0xF0; second byte must be 0x90–0xBF
        cmp     cl, 0xF0
        jnz     .check_f4

        cmp     ebx, 0x90
        jb      .is_invalid
        cmp     ebx, 0xBF
        ja      .is_invalid

        jmp     .check_bytes_3_4

; For cl == 0xF4; second byte must be 0x80–0x8F
.check_f4:
        cmp     cl, 0xF4
        jnz     .check_general_F1_F3

        cmp     ebx, 0x80
        jb      .is_invalid
        cmp     ebx, 0x8F
        ja      .is_invalid

        jmp     .check_bytes_3_4

.check_general_F1_F3:
        and     ebx, 0xC0
        cmp     ebx, 0x80
        jne     .is_invalid

.check_bytes_3_4:
        movzx   ebx, byte [eax+2]
; If it is 0x00 then invalid
        test    ebx, ebx
        jz      .is_invalid

        and     ebx, 0xC0
        cmp     ebx, 0x80
        jne     .is_invalid

        movzx   ebx, byte [eax+3]
; If it is 0x00 then invalid
        test    ebx, ebx
        jz      .is_invalid

        and     ebx, 0xC0
        cmp     ebx, 0x80
        jne     .is_invalid

        jmp     .is_valid_4_byte

.check_valid_2_bytes:
        cmp     [_next_max_bytes], 2  ; If it is 2 bytes then max bytes should be at least 2.
        jb      .is_invalid

        ; cl contains the first byte
        movzx   ebx, byte [eax+1]
; If it is 0x00 then invalid
        test    ebx, ebx
        jz      .is_invalid

        and     ebx, 0xC0
        cmp     ebx, 0x80
        jne     .is_invalid

        jmp     .is_valid_2_byte
        
.check_valid_3_bytes:
        cmp     [_next_max_bytes], 3  ; If it is 3 bytes then max bytes should be at least 3.
        jb      .is_invalid

        ; cl contains the first byte
        movzx   ebx, byte [eax+1]

; If it is 0x00 then invalid
        test    ebx, ebx
        jz      .is_invalid

; for cl == 0xE0; second byte must be 0xA0 - 0xBF
        cmp     cl, 0xE0
        jnz     .check_ED

        cmp     ebx, 0xA0
        jb      .is_invalid
        cmp     ebx, 0xBF
        ja      .is_invalid

        jmp     .check_byte_3

; for cl == 0xED; second byte must be 0x80 - 0x9F
.check_ED:
        cmp     cl, 0xED
        jnz     .check_general_Exx

        cmp     ebx, 0x80
        jb      .is_invalid
        cmp     ebx, 0x9F
        ja      .is_invalid

        jmp     .check_byte_3
        
.check_general_Exx:
        and     ebx, 0xC0
        cmp     ebx, 0x80
        jne     .is_invalid

.check_byte_3:
        movzx   ebx, byte [eax+2]
; If it is 0x00 then invalid
        test    ebx, ebx
        jz      .is_invalid

        and     ebx, 0xC0
        cmp     ebx, 0x80
        jne     .is_invalid

        jmp     .is_valid_3_byte

.is_valid_byte:
        mov     eax, 1      ; Return 1 (Valid) in eax
        ret

.is_valid_2_byte:
        mov     eax, 2      ; Return 2 (Valid) in eax
        ret

.is_valid_3_byte:
        mov     eax, 3      ; Return 3 (Bytes) in eax
        ret

.is_valid_4_byte:
        mov     eax, 4      ; Return 4 (Bytes) in eax
        ret

.is_invalid:
        mov     eax, 0      ; Return 0 (Invalid) in eax
        ret
endp

;=============================================================
;   CodePoint Counting function
;   Parameters:
;       _str_ptr = pointer to the memory address of the null
;             terminated string
;   Return values:
;       eax = total codepoints
;=============================================================

proc count_utf8_codepoints uses ebx, _str_ptr
        mov     eax, [_str_ptr]
        mov     ebx, 0

.read_loop:
        mov     cl, byte [eax]
        
        test    cl, cl        ; if it is an ending byte (0)
        je      .done

        and     cl, 0xC0
        cmp     cl, 0x80
        je      .skip_count
        inc     ebx

.skip_count:
        inc     eax
        jmp     .read_loop
.done:
        mov     eax, ebx
        ret
endp

;=============================================================
;   Grapheme Counting function
;   Parameters:
;       _str_ptr = pointer to the memory address of the null
;             terminated string
;   Return values:
;       eax = total grapheme count
;=============================================================

proc count_utf8_graphemes uses ebx, _str_ptr
        mov     eax, [_str_ptr]
        mov     ebx, 0

.read_loop:
        mov     cl, byte [eax]
        
        test    cl, cl        ; if it is an ending byte (0)
        je      .done
; Is this accent

        cmp     cl, 0xCC
        je      .skip_count

        cmp     cl, 0xCD
        je      .skip_count

; Check for not a zero width joint
        cmp     cl, 0xE2
        jne     .not_any_special

        cmp byte [eax+1], 0x80
        jne     .not_any_special

        cmp byte [eax+2], 0x8D
        jne     .not_any_special

        dec     ebx
        add     eax, 3
        jmp     .read_loop

.not_any_special:
        and     cl, 0xC0        ; Is this a continution byte
        cmp     cl, 0x80
        je      .skip_count
        inc     ebx

.skip_count:
        inc     eax
        jmp     .read_loop

.done:
        mov     eax, ebx
        ret
endp

align 4
@EXPORT:

export \
        count_utf8_codepoints, "utf8.count_codepoints", \
        count_utf8_graphemes,   "utf8.count_graphemes", \
        is_valid_utf8_char,       "utf8.is_valid_char"

