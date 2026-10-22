; SPDX-License-Identifier: GPL-2.0-only
;
; group_box - a frame with a title over its top or its bottom line
; Copyright (C) 2026 KolibriOS team
;
; The frame is a line of work_graph and one of work_light inside it, as
; DrawFrame of the C-- programs draws it, the lines only: what is inside
; is the program's, not drawn over. Its top lines, or with GROUP_BOTTOM
; its bottom ones, cross the middle of the capitals of the title. The
; title is work_text on work, GROUP_TITLE_PAD
; wider at both sides, in a font of function 4, regular or bold (each
; letter twice, a pixel apart). An empty title leaves the lines whole.
; The colours are those of a system_colors, of the system or the
; program's own.
;
; Called as the widgets of box_lib: push the GROUP_BOX, call
; group_box_draw; it keeps all the registers and takes the parameter off
; the stack. It keeps nothing of its own between calls, so threads may
; draw at the same time.
;
; The program sets GROUP_USES, the GROUP_USE_* its groups use (all of
; them, GROUP_USES_ALL, for a library), anywhere, as a constant:
; only their code is assembled, and the style of a group is read at run
; time only where the program uses more than one. A style it does not
; use is not drawn right.
;
; For the layout: the capitals of the 8x16 title are GROUP_TITLE_ABOVE
; rows above the top line and GROUP_TITLE_BELOW rows from it down. The
; title must fit in the frame.

; ====================================================================

; the style of a group: a font as function 4 takes it, and the flags
GROUP_FONT_6X9    = 0x00000000                  ; cp866
GROUP_FONT_CP866  = 0x10000000                  ; 8x16
GROUP_FONT_UTF16  = 0x20000000
GROUP_FONT_UTF8   = 0x30000000
GROUP_FONT_MASK   = 0x30000000
GROUP_FONT_SHIFT  = 28
GROUP_BOLD        = 1
GROUP_BOTTOM      = 2                           ; the title over the bottom lines

; what the program uses: the fonts by their numbers, the weights, the
; places of the title
GROUP_USE_6X9     = 1 shl (GROUP_FONT_6X9 shr GROUP_FONT_SHIFT)
GROUP_USE_CP866   = 1 shl (GROUP_FONT_CP866 shr GROUP_FONT_SHIFT)
GROUP_USE_UTF16   = 1 shl (GROUP_FONT_UTF16 shr GROUP_FONT_SHIFT)
GROUP_USE_UTF8    = 1 shl (GROUP_FONT_UTF8 shr GROUP_FONT_SHIFT)
GROUP_USE_REGULAR = 0x10
GROUP_USE_BOLD    = 0x20
GROUP_USE_TOP     = 0x40
GROUP_USE_BOTTOM  = 0x80
GROUP_USES_ALL    = 0xFF

struct GROUP_BOX
        x_width         dd ?            ; x shl 16 + width, as the kernel takes them
        y_height        dd ?            ; y shl 16 + height
        title           dd ?            ; zero terminated, in the encoding of the font
        title_x         dd ?            ; from the left of the frame to its letters
        colors          dd ?            ; -> system_colors
        style           dd ?            ; GROUP_FONT_* + GROUP_BOLD + GROUP_BOTTOM
ends

; a GROUP_BOX in a table: the frame, its title, the colors, the style,
; and the x of the letters from the left of the frame, GROUP_TITLE_X if it
; is left out; the style must be one the program uses
macro group_box_row x, y, width, height, title, colors, style, title_x
{
        local   start, use
start:
        dd      (x) shl 16 + (width), (y) shl 16 + (height), title
        if title_x eq
                dd      GROUP_TITLE_X
        else
                dd      title_x
        end if
        dd      colors, style
        assert  $ - start = sizeof.GROUP_BOX

        use = 1 shl (((style) and GROUP_FONT_MASK) shr GROUP_FONT_SHIFT)
        if (style) and GROUP_BOLD
                use = use + GROUP_USE_BOLD
        else
                use = use + GROUP_USE_REGULAR
        end if
        if (style) and GROUP_BOTTOM
                use = use + GROUP_USE_BOTTOM
        else
                use = use + GROUP_USE_TOP
        end if
        assert  use and GROUP_USES = use
}

; the fonts: 6x9 the only small one, its capitals are its rows 0..6; the
; capitals of 8x16 are its rows 2..11
SMALL_CHAR_W      = 6
SMALL_CHAR_H      = 9
SMALL_CAPS_Y      = 0
SMALL_CAPS_H      = 7
FONT_CHAR_W       = 8
FONT_CHAR_H       = 16
FONT_CAPS_Y       = 2
FONT_CAPS_H       = 10

GROUP_FRAME_LINES = 2                           ; of the frame, the inside past them

; from the title down to its lines: its capitals centred on them, as
; many rows above them as below; in 6x9 an odd number is left, the extra
; row above them
GROUP_TITLE_DY    = FONT_CAPS_Y + (FONT_CAPS_H - GROUP_FRAME_LINES)/2
GROUP_TITLE_ABOVE = GROUP_TITLE_DY - FONT_CAPS_Y
GROUP_TITLE_BELOW = FONT_CAPS_Y + FONT_CAPS_H - GROUP_TITLE_DY
assert GROUP_TITLE_ABOVE = GROUP_TITLE_BELOW - GROUP_FRAME_LINES
GROUP_SMALL_TITLE_DY = SMALL_CAPS_Y + (SMALL_CAPS_H - GROUP_FRAME_LINES + 1)/2
GROUP_TITLE_PAD   = 7                           ; with the empty column of a letter: 8

; the letters of the title by default as in CPUINFO: GROUP_TITLE_GAP from
; the lines with the empty column of a letter
GROUP_TITLE_GAP   = 12
GROUP_TITLE_X     = GROUP_FRAME_LINES + GROUP_TITLE_GAP - 1
GROUP_ASCIIZ      = 0x80000000                  ; function 4: up to the zero

; ====================================================================

; what the groups of the program have, GROUP_HAS_* 1 or 0, and how many
; of a kind: more than one is read from the style at run time
assert GROUP_USES and GROUP_USES_ALL = GROUP_USES

GROUP_HAS_BYTES   = 0                   ; a byte a letter: 6x9, cp866
GROUP_HAS_UTF16   = 0
GROUP_HAS_UTF8    = 0
GROUP_HAS_SMALL   = 0
GROUP_HAS_BIG     = 0
GROUP_HAS_REGULAR = 0
GROUP_HAS_BOLD    = 0
GROUP_HAS_TOP     = 0
GROUP_HAS_BOTTOM  = 0
if GROUP_USES and (GROUP_USE_6X9 + GROUP_USE_CP866)
        GROUP_HAS_BYTES = 1
end if
if GROUP_USES and GROUP_USE_UTF16
        GROUP_HAS_UTF16 = 1
end if
if GROUP_USES and GROUP_USE_UTF8
        GROUP_HAS_UTF8 = 1
end if
if GROUP_USES and GROUP_USE_6X9
        GROUP_HAS_SMALL = 1
end if
if GROUP_USES and (GROUP_USE_CP866 + GROUP_USE_UTF16 + GROUP_USE_UTF8)
        GROUP_HAS_BIG = 1
end if
if GROUP_USES and GROUP_USE_REGULAR
        GROUP_HAS_REGULAR = 1
end if
if GROUP_USES and GROUP_USE_BOLD
        GROUP_HAS_BOLD = 1
end if
if GROUP_USES and GROUP_USE_TOP
        GROUP_HAS_TOP = 1
end if
if GROUP_USES and GROUP_USE_BOTTOM
        GROUP_HAS_BOTTOM = 1
end if
GROUP_ENCODINGS   = GROUP_HAS_BYTES + GROUP_HAS_UTF16 + GROUP_HAS_UTF8
GROUP_SIZES       = GROUP_HAS_SMALL + GROUP_HAS_BIG
GROUP_WEIGHTS     = GROUP_HAS_REGULAR + GROUP_HAS_BOLD
GROUP_PLACES      = GROUP_HAS_TOP + GROUP_HAS_BOTTOM

; the font, the letters and the place of the title when all the groups
; have one size, one weight or one font
if GROUP_HAS_BIG
        GROUP_ONE_W  = FONT_CHAR_W
        GROUP_ONE_H  = FONT_CHAR_H
        GROUP_ONE_DY = GROUP_TITLE_DY
else
        GROUP_ONE_W  = SMALL_CHAR_W
        GROUP_ONE_H  = SMALL_CHAR_H
        GROUP_ONE_DY = GROUP_SMALL_TITLE_DY
end if
if GROUP_HAS_BOLD & ~ GROUP_HAS_REGULAR
        GROUP_ALL_BOLD_PIXEL = 1                ; a pixel wider: all are bold
else
        GROUP_ALL_BOLD_PIXEL = 0
end if
if GROUP_HAS_UTF16
        GROUP_ONE_FONT = GROUP_FONT_UTF16
else if GROUP_HAS_UTF8
        GROUP_ONE_FONT = GROUP_FONT_UTF8
else if GROUP_HAS_SMALL
        GROUP_ONE_FONT = GROUP_FONT_6X9
else
        GROUP_ONE_FONT = GROUP_FONT_CP866
end if

; ====================================================================

; stdcall group_box_draw, box: the GROUP_BOX at box; keeps all the
; registers
group_box_draw:
        pushad
        mov     edi, [esp + 8*4 + 4]    ; past the registers and the return
        mov     esi, [edi + GROUP_BOX.colors]
        mov     ebx, [edi + GROUP_BOX.x_width]
        mov     ecx, [edi + GROUP_BOX.y_height]

        ; the frame: work_graph, work_light a pixel inside it; x + 1 and
        ; width - 2 in one add, y and height the same
        mov     edx, [esi + system_colors.work_graph]
        call    .ring
        add     ebx, (1 shl 16) - 2
        add     ecx, (1 shl 16) - 2
        mov     edx, [esi + system_colors.work_light]
        call    .ring

        ; the title; none: the lines stay whole
        mov     edx, [edi + GROUP_BOX.title]
        call    .at_end
        je      .exit

        ; the letters of its font: ebp = their step, a pixel wider in
        ; bold; eax = their height; esi = the rows from them down to
        ; their lines
        mov     ebp, GROUP_ONE_W + GROUP_ALL_BOLD_PIXEL
        mov     eax, GROUP_ONE_H
        mov     esi, GROUP_ONE_DY
if GROUP_SIZES > 1
        test    dword [edi + GROUP_BOX.style], GROUP_FONT_MASK
        jnz     .weight
        mov     ebp, SMALL_CHAR_W + GROUP_ALL_BOLD_PIXEL
        mov     eax, SMALL_CHAR_H
        mov     esi, GROUP_SMALL_TITLE_DY

.weight:
end if
if GROUP_WEIGHTS > 1
        test    dword [edi + GROUP_BOX.style], GROUP_BOLD
        jz      .place
        inc     ebp

.place:
end if

        ; ebx = the place of the letters: title_x from the left of the
        ; frame, over its top lines or its bottom ones; kept for them
        mov     ebx, [edi + GROUP_BOX.x_width]
        shr     ebx, 16
        add     ebx, [edi + GROUP_BOX.title_x]
        shl     ebx, 16
        mov     ecx, [edi + GROUP_BOX.y_height]
        shr     ecx, 16                 ; the top lines
if GROUP_PLACES > 1
        test    dword [edi + GROUP_BOX.style], GROUP_BOTTOM
        jz      .lines
end if
if GROUP_HAS_BOTTOM
        movzx   edx, word [edi + GROUP_BOX.y_height]
        add     ecx, edx                ; the bottom ones
        sub     ecx, GROUP_FRAME_LINES

.lines:
end if
        sub     ecx, esi
        add     ebx, ecx
        push    ebx

        ; their place on the work colour: as high as the font, the letters
        ; and GROUP_TITLE_PAD more at both sides: x - PAD and PAD * 2 in
        ; one add, then the letters
        mov     ecx, ebx
        shl     ecx, 16
        add     ecx, eax
        and     ebx, 0xFFFF0000
        add     ebx, (-GROUP_TITLE_PAD shl 16) + GROUP_TITLE_PAD*2
        mov     edx, [edi + GROUP_BOX.title]

.width:
        add     ebx, ebp
        call    .next_letter
        jne     .width

        mov     esi, [edi + GROUP_BOX.colors]
        mcall   SF_DRAW_RECT, , , [esi + system_colors.work]

        ; the letters in its font
        mov     ecx, [esi + system_colors.work_text]
if GROUP_ENCODINGS > 1 | GROUP_SIZES > 1
        mov     eax, [edi + GROUP_BOX.style]
        and     eax, GROUP_FONT_MASK
        or      ecx, eax
else
        or      ecx, GROUP_ONE_FONT
end if
        mov     edx, [edi + GROUP_BOX.title]
        pop     ebx
if GROUP_WEIGHTS > 1
        test    dword [edi + GROUP_BOX.style], GROUP_BOLD
        jnz     .bold
end if

if GROUP_HAS_REGULAR
        ; regular: in one call
        or      ecx, GROUP_ASCIIZ
        mcall   SF_DRAW_TEXT
if GROUP_HAS_BOLD
        jmp     .exit
end if
end if

if GROUP_HAS_BOLD
        ; bold: each letter twice, a pixel apart
.bold:
        mov     esi, 1                  ; a letter
        dec     ebp                     ; from the second one to the next letter
        shl     ebp, 16

.letter:
        mcall   SF_DRAW_TEXT
        add     ebx, 1 shl 16
        mcall   SF_DRAW_TEXT
        add     ebx, ebp
        call    .next_letter
        jne     .letter
end if

.exit:
        popad
        ret     4

; ebx = x shl 16 + width, ecx = y shl 16 + height, edx = the colour: a
; line of a pixel round the box; keeps ebx, ecx
.ring:
        pushad
        mov     esi, ebx
        mov     edi, ecx

        ; the top and the bottom line
        and     ecx, 0xFFFF0000
        inc     ecx
        mcall   SF_DRAW_RECT
        movzx   eax, di
        dec     eax
        shl     eax, 16
        add     ecx, eax
        mcall   SF_DRAW_RECT

        ; the left and the right line
        mov     ecx, edi
        and     ebx, 0xFFFF0000
        inc     ebx
        mcall   SF_DRAW_RECT
        movzx   eax, si
        dec     eax
        shl     eax, 16
        add     ebx, eax
        mcall   SF_DRAW_RECT
        popad
        ret

; edx -> a letter of the title: edx -> the next one: a byte on, a word in
; UTF-16, past the bytes 10xxxxxx in UTF-8; then as .at_end
.next_letter:
if GROUP_ENCODINGS > 1
        mov     eax, [edi + GROUP_BOX.style]
        and     eax, GROUP_FONT_MASK
end if
if GROUP_HAS_UTF16 & GROUP_ENCODINGS > 1
        cmp     eax, GROUP_FONT_UTF16
        je      .next_word
end if
if GROUP_HAS_UTF8 & GROUP_ENCODINGS > 1
        cmp     eax, GROUP_FONT_UTF8
        je      .next_utf8
end if
if GROUP_HAS_BYTES
        inc     edx
end if
if GROUP_HAS_BYTES & (GROUP_HAS_UTF16 | GROUP_HAS_UTF8)
        jmp     .at_end
end if
if GROUP_HAS_UTF16
.next_word:
        add     edx, 2
end if
if GROUP_HAS_UTF16 & GROUP_HAS_UTF8
        jmp     .at_end
end if
if GROUP_HAS_UTF8
.next_utf8:
        inc     edx
        mov     al, [edx]
        and     al, 0xC0
        cmp     al, 0x80
        je      .next_utf8
end if

; edx -> a letter of the title: ZF = 1 at the zero that ends it, a word
; of it in UTF-16; changes eax
.at_end:
if GROUP_HAS_UTF16 & GROUP_ENCODINGS > 1
        mov     eax, [edi + GROUP_BOX.style]
        and     eax, GROUP_FONT_MASK
        cmp     eax, GROUP_FONT_UTF16
        je      .at_end_word
end if
if GROUP_HAS_BYTES | GROUP_HAS_UTF8
        cmp     byte [edx], 0
        ret
end if
if GROUP_HAS_UTF16
.at_end_word:
        cmp     word [edx], 0
        ret
end if
