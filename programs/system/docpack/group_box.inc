; SPDX-License-Identifier: GPL-2.0-only
;
; group_box - a frame with a bold title over its top line
; Copyright (C) 2026 KolibriOS team
;
; The frame is a line of work_graph and one of work_light inside it, as
; DrawFrame of the C-- programs draws it, with work inside them; its top
; lines cross the middle of the capitals of the title. The title is bold,
; work_text on work, GROUP_TITLE_PAD wider at both sides. An empty
; title leaves the top line whole.
;
; For the layout: the capitals of the title are GROUP_TITLE_ABOVE rows
; above the top line and GROUP_TITLE_BELOW rows from it down. The title
; must fit in the frame: title_x - GROUP_TITLE_PAD > 0 and
; title_x + letters*GROUP_BOLD_W + GROUP_TITLE_PAD < width.

; ====================================================================

struct GROUP_BOX
        x_width         dd ?            ; x shl 16 + width, as the kernel takes them
        y_height        dd ?            ; y shl 16 + height
        title           dd ?            ; UTF-8, zero terminated
        title_at        dd ?            ; x shl 16 + y of its letters, over the top line
        colors          dd ?            ; -> system_colors
ends

; a GROUP_BOX in a table: the frame, its title, the x of its letters from
; the left of the frame, the colors
macro group_box_row x, y, width, height, title, title_x, colors
{
        local   start
start:
        dd      (x) shl 16 + (width), (y) shl 16 + (height), title
        dd      (x + title_x) shl 16 + (y - GROUP_TITLE_DY), colors
        assert  $ - start = sizeof.GROUP_BOX
}

; the 8x16 font: its capitals are the rows 2..11, its letters have an
; empty column at both sides
FONT_CHAR_W       = 8
FONT_CHAR_H       = 16
FONT_CAPS_Y       = 2
FONT_CAPS_H       = 10

GROUP_LINES       = 2                           ; of the frame, the inside past them

; from the title down to the top line: its capitals centred on the lines,
; as many rows above them as below
GROUP_TITLE_DY    = FONT_CAPS_Y + (FONT_CAPS_H - GROUP_LINES)/2
GROUP_TITLE_ABOVE = GROUP_TITLE_DY - FONT_CAPS_Y
GROUP_TITLE_BELOW = FONT_CAPS_Y + FONT_CAPS_H - GROUP_TITLE_DY
assert GROUP_TITLE_ABOVE = GROUP_TITLE_BELOW - GROUP_LINES
GROUP_TITLE_PAD   = 7                           ; with the empty column of a letter: 8
GROUP_BOLD_W      = FONT_CHAR_W + 1             ; a bold letter, a pixel apart
GROUP_TITLE_FONT  = 0x30000000                  ; 8x16 UTF-8, esi letters

; ====================================================================

; edi -> GROUP_BOX; changes eax, ebx, ecx, edx, esi, ebp
group_box:
        ; the frame, GROUP_LINES: work_graph, work_light a pixel inside
        ; it, then work a pixel inside that; x + 1 and width - 2 in one
        ; add, y and height the same
        mov     esi, [edi + GROUP_BOX.colors]
        mov     ebx, [edi + GROUP_BOX.x_width]
        mov     ecx, [edi + GROUP_BOX.y_height]
        mcall   SF_DRAW_RECT, , , [esi + system_colors.work_graph]
        add     ebx, (1 shl 16) - 2
        add     ecx, (1 shl 16) - 2
        mcall   SF_DRAW_RECT, , , [esi + system_colors.work_light]
        add     ebx, (1 shl 16) - 2
        add     ecx, (1 shl 16) - 2
        mcall   SF_DRAW_RECT, , , [esi + system_colors.work]

        ; no title: the top line stays whole
        mov     edx, [edi + GROUP_BOX.title]
        cmp     byte [edx], 0
        je      .done

        ; its place on the work colour over the top line: its letters,
        ; GROUP_TITLE_PAD more at both sides, as high as the font
        mov     ebp, [edi + GROUP_BOX.title_at]
        mov     ebx, ebp
        and     ebx, 0xFFFF0000
        sub     ebx, GROUP_TITLE_PAD shl 16
        add     ebx, GROUP_TITLE_PAD*2

.width:
        add     ebx, GROUP_BOLD_W
        call    .next_letter
        cmp     byte [edx], 0
        jne     .width

        mov     ecx, ebp
        shl     ecx, 16
        add     ecx, FONT_CHAR_H
        mcall   SF_DRAW_RECT, , , [esi + system_colors.work]

        ; its letters in bold: each one twice, a pixel apart
        mov     ecx, [esi + system_colors.work_text]
        or      ecx, GROUP_TITLE_FONT
        mov     edx, [edi + GROUP_BOX.title]
        mov     ebx, ebp
        mov     esi, 1

.letter:
        mcall   SF_DRAW_TEXT
        add     ebx, 1 shl 16
        mcall   SF_DRAW_TEXT
        add     ebx, (GROUP_BOLD_W - 1) shl 16
        call    .next_letter
        cmp     byte [edx], 0
        jne     .letter

.done:
        ret

; edx -> a letter of UTF-8 text: edx -> the next one, past the bytes
; 10xxxxxx of this one; changes al
.next_letter:
        inc     edx
        mov     al, [edx]
        and     al, 0xC0
        cmp     al, 0x80
        je      .next_letter

        ret
