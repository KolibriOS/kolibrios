; SPDX-License-Identifier: NOASSERTION
;
; End - the shutdown dialog of KolibriOS
;
; Authors: Veliant, Leency, Heavyiron, Burer
;
; Restarts the kernel, reboots or powers off, saving the RAM-disk first to
; the target of rdsave/save.inc if the checkbox is on; without a target or
; if the save fails, RDsave asks and does the shutdown. Drawn as a window
; of the current skin, its caption big and without buttons.

; ====================================================================

include "../../macros.inc"
include "../../KOSfuncs.inc"
include "../../encoding.inc"

format meos KX
entry START
stack 4096

include "../../proc32.inc"

; ====================================================================

; the kernel has made the heap, linking libini
START:
        ; the skin of system.ini: the colours of the frame of an active
        ; window and the base bitmap of its caption; the kernel unpacks a
        ; packed skin file
        invoke  ini_get_str, system_ini, sec_style, key_skin, fname_buf, PATH_MAX, no_skin
        mcall   SF_SYS_MISC, SSF_LOAD_FILE, fname_buf
        test    eax, eax
        jz      .window
        cmp     dword [eax + SKIN_IDENT], SKIN_SIGN
        jne     .window

        mov     ebx, [eax + SKIN_PARAMS]
        add     ebx, eax
        mov     ecx, [ebx + PARAM_OUTER]
        mov     [frame_outer], ecx
        mov     ecx, [ebx + PARAM_FRAME]
        mov     [frame_middle], ecx
        mov     ecx, [ebx + PARAM_INNER]
        mov     [frame_inner], ecx

        mov     ebx, [eax + SKIN_BITMAPS]
        add     ebx, eax

.bitmap:
        cmp     dword [ebx], 0
        je      .window
        cmp     word [ebx + BITMAP_KIND], BITMAP_BASE
        jne     .next_bitmap
        cmp     word [ebx + BITMAP_TYPE], BITMAP_INACTIVE
        jne     .base

.next_bitmap:
        add     ebx, BITMAP_ENTRY
        jmp     .bitmap

.base:
        mov     ecx, [ebx + BITMAP_DATA]
        add     ecx, eax
        mov     [skin_bitmap], ecx

        ; the kept target, shown in the label of the checkbox
.window:
        call    rd_read_target
        call    draw_window
        mcall   SF_SYSTEM, SSF_WINDOW_BEHAVIOR, SSSF_SET_WB, -1, ZPOS_ALWAYS_TOP
        jmp     still

redraw:
        call    draw_window

; ====================================================================

still:
        mcall   SF_WAIT_EVENT

        cmp     al, EV_REDRAW
        je      redraw
        cmp     al, EV_KEY
        je      key
        cmp     al, EV_BUTTON
        je      button

        jmp     still

; ====================================================================

key:
        mcall   SF_GET_KEY

        cmp     ah, KEY_ESC
        je      exit
        cmp     ah, KEY_CTRL_S
        je      toggle_autosave

        mov     edi, buttons

.next:
        cmp     ah, [edi + BUTTON.key]
        je      .found

        add     edi, sizeof.BUTTON
        cmp     edi, buttons_end
        jb      .next

        jmp     still

.found:
        movzx   eax, [edi + BUTTON.id]
        mov     [action], eax
        jmp     shutdown

; ====================================================================

button:
        mcall   SF_GET_BUTTON

        cmp     ah, BTN_CANCEL
        jb      still                           ; SF 17: eax = 1, no button
        je      exit
        cmp     ah, BTN_AUTOSAVE
        je      toggle_autosave

        movzx   eax, ah
        mov     [action], eax

; [action] = SSSF_SHUTDOWN_*
shutdown:
        cmp     [autosave], 0
        je      .off

        cmp     [target], TARGET_NONE
        je      .ask

        mov     edx, sz_saving
        call    draw_label

        call    rd_save
        test    eax, eax
        jz      .off

        ; not saved: say so, RDsave tells why on its own save
        mcall   SF_FILE, notify_run

        ; RDsave asks for the target and does the shutdown; this window
        ; away, as RDsave would open under it
.ask:
        mov     eax, [action]
        add     al, '0'
        mov     [rdsave_then.action], al
        mcall   SF_FILE, rdsave_run
        test    eax, eax
        jns     exit

        ; RDsave did not start: the window stays; not saved, said already
        ; after a failed save
        cmp     [target], TARGET_NONE
        jne     redraw
        mcall   SF_FILE, notify_run
        jmp     redraw

.off:
        mcall   SF_SYSTEM, SSF_SHUTDOWN, [action]

exit:
        mcall   SF_TERMINATE_PROCESS

; the checkbox is a button, so all of it again
toggle_autosave:
        xor     [autosave], 1
        jmp     redraw

; ====================================================================

include "../rdsave/save.inc"

; ====================================================================

draw_window:
        mcall   SF_REDRAW, SSF_BEGIN_DRAW
        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, sizeof.system_colors

        call    make_colors

        ; the window in the centre of the screen; unmovable, so the kernel
        ; neither drags nor rolls it up
        mcall   SF_GET_SCREEN_SIZE      ; (width - 1) shl 16 + height - 1
        movzx   ecx, ax
        sub     ecx, DIALOG_H - 1
        shr     ecx, 1
        shl     ecx, 16
        add     ecx, DIALOG_H - 1
        shr     eax, 16
        sub     eax, DIALOG_W - 1
        shr     eax, 1
        shl     eax, 16
        lea     ebx, [eax + DIALOG_W - 1]
        mcall   SF_CREATE_WINDOW, , , WINDOW_STYLE, WINDOW_UNMOVABLE

        ; the frame of the skin: outer round all and under the header, then
        ; under the header three lines of frame and one of inner
        mov     ebx, DIALOG_W
        mov     ecx, DIALOG_H
        mov     edx, [frame_outer]
        call    frame
        mcall   SF_DRAW_RECT, <1, HEADER_W>, <HEADER_LINE_Y, 1>

        mov     ebx, (1 shl 16) + HEADER_W
        mov     ecx, ((HEADER_LINE_Y + 1) shl 16) + DIALOG_H - HEADER_LINE_Y - 2
        mov     edx, [frame_middle]
        call    ring
        call    ring
        call    ring
        ; an inner line in the colour of the frame would not show: work_graph
        mov     edx, [frame_inner]
        cmp     edx, [frame_middle]
        jne     .inner
        mov     edx, [sc.work_graph]

.inner:
        call    ring

        mcall   SF_DRAW_RECT, , , [sc.work] ; the box inside the frame

        call    draw_header

        mov     ecx, [sc.grab_text]
        or      ecx, FONT_TITLE_LETTERS
        mov     ebx, (TITLE_X shl 16) + TITLE_TEXT_Y
        mov     edx, sz_title
        mov     ebp, (2*FONT_CHAR_W) shl 16
        call    draw_bold

        call    draw_checkbox
        call    draw_target

        ; the label clicks as the box
        mcall   SF_DEFINE_BUTTON, <LABEL_X, LABEL_W>, <CHECK_ROW_Y, ROW_H>, BTN_AUTOSAVE + BT_HIDE + BT_NOFRAME

        mov     edi, buttons

.button:
        mov     ebx, [edi + BUTTON.x_w]
        mov     ecx, (BTN_Y shl 16) + BTN_H
        movzx   edx, [edi + BUTTON.id]
        mov     esi, [edi + BUTTON.color]
        mov     esi, [esi]
        call    button_box

        mov     ecx, [sc.work_button_text]
        or      ecx, FONT_LETTERS
        mov     ebp, FONT_CHAR_W shl 16
        mov     ebx, [edi + BUTTON.caption_x_y]
        mov     edx, [edi + BUTTON.caption]
        call    draw_bold

        ; the hotkey regular and softer
        mov     eax, [sc.work_button_text]
        mov     edx, [edi + BUTTON.color]
        mov     edx, [edx]
        call    soften
        or      ecx, FONT
        mcall   SF_DRAW_TEXT, [edi + BUTTON.hotkey_x_y], , [edi + BUTTON.hotkey]

        add     edi, sizeof.BUTTON
        cmp     edi, buttons_end
        jb      .button

        ; Cancel under them over their whole width
        mov     ebx, (INNER shl 16) + CANCEL_W
        mov     ecx, (CANCEL_Y shl 16) + ROW_H
        mov     edx, BTN_CANCEL
        mov     esi, [sc.work]
        call    button_box
        mov     eax, [sc.work_text]
        mov     edx, [sc.work]
        call    soften
        or      ecx, FONT
        mcall   SF_DRAW_TEXT, <INNER + (CANCEL_W - sz_cancel.chars*FONT_CHAR_W)/2, CANCEL_Y + TEXT_DY>, , sz_cancel

        mcall   SF_REDRAW, SSF_END_DRAW

        ret

; the colours of the shutdown buttons, as in TaskMan, from the channels
; of work_button, h the brightest one: [blue] = its channels sorted, the
; brightest in blue, the darkest in red; [green] = h/2, h, h/2 and [red]
; = h, h/2, h/2 (red, green, blue)
make_colors:
        mov     eax, [sc.work_button]   ; blue, green, red -> al, ah, dl
        mov     edx, eax
        shr     edx, 16

        ; sort them: the brightest in al, the darkest in dl
        cmp     al, ah
        jae     .sorted1
        xchg    al, ah

.sorted1:
        cmp     al, dl
        jae     .sorted2
        xchg    al, dl

.sorted2:
        cmp     ah, dl
        jae     .sorted3
        xchg    ah, dl

.sorted3:
        movzx   ecx, dl
        shl     ecx, 16
        mov     cx, ax
        mov     [blue], ecx

        ; h/2 in every channel, then h in green, in red
        mov     dl, al
        shr     dl, 1
        movzx   ecx, dl
        imul    ecx, 0x10101
        mov     [green], ecx
        mov     byte [green + 1], al
        mov     [red], ecx
        mov     byte [red + 2], al
        ret

; the header inside the outer line, as a caption: the base bitmap of the
; caption of the skin repeated across and stretched down, a row at a
; time; work_dark if it was not read
draw_header:
        cmp     [skin_bitmap], 0
        jne     .skin
        mcall   SF_DRAW_RECT, <1, HEADER_W>, <1, HEADER_H>, [sc.work_dark]
        ret

.skin:
        xor     ebp, ebp                ; the row of the header

        ; its row of the bitmap, past the rows that look as a frame
.row:
        mov     ebx, [skin_bitmap]
        mov     eax, ebp
        mov     ecx, [ebx + BITMAP_H]
        sub     ecx, SKIN_TOP + SKIN_BOTTOM
        mul     ecx
        mov     ecx, HEADER_H
        div     ecx
        add     eax, SKIN_TOP
        imul    eax, [ebx + BITMAP_W]
        imul    eax, BGR                ; bytes a pixel
        add     eax, ebx
        add     eax, BITMAP_PIXELS
        mov     edi, row_buf
        mov     edx, HEADER_W

.tile:
        mov     esi, eax
        mov     ecx, [ebx + BITMAP_W]

.pixel:
        movsw
        movsb
        dec     edx
        jz      .put
        loop    .pixel
        jmp     .tile

.put:
        mov     edx, ebp                ; x = 1, y = 1 + the row
        add     edx, (1 shl 16) + 1
        mcall   SF_PUT_IMAGE, row_buf, <HEADER_W, 1>
        inc     ebp
        cmp     ebp, HEADER_H
        jb      .row
        ret

; ebx = x shl 16 + y, ecx = the colour and a font of letters, edx -> UTF-8
; text, ebp = the width of a letter shl 16: bold, each letter twice a
; pixel apart and the next one a pixel further; keeps ecx, ebp
draw_bold:
        mov     esi, 1                  ; a letter

.letter:
        mcall   SF_DRAW_TEXT
        add     ebx, 1 shl 16
        mcall
        add     ebx, ebp

.skip:
        inc     edx
        mov     al, [edx]
        and     al, UTF8_TAIL_MASK
        cmp     al, UTF8_TAIL
        je      .skip

        cmp     byte [edx], 0
        jne     .letter

        ret

; the label of the checkbox: sz_save, with a kept target sz_save_to and
; the target in cp866, floppy A or the file, its end if it is longer than
; the place; without a target RDsave asks
draw_target:
        mov     edx, sz_save
        cmp     [target], TARGET_NONE
        jne     .target
        call    draw_label
        ret

.target:
        mov     edx, sz_save_to
        call    draw_label

        mov     edx, sz_floppy
        cmp     [target], TARGET_FLOPPY
        je      .length
        mov     edx, fname_buf

        ; esi = the length of the name at edx, up to NAME_CHARS from its end
.length:
        mov     esi, edx

.byte:
        cmp     byte [esi], 0
        je      .end
        inc     esi
        jmp     .byte

.end:
        sub     esi, edx
        cmp     esi, NAME_CHARS
        jbe     .draw
        add     edx, esi
        sub     edx, NAME_CHARS
        mov     esi, NAME_CHARS

.draw:
        mov     ecx, [sc.work_text]
        or      ecx, FONT_CP866
        mcall   SF_DRAW_TEXT, <NAME_X, CHECK_ROW_Y + TEXT_DY>
        ret

; edx -> a cp866 text: drawn in the place of the checkbox label
draw_label:
        push    edx
        mcall   SF_DRAW_RECT, <LABEL_X, LABEL_W>, <CHECK_ROW_Y, ROW_H>, [sc.work]
        pop     edx
        mov     ecx, [sc.work_text]
        or      ecx, FONT_LABEL
        mcall   SF_DRAW_TEXT, <LABEL_X, CHECK_ROW_Y + TEXT_DY>
        ret

; set: a button with a tick; not set: a sunken field under a hidden button
draw_checkbox:
        mov     ebx, (CHECK_X shl 16) + CHECK_SIZE
        mov     ecx, (CHECK_Y shl 16) + CHECK_SIZE

        cmp     [autosave], 0
        jne     .set

        mov     edx, [sc.work_graph]
        call    sunken_box

        ; inside the frame, the kernel inverts its press frame on the field
        mcall   SF_DEFINE_BUTTON, <CHECK_X + 1, CHECK_SIZE - 3>, <CHECK_Y + 1, CHECK_SIZE - 3>, BTN_AUTOSAVE + BT_HIDE
        ret

.set:
        mov     edx, BTN_AUTOSAVE
        mov     esi, [sc.work_button]
        call    button_box

        ; the tick: two strokes, each two lines a pixel apart
        mov     edx, [sc.work_button_text]
        mcall   SF_DRAW_LINE, <TICK_X1, TICK_X2>, <TICK_Y1,     TICK_Y2>
        mcall               ,                     <TICK_Y1 - 1, TICK_Y2 - 1>
        mcall               , <TICK_X2, TICK_X3>, <TICK_Y2,     TICK_Y3>
        mcall               ,                     <TICK_Y2 - 1, TICK_Y3 - 1>
        ret

; ====================================================================

; eax = a text colour, edx = its background: ecx = the text a quarter of
; the way to the background, text/2 + text/4 + background/4
soften:
        mov     ecx, eax
        shr     ecx, 1
        and     ecx, HALF_MASK
        shr     eax, 2
        and     eax, QUARTER_MASK
        add     ecx, eax
        shr     edx, 2
        and     edx, QUARTER_MASK
        add     ecx, edx
        ret

; ebx = x shl 16 + width, ecx = y shl 16 + height, edx = the id, esi =
; the colour: a kernel button with a square frame, half its colour, over
; the corners the kernel leaves out; edx = that colour, keeps ebx, ecx
button_box:
        dec     ebx                     ; the button is a pixel bigger than given
        dec     ecx
        mcall   SF_DEFINE_BUTTON
        inc     ebx
        inc     ecx
        mov     edx, esi
        shr     edx, 1
        and     edx, HALF_MASK
        jmp     frame

; ebx = x shl 16 + width, ecx = y shl 16 + height, edx = the colour of the
; frame: a field, the frame then sunken inside: three boxes, each 1 pixel
; smaller, leave darker lines left and top and lighter right and bottom
sunken_box:
        call    frame
        add     ebx, (1 shl 16) - 2
        add     ecx, (1 shl 16) - 2
        mov     edx, [sc.work_light]
        mov     eax, edx
        shr     eax, 3
        and     eax, SHADE_MASK
        sub     edx, eax                ; 1/8 darker
        mcall   SF_DRAW_RECT
        mov     edx, [sc.work_light]
        or      edx, SHADE_MASK         ; lighter
        add     ebx, (1 shl 16) - 1
        add     ecx, (1 shl 16) - 1
        mcall
        dec     ebx
        dec     ecx
        mcall   , , , [sc.work_light]
        ret

; as open_frame, then ebx, ecx the box a pixel inside but at the top
ring:
        call    open_frame
        add     ebx, (1 shl 16) - 2
        dec     ecx
        ret

; ebx = x shl 16 + width, ecx = y shl 16 + height, edx = the colour: a
; line of a pixel round the box; keeps ebx, ecx, edx
frame:
        push    ecx
        mov     cx, 1
        mcall   SF_DRAW_RECT            ; the top
        pop     ecx
        jmp     open_frame

; as frame, but the top
open_frame:
        push    ecx
        movzx   eax, cx
        dec     eax
        shl     eax, 16
        add     ecx, eax
        mov     cx, 1
        mcall   SF_DRAW_RECT            ; the bottom
        pop     ecx

        push    ebx
        mov     bx, 1
        mcall   SF_DRAW_RECT            ; the left
        movzx   eax, word [esp]
        dec     eax
        shl     eax, 16
        add     ebx, eax
        mcall   SF_DRAW_RECT            ; the right
        pop     ebx
        ret

; ====================================================================

; layout of the window: GAP from what is seen, the capitals and the
; boxes, to the next and to the frame; BTN_W is after the captions
GAP         = 16
ROW_H       = 24
TEXT_DY     = (ROW_H - FONT_CAPS_H)/2 - FONT_CAPS_Y ; the capitals centred in a row
FRAME       = 5                                 ; as a window of the skin
INNER       = FRAME + GAP

; the header inside the outer line, as the caption of a window, a line
; of outer under it: the capitals of the title in the double font GAP
; from both lines
TITLE_CAPS_H = 2*FONT_CAPS_H
HEADER_W    = DIALOG_W - 2
HEADER_H    = GAP + TITLE_CAPS_H + GAP
HEADER_LINE_Y = 1 + HEADER_H
TITLE_W     = sz_title.chars*(2*FONT_CHAR_W + 1) - 1
TITLE_X     = (DIALOG_W - TITLE_W)/2
TITLE_TEXT_Y = 1 + GAP - 2*FONT_CAPS_Y

; the checkbox row, the box GAP under the header: the label at the left
; edge, the box at the right one
CHECK_SIZE  = 15
CHECK_X     = DIALOG_W - INNER - CHECK_SIZE
CHECK_Y     = HEADER_LINE_Y + 1 + GAP
CHECK_ROW_Y = CHECK_Y - (ROW_H - CHECK_SIZE)/2
LABEL_X     = INNER
LABEL_W     = CHECK_X - FONT_CHAR_W - LABEL_X

; the tick, 9 x 7 pixels centred in the box: from TICK_X1, TICK_Y1 down
; to TICK_X2, TICK_Y2, then up to TICK_X3, TICK_Y3
TICK_X      = CHECK_X + (CHECK_SIZE - 9)/2
TICK_Y      = CHECK_Y + (CHECK_SIZE - 7)/2
TICK_X1     = TICK_X
TICK_X2     = TICK_X + 3
TICK_X3     = TICK_X + 8
TICK_Y1     = TICK_Y + 3
TICK_Y2     = TICK_Y + 6
TICK_Y3     = TICK_Y + 1

; the shutdown buttons GAP under the box: the caption over its hotkey,
; each line centred, the capitals of the two centred together
BTN_Y       = CHECK_Y + CHECK_SIZE + GAP
BTN_H       = 48
LINES_H     = FONT_CHAR_H + FONT_CAPS_H
CAPTION_Y   = BTN_Y + (BTN_H - LINES_H)/2 - FONT_CAPS_Y
BTN_COL1    = INNER
BTN_COL2    = BTN_COL1 + BTN_W + GAP
BTN_COL3    = BTN_COL2 + BTN_W + GAP

; Cancel GAP under the buttons
CANCEL_Y    = BTN_Y + BTN_H + GAP
CANCEL_W    = DIALOG_W - 2*INNER

; the window
DIALOG_W    = BTN_COL3 + BTN_W + INNER
DIALOG_H    = CANCEL_Y + ROW_H + INNER
WINDOW_STYLE = 0x01000000                       ; no skin, drawn here
WINDOW_UNMOVABLE = 0x01000000
ZPOS_ALWAYS_TOP = 1

; text; the capitals of 8x16 are its rows 2..11
FONT        = 0xB0000000                        ; zero terminated, 8x16 UTF-8
FONT_CP866  = 0x10000000                        ; 8x16 cp866, esi characters
FONT_LABEL  = 0x90000000                        ; zero terminated, 8x16 cp866: the
                                                ; texts of the checkbox label, as the path
FONT_LETTERS = 0x30000000                       ; 8x16 UTF-8, esi letters
FONT_TITLE_LETTERS = 0x31000000                 ; the same, double size
BOLD_W      = FONT_CHAR_W + 1                   ; a bold letter, a pixel apart
UTF8_TAIL_MASK = 0xC0
UTF8_TAIL   = 0x80                              ; 10xxxxxx goes on with a letter
FONT_CHAR_W = 8
FONT_CHAR_H = 16
FONT_CAPS_Y = 2
FONT_CAPS_H = 10

; colours
SHADE_MASK   = 0x1F1F1F                         ; an eighth of each channel
HALF_MASK    = 0x7F7F7F                         ; a half of each channel
QUARTER_MASK = 0x3F3F3F                         ; a quarter of each channel

; a skin file, as the kernel reads it (gui/skincode.inc): the offsets of
; its parameters and of the table of bitmaps, its entries kind, type, the
; offset of the bitmap
SKIN_SIGN   = "SKIN"
SKIN_IDENT  = 0
SKIN_PARAMS = 8
SKIN_BITMAPS = 16
PARAM_INNER = 12                                ; the colours of an active window
PARAM_OUTER = 16
PARAM_FRAME = 20
BITMAP_KIND = 0
BITMAP_TYPE = 2
BITMAP_DATA = 4
BITMAP_ENTRY = 8
BITMAP_W    = 0                                 ; a bitmap: its width, height, BGR rows
BITMAP_H    = 4
BITMAP_PIXELS = 8
BGR         = 3                                 ; a pixel of a bitmap: blue, green, red
BITMAP_BASE = 3                                 ; the middle of the caption
BITMAP_INACTIVE = 0
SKIN_TOP    = 2                                 ; rows of a caption that look as a frame
SKIN_BOTTOM = 1

; buttons: the ids of the shutdown buttons are their SSSF_SHUTDOWN_*
BTN_CANCEL   = 1
BTN_AUTOSAVE = 5

; keys
KEY_ENTER  = 13
KEY_CTRL_S = 19
KEY_ESC    = 27
KEY_HOME   = 180
KEY_END    = 181

; ====================================================================

; a shutdown button and its key
struct BUTTON
        x_w             dd ?            ; x shl 16 + BTN_W
        id              db ?
        key             db ?
        color           dd ?            ; -> the colour
        caption         dd ?
        caption_x_y     dd ?
        hotkey          dd ?
        hotkey_x_y      dd ?
ends

; a BUTTON in the table, its bold texts centred: a pixel apart, so the
; last letter is a pixel narrower
macro button_row x, id, key, color, caption, hotkey
{
        local   start
start:
        dd      x shl 16 + BTN_W
        db      id, key
        dd      color
        dd      caption, (x + (BTN_W - caption#.chars*BOLD_W + 1)/2) shl 16 + CAPTION_Y
        dd      hotkey, (x + (BTN_W - hotkey#.chars*FONT_CHAR_W)/2) shl 16 + CAPTION_Y + FONT_CHAR_H
        assert  $ - start = sizeof.BUTTON
}

; a text and the number of its characters: its bytes but the ones that go
; on with a character, 10xxxxxx

struc text string
{
        local   n, b

        . db    string, 0

        n = 0
        repeat  $ - . - 1
                load    b byte from . + % - 1
                if b shr 6 <> 10b
                        n = n + 1
                end if
        end repeat
        .chars = n
}

; ====================================================================

buttons:
        ;          x         id                      key        color   caption    hotkey
        button_row BTN_COL1, SSSF_SHUTDOWN_KERNEL,   KEY_HOME,  blue,   sz_kernel, sz_home
        button_row BTN_COL2, SSSF_SHUTDOWN_REBOOT,   KEY_ENTER, green,  sz_reboot, sz_enter
        button_row BTN_COL3, SSSF_SHUTDOWN_POWEROFF, KEY_END,   red,    sz_off,    sz_end
buttons_end:

rdsave_run:
        dd      SSF_START_APP
        dd      0
        dd      rdsave_then
        dd      0, 0
        db      "/sys/rdsave", 0

notify_run:
        dd      SSF_START_APP
        dd      0
        dd      sz_not_saved
        dd      0, 0
        db      "/sys/@notify", 0

rdsave_then     db THEN_KEY
.action         db ?, 0                         ; the digit of [action]

system_ini      db "/sys/settings/system.ini", 0
sec_style       db "style", 0
key_skin        db "default_skin", 0
no_skin         db 0                            ; ini_get_str: no skin, empty

; libini linked by the kernel before START
data import
library libini, "libini.obj"
import  libini, \
        ini_get_str, "ini_get_str", \
        ini_get_int, "ini_get_int"
end data

; ====================================================================

if lang eq ru_RU

sz_title        text "Выход из KolibriOS"
sz_save         cp866 "Сохранить изменения", 0
sz_save_to      cp866 "Сохранить изменения: "
.chars = $ - sz_save_to
                db 0
sz_floppy       cp866 "дискета A", 0
sz_saving       cp866 "Сохранение...", 0
sz_not_saved    cp866 "'RDsave\nОбраз не сохранён' -tE", 0
BTN_CHARS       = 12                            ; the longest caption, 'Перезагрузка'
sz_kernel       text "Ядро"
sz_reboot       text "Перезагрузка"
sz_off          text "Выключение"
sz_cancel       text "Отмена"

else if lang eq es_ES

sz_title        text "Apagar KolibriOS"
sz_save         db "Guardar cambios", 0
sz_save_to      db "Guardar cambios en: "
.chars = $ - sz_save_to
                db 0
sz_floppy       db "disquete A", 0
sz_saving       db "Guardando...", 0
sz_not_saved    db "'RDsave\nNo se pudo guardar la imagen' -tE", 0
BTN_CHARS       = 9                             ; 'Reiniciar'
sz_kernel       text "Núcleo"
sz_reboot       text "Reiniciar"
sz_off          text "Apagar"
sz_cancel       text "Cancelar"

else

sz_title        text "Shutdown KolibriOS"
sz_save         db "Save changes", 0
sz_save_to      db "Save changes to: "
.chars = $ - sz_save_to
                db 0
sz_floppy       db "floppy A", 0
sz_saving       db "Saving...", 0
sz_not_saved    db "'RDsave\nThe image is not saved' -tE", 0
BTN_CHARS       = 9                             ; 'Power Off'
sz_kernel       text "Kernel"
sz_reboot       text "Reboot"
sz_off          text "Power Off"
sz_cancel       text "Cancel"

end if

sz_home         text "[Home]"
sz_enter        text "[Enter]"
sz_end          text "[End]"

; the buttons fit the longest text, the window the label and the title
BTN_W       = GAP + BTN_CHARS*BOLD_W + GAP
irp t, sz_kernel, sz_reboot, sz_off, sz_home, sz_enter, sz_end
{
        assert t#.chars <= BTN_CHARS
}
; the label of the checkbox: the texts fit, a path of some length after
; sz_save_to
NAME_X      = LABEL_X + sz_save_to.chars*FONT_CHAR_W
NAME_CHARS  = (LABEL_X + LABEL_W - NAME_X)/FONT_CHAR_W
assert NAME_CHARS >= 16
assert TITLE_W <= DIALOG_W - 2*INNER

; ====================================================================

align 4
autosave        rd 1
target          rd 1
action          rd 1                            ; SSSF_SHUTDOWN_* of the pressed button
blue            rd 1
green           rd 1
red             rd 1
frame_outer     rd 1                            ; the lines of the frame, in order
frame_middle    rd 1
frame_inner     rd 1
skin_bitmap     rd 1                            ; the base bitmap of the skin

sc              system_colors

fname_buf       rb PATH_MAX                     ; the skin path, at the end the RDsave one
row_buf         rb HEADER_W*3                   ; a row of the header, BGR
