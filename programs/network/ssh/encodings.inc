;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;   Written by CleverMouse                                        ;;
;;                                                                 ;;
;;         GNU GENERAL PUBLIC LICENSE                              ;;
;;          Version 2, June 1991                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

uglobal

utf8_bytes_rest dd ?                    ; bytes rest in current UTF8 sequence
utf8_char       dd ?                    ; first bits of current UTF8 character

endg


;get_next_byte:
;; Load next byte from the packet, translating to cp866 if necessary
;; At input esi = pointer to data, edx = limit of data
;; Output is either (translated) byte in al with CF set or CF cleared.
;        mov     eax, [encoding]
;        jmp     [get_byte_table+eax*4]
;
;get_byte_cp866:
;        cmp     esi, edx
;        jae     .nothing
;        lodsb
;.nothing:
;        ret
;
;get_byte_cp1251:
;        cmp     esi, edx
;        jae     .nothing
;        lodsb
;        cmp     al, 0x80
;        jb      @f
;        and     eax, 0x7F
;        mov     al, [cp1251_table+eax]
;@@:
;        stc
;.nothing:
;        ret

get_byte_utf8:
; UTF8 decoding is slightly complicated.
; One character can occupy one or more bytes.
; The boundary in packets theoretically can be anywhere in data,
; so this procedure keeps internal state between calls and handles
; one byte at a time, looping until character is read or packet is over.
; Globally, there are two distinct tasks: decode byte sequence to unicode char
; and convert this unicode char to our base encoding (that is cp866).
; 1. Check that there are data.
        cmp     esi, edx
        jae     .nothing
; 2. Load byte.
        lodsb
        movzx   ecx, al
; 3. Bytes in an UTF8 sequence can be of any of three types.
; If most significant bit is cleared, sequence is one byte and usual ASCII char.
; First byte of a sequence must be 11xxxxxx, other bytes are 10yyyyyy.
        and     al, 0xC0
        jns     .single_byte
        jp      .first_byte
; 4. This byte is not first in UTF8 sequence.
; 4a. Check that the sequence was started. If no, it is invalid byte
; and we simply ignore it.
        cmp     [utf8_bytes_rest], 0
        jz      get_byte_utf8
; 4b. Otherwise, it is really next byte and it gives some more bits of char.
        mov     eax, [utf8_char]
        shl     eax, 6
        lea     eax, [eax+ecx-0x80]
; 4c. Decrement number of bytes rest in the sequence.
; If it goes to zero, character is read, so return it.
        dec     [utf8_bytes_rest]
        jz      .got_char
        mov     [utf8_char], eax
        jmp     get_byte_utf8
; 5. If the byte is first in UTF8 sequence, calculate the number of leading 1s
; - it equals total number of bytes in the sequence; some other bits rest for
; leading bits in the character.
.first_byte:
        mov     eax, -1
@@:
        inc     eax
        add     cl, cl
        js      @b
        mov     [utf8_bytes_rest], eax
        xchg    eax, ecx
        inc     ecx
        shr     al, cl
        mov     [utf8_char], eax
        jmp     get_byte_utf8
; 6. If the byte is ASCII char, it is the character.
.single_byte:
        xchg    eax, ecx
.got_char:
; We got the character, now abandon a possible sequence in progress.
        and     [utf8_bytes_rest], 0
; Now second task. The unicode character is in eax, and now we shall convert it
; to cp866.
        cmp     eax, 0x80
        jb      .done
; 0x410-0x43F -> 0x80-0xAF, 0x440-0x44F -> 0xE0-0xEF, 0x401 -> 0xF0, 0x451 -> 0xF1
        cmp     eax, 0x401
        jz      .YO
        cmp     eax, 0x451
        jz      .yo
        cmp     eax, 0x410
        jb      .unrecognized
        cmp     eax, 0x440
        jb      .part1
        cmp     eax, 0x450
        jb      .part2
        cmp     eax, 0x25a0
        jae     .unrecognized
        sub     eax, 0x2500
        jb      .unrecognized
        mov     al, [cp866_boxes+eax]
        ret
.part1:
        sub     al, 0x10-0x80
.nothing:
.done:
        ret
.part2:
        sub     al, (0x40-0xE0) and 0xFF
        ret
.unrecognized:
        mov     al, '?'
        stc
        ret
.YO:
        mov     al, 0xF0
        stc
        ret
.yo:
        mov     al, 0xF1
        stc
        ret



;recode_to_cp866:
;        rep     movsb
;        ret
;
;recode_to_cp1251:
;        xor     eax, eax
;        jecxz   .nothing
;  .loop:
;        lodsb
;        cmp     al,0x80
;        jb      @f
;        mov     al, [cp866_table-0x80+eax]
;    @@: stosb
;        loop    .loop
;  .nothing:
;        ret

recode_to_utf8:
        jecxz   .nothing
  .loop:
        lodsb
        cmp     al, 0x80
        jb      .single_byte
        and     eax, 0x7F
        mov     ax, [utf8_table+eax*2]
        stosw
        loop    .loop
        ret
  .single_byte:
        stosb
        loop    .loop
  .nothing:
        ret

;recode:
;        mov     eax, [encoding]
;        jmp     [recode_proc+eax*4]



;encoding        dd      UTF8
;recode_proc     dd      recode_to_cp866, recode_to_cp1251, recode_to_utf8
;get_byte_table  dd      get_byte_cp866, get_byte_cp1251, get_byte_utf8


;cp1251_table:
;  db '?','?','?','?','?','?','?','?' , '?','?','?','?','?','?','?','?' ; 8
;  db '?','?','?','?','?',$F9,'?','?' , '?','?','?','?','?','?','?','?' ; 9
;  db '?',$F6,$F7,'?',$FD,'?','?','?' , $F0,'?',$F2,'?','?','?','?',$F4 ; A
;  db $F8,'?','?','?','?','?','?',$FA , $F1,$FC,$F3,'?','?','?','?',$F5 ; B
;  db $80,$81,$82,$83,$84,$85,$86,$87 , $88,$89,$8A,$8B,$8C,$8D,$8E,$8F ; C
;  db $90,$91,$92,$93,$94,$95,$96,$97 , $98,$99,$9A,$9B,$9C,$9D,$9E,$9F ; D
;  db $A0,$A1,$A2,$A3,$A4,$A5,$A6,$A7 , $A8,$A9,$AA,$AB,$AC,$AD,$AE,$AF ; E
;  db $E0,$E1,$E2,$E3,$E4,$E5,$E6,$E7 , $E8,$E9,$EA,$EB,$EC,$ED,$EE,$EF ; F

;    0   1   2   3   4   5   6   7     8   9   A   B   C   D   E   F

utf8_table:
        times 80h dw 0x98C3     ; default placeholder

; 0x80-0xAF -> 0x90D0-0xBFD0
repeat 0x30
        store byte 0xD0 at utf8_table+2*(%-1)
        store byte 0x90+%-1 at utf8_table+2*%-1
end repeat

; 0xE0-0xEF -> 0x80D1-0x8FD1
repeat 0x10
        store byte 0xD1 at utf8_table+2*(0xE0-0x80+%-1)
        store byte 0x80+%-1 at utf8_table+2*(0xE0-0x80+%)-1
end repeat

; 0xF0 -> 0x81D0, 0xF1 -> 0x91D1
        store dword 0x91D181D0 at utf8_table+2*(0xF0-0x80)

;cp866_table:
;  db $C0,$C1,$C2,$C3,$C4,$C5,$C6,$C7 , $C8,$C9,$CA,$CB,$CC,$CD,$CE,$CF ; 8
;  db $D0,$D1,$D2,$D3,$D4,$D5,$D6,$D7 , $D8,$D9,$DA,$DB,$DC,$DD,$DE,$DF ; 9
;  db $E0,$E1,$E2,$E3,$E4,$E5,$E6,$E7 , $E8,$E9,$EA,$EB,$EC,$ED,$EE,$EF ; A
;  db '?','?','?','?','?','?','?','?' , '?','?','?','?','?','?','?','?' ; B
;  db '?','?','?','?','?','?','?','?' , '?','?','?','?','?','?','?','?' ; C
;  db '?','?','?','?','?','?','?','?' , '?','?','?','?','?','?','?','?' ; D
;  db $F0,$F1,$F2,$F3,$F4,$F5,$F6,$F7 , $F8,$F9,$FA,$FB,$FC,$FD,$FE,$FF ; E
;  db $A8,$B8,$AA,$BA,$AF,$BF,$A1,$A2 , $B0,$95,$B7,'?',$B9,$A4,'?','?' ; F

;    0   1   2   3   4   5   6   7     8   9   A   B   C   D   E   F


; Codepoints for 0xB0-0xDF, unicode offset 0x2500
cp866_boxes:
        times 0xA0 db '?'

        store byte 0xB0 at cp866_boxes+0x91
        store byte 0xB1 at cp866_boxes+0x92
        store byte 0xB2 at cp866_boxes+0x93
        store byte 0xB3 at cp866_boxes+0x02
        store byte 0xB4 at cp866_boxes+0x24
        store byte 0xB5 at cp866_boxes+0x61
        store byte 0xB6 at cp866_boxes+0x62
        store byte 0xB7 at cp866_boxes+0x56

        store byte 0xB8 at cp866_boxes+0x55
        store byte 0xB9 at cp866_boxes+0x63
        store byte 0xBA at cp866_boxes+0x51
        store byte 0xBB at cp866_boxes+0x57
        store byte 0xBC at cp866_boxes+0x5D
        store byte 0xBD at cp866_boxes+0x5C
        store byte 0xBE at cp866_boxes+0x5B
        store byte 0xBF at cp866_boxes+0x10

        store byte 0xC0 at cp866_boxes+0x14
        store byte 0xC1 at cp866_boxes+0x34
        store byte 0xC2 at cp866_boxes+0x2C
        store byte 0xC3 at cp866_boxes+0x1C
        store byte 0xC4 at cp866_boxes+0x00
        store byte 0xC5 at cp866_boxes+0x3C
        store byte 0xC6 at cp866_boxes+0x5E
        store byte 0xC7 at cp866_boxes+0x5F

        store byte 0xC8 at cp866_boxes+0x5A
        store byte 0xC9 at cp866_boxes+0x54
        store byte 0xCA at cp866_boxes+0x69
        store byte 0xCB at cp866_boxes+0x66
        store byte 0xCC at cp866_boxes+0x60
        store byte 0xCD at cp866_boxes+0x50
        store byte 0xCE at cp866_boxes+0x6C
        store byte 0xCF at cp866_boxes+0x67

        store byte 0xD0 at cp866_boxes+0x68
        store byte 0xD1 at cp866_boxes+0x64
        store byte 0xD2 at cp866_boxes+0x65
        store byte 0xD3 at cp866_boxes+0x59
        store byte 0xD4 at cp866_boxes+0x58
        store byte 0xD5 at cp866_boxes+0x52
        store byte 0xD6 at cp866_boxes+0x53
        store byte 0xD7 at cp866_boxes+0x6B

        store byte 0xD8 at cp866_boxes+0x6A
        store byte 0xD9 at cp866_boxes+0x18
        store byte 0xDA at cp866_boxes+0x0C
        store byte 0xDB at cp866_boxes+0x88
        store byte 0xDC at cp866_boxes+0x84
        store byte 0xDD at cp866_boxes+0x8C
        store byte 0xDE at cp866_boxes+0x90
        store byte 0xDF at cp866_boxes+0x80
