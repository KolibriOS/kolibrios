; SPDX-License-Identifier: NOASSERTION
;
; SETUP - system settings
;
; SETUP BOOT  apply the settings from system.ini, run from AUTORUN.DAT
; SETUP       window to change them, each change saved at once
;
; Texts are UTF-8

; ====================================================================

include "../../macros.inc"
include "../../KOSfuncs.inc"

format meos 01
entry START
stack 4096

include "../../proc32.inc"
include "../../dll.inc"

include "group_box.inc"
include "settings.inc"

; ====================================================================

START:
        mcall   SF_SYS_MISC, SSF_HEAP_INIT
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     quit

        GetCommandLine eax
        cmp     dword [eax], 'BOOT'
        jne     window

        call    settings_read_boot
        call    settings_read_ini
        call    settings_apply_boot
        call    style_apply

quit:
        mcall   SF_TERMINATE_PROCESS

window:
        call    settings_read_system

; ====================================================================

draw_window:
        mov     eax, [language]
        mov     eax, [texts + eax*4]
        mov     [text], eax

        mcall   SF_REDRAW, SSF_BEGIN_DRAW
        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, sizeof.system_colors

        ; the text colours with the font flags; sc stays as the skin has
        ; it for group_box
        mov     eax, [sc.work_text]
        or      eax, FONT
        mov     [textColor], eax
        mov     eax, [sc.work_button_text]
        or      eax, FONT
        mov     [buttonTextColor], eax

        ; the border colour of a kernel button, for the frames drawn over
        ; buttons and round the checkboxes
        mov     eax, [sc.work_button]
        shr     eax, 1
        and     eax, 0x7F7F7F
        mov     [buttonBorder], eax

        ; the title in the window language
        mov     ecx, T_TITLE
        call    get_text
        mov     edi, edx

        ; the window in the centre of the screen, as DOCPACK; the kernel
        ; takes its place only the first time
        mcall   SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
        lea     esi, [eax + WINDOW_H]
        mcall   SF_GET_SCREEN_SIZE

        movzx   ecx, ax
        sub     ecx, esi
        shr     ecx, 1
        shl     ecx, 16
        add     ecx, esi

        shr     eax, 16
        sub     eax, WINDOW_W
        shr     eax, 1
        shl     eax, 16
        lea     ebx, [eax + WINDOW_W]

        mov     edx, [sc.work]
        or      edx, WINDOW_STYLE
        mcall   SF_CREATE_WINDOW, , , , 0

        ; a row: the label, the value (drawn below), the '-' and '+'
        ; buttons
        xor     ebp, ebp

.row:
        imul    edi, ebp, ROW_STEP
        add     edi, GAP                ; y of the row
        mov     ecx, edi
        shl     ecx, 16
        add     ecx, ROW_H - 1          ; the button is 1 pixel bigger than given
        lea     edx, [FIRST_ROW_BUTTON + ebp*2]
        mcall   SF_DEFINE_BUTTON, <BUTTON1_X, ROW_H - 1>, , , [sc.work_button]
        lea     edx, [FIRST_ROW_BUTTON + 1 + ebp*2]
        mcall   SF_DEFINE_BUTTON, <BUTTON2_X, ROW_H - 1>

        ; the kernel leaves the corners of a button out: one frame for
        ; both, its right corners stay out
        mov     edx, [buttonBorder]
        mov     cx, 1
        mcall   SF_DRAW_RECT, <BUTTON1_X, BUTTON2_X + ROW_H - 1 - BUTTON1_X>
        add     ecx, (ROW_H - 1) shl 16
        mcall   SF_DRAW_RECT

        lea     ebx, [edi + TEXT_DY + (LABEL_X shl 16)]
        mov     ecx, ebp
        call    get_text
        mcall   SF_DRAW_TEXT, , [textColor]

        lea     ebx, [edi + TEXT_DY + ((BUTTON1_X + 8) shl 16)]
        lea     edx, [glyphs + ebp*4]
        mcall   SF_DRAW_TEXT, , [buttonTextColor]
        lea     ebx, [edi + TEXT_DY + ((BUTTON2_X + 8) shl 16)]
        lea     edx, [glyphs + 2 + ebp*4]
        mcall   SF_DRAW_TEXT

        inc     ebp
        cmp     ebp, ROWS
        jb      .row

        ; the group of the checkboxes, its title in the window language
        mov     ecx, T_GROUP
        call    get_text
        mov     [checkGroup + GROUP_BOX.title], edx
        push    checkGroup
        call    group_box_draw

        ; the checkboxes are spread over the group, the last one ends at
        ; its end; LBA and PCI are the same in all the languages, so the
        ; step only depends on the speaker label
        mov     ecx, T_CHECKS + CHECKS - 1
        call    get_text
        mov     eax, ITEM_END - ITEM_X - CHECK_TEXT_X

.length:
        mov     cl, [edx]
        inc     edx
        test    cl, cl
        jz      .step
        and     cl, 0xC0
        cmp     cl, 0x80                ; a UTF-8 continuation byte
        je      .length
        sub     eax, 8
        jmp     .length

.step:
        shr     eax, 1
        mov     [checkStep], eax

        ; a checkbox: the box, then the label, both under one button up to
        ; the end of the group; the next button covers the rest. A set box
        ; is a button of its own with the same id, over the big one, with a
        ; tick; an empty one a frame, light inside
        xor     ebp, ebp

.check:
        mov     edi, [checkStep]
        imul    edi, ebp
        add     edi, ITEM_X             ; x of the box
        lea     ebx, [edi + CHECK_TEXT_X]
        shl     ebx, 16
        add     ebx, CHECK_Y + TEXT_DY
        lea     ecx, [T_CHECKS + ebp]
        call    get_text
        mcall   SF_DRAW_TEXT, , [textColor]

        mov     ebx, edi
        shl     ebx, 16
        add     ebx, ITEM_END
        sub     ebx, edi
        lea     edx, [FIRST_CHECK_BUTTON + BT_HIDE + BT_NOFRAME + ebp]
        mcall   SF_DEFINE_BUTTON, , <CHECK_Y, ROW_H - 1>

        mov     ebx, edi
        shl     ebx, 16
        add     ebx, CHECK_SIZE
        mov     eax, [checks + ebp*sizeof.CHECK + CHECK.variable]
        mov     eax, [eax]
        xor     eax, [checks + ebp*sizeof.CHECK + CHECK.unchecked]
        jnz     .set

        mcall   SF_DRAW_RECT, , <CHECK_BOX_Y, CHECK_SIZE>, [buttonBorder]
        add     ebx, (1 shl 16) - 2
        mcall   SF_DRAW_RECT, , <CHECK_BOX_Y + 1, CHECK_SIZE - 2>, [sc.work_light]
        jmp     .next
.set:
        dec     ebx                     ; the button is 1 pixel bigger than given
        lea     edx, [FIRST_CHECK_BUTTON + ebp]
        mcall   SF_DEFINE_BUTTON, , <CHECK_BOX_Y, CHECK_SIZE - 1>, , [sc.work_button]

        ; the frame of an empty box over the edge of the button
        inc     ebx
        mov     edx, [buttonBorder]
        mcall   SF_DRAW_RECT, , <CHECK_BOX_Y, 1>
        mcall   SF_DRAW_RECT, , <CHECK_BOX_Y + CHECK_SIZE - 1, 1>
        mov     bx, 1
        mcall   SF_DRAW_RECT, , <CHECK_BOX_Y, CHECK_SIZE>
        add     ebx, (CHECK_SIZE - 1) shl 16
        mcall   SF_DRAW_RECT

        ; the tick: two strokes, each two lines a pixel apart; x shl 16 + x
        ; of the inside of the box, the strokes from it
        lea     ebx, [edi + 1]
        mov     eax, ebx
        shl     ebx, 16
        add     ebx, eax
        lea     esi, [ebx + (TICK_X1 shl 16) + TICK_X2]
        lea     edi, [ebx + (TICK_X2 shl 16) + TICK_X3]
        mov     edx, [sc.work_button_text]
        mcall   SF_DRAW_LINE, esi, <TICK_Y1, TICK_Y2>
        mcall   SF_DRAW_LINE, , <TICK_Y1 - 1, TICK_Y2 - 1>
        mcall   SF_DRAW_LINE, edi, <TICK_Y2, TICK_Y3>
        mcall   SF_DRAW_LINE, , <TICK_Y2 - 1, TICK_Y3 - 1>

.next:
        inc     ebp
        cmp     ebp, CHECKS
        jb      .check

        ; the values: the language, the font smoothing, the font height
        mov     edx, languageNames
        mov     ecx, [language]
        call    nth_string
        mov     [valueText], edx

        mov     ecx, [fontSmoothing]
        add     ecx, T_OFF              ; 'off', 'on', 'subpixel'
        call    get_text
        mov     [valueText + 4], edx

        mov     eax, [fontHeight]
        mov     bl, 10
        div     bl
        add     ax, '00'
        mov     word [heightText], ax
        mov     [valueText + 8], heightText

        ; each value in a light box framed on three sides, the button is
        ; the fourth, the left corners are cut off
        xor     ebp, ebp

.value:
        imul    edi, ebp, ROW_STEP
        add     edi, GAP
        mov     ecx, edi
        shl     ecx, 16
        add     ecx, ROW_H
        mcall   SF_DRAW_RECT, <VALUE_X + 1, VALUE_W - 1>, , [sc.work_graph]
        add     ecx, (1 shl 16) - 2
        mcall   SF_DRAW_RECT, <VALUE_X, VALUE_W>

        ; sunken, the inner lines of a button reversed: three boxes, each
        ; 1 pixel smaller, leave darker lines left and top and lighter
        ; right and bottom
        mov     edx, [sc.work_light]
        mov     eax, edx
        shr     eax, 3
        and     eax, 0x1F1F1F
        sub     edx, eax                ; 1/8 darker
        mcall   SF_DRAW_RECT, <VALUE_X + 1, VALUE_W - 1>
        mov     edx, [sc.work_light]
        or      edx, 0x1F1F1F           ; lighter
        add     ecx, (1 shl 16) - 1
        mcall   SF_DRAW_RECT, <VALUE_X + 2, VALUE_W - 2>
        dec     ecx
        mcall   SF_DRAW_RECT, <VALUE_X + 2, VALUE_W - 3>, , [sc.work_light]

        lea     ebx, [edi + TEXT_DY + ((VALUE_X + GAP) shl 16)]
        mov     edx, [valueText + ebp*4]
        mcall   SF_DRAW_TEXT, , [textColor]

        inc     ebp
        cmp     ebp, ROWS
        jb      .value

        mcall   SF_REDRAW, SSF_END_DRAW

; ====================================================================

still:
        mcall   SF_WAIT_EVENT

        cmp     eax, EV_REDRAW
        je      draw_window
        cmp     eax, EV_KEY
        je      key
        cmp     eax, EV_BUTTON
        jne     still

        mcall   SF_GET_BUTTON
        shr     eax, 8
        cmp     eax, 1                  ; the close button
        je      quit
        sub     eax, FIRST_ROW_BUTTON
        jb      still                   ; 0: no button after all
        cmp     eax, FIRST_CHECK_BUTTON - FIRST_ROW_BUTTON
        jae     .check

        ; '-' or '+' of a row: step the value within the limits of the row
        shr     eax, 1                  ; eax = row, CF = '+'
        lea     esi, [eax*3]            ; sizeof.ROW_LIMITS = 3*4
        lea     esi, [rows + esi*4]
        mov     edi, [esi + ROW_LIMITS.variable]
        mov     ecx, [edi]
        jc      .plus
        cmp     ecx, [esi + ROW_LIMITS.min]
        jbe     still
        dec     ecx
        jmp     .set

.plus:
        cmp     ecx, [esi + ROW_LIMITS.max]
        jae     still
        inc     ecx

.set:
        mov     [edi], ecx
        jmp     .changed

.check:
        mov     eax, [checks - (FIRST_CHECK_BUTTON - FIRST_ROW_BUTTON)*sizeof.CHECK + eax*sizeof.CHECK]
        xor     dword [eax], 1

        ; every change takes effect at once, is kept in system.ini and
        ; repaints the window
.changed:
        call    settings_apply
        call    settings_save_ini
        jmp     draw_window

key:
        mcall   SF_GET_KEY
        jmp     still

; ====================================================================

; edx = the string number ecx of the window language
get_text:
        mov     edx, [text]

; edx = the string number ecx of the zero separated list at edx
nth_string:
        jecxz   .done

.skip:
        inc     edx
        cmp     byte [edx - 1], 0
        jne     .skip
        loop    .skip

.done:
        ret

; ====================================================================

; layout of the client area, on the grid of DOCPACK: GAP between the
; controls and to the window border, the controls ROW_H high, the texts
; in cells of FONT_CHAR_W; a RECT is x, y, width, height
GAP             = 12
ROW_H           = 24
ROW_STEP        = ROW_H + GAP
TEXT_DY         = (ROW_H - FONT_CAPS_H)/2 - FONT_CAPS_Y ; the capitals centred in a row

; the rows: the label, the value box, the '-' and '+' buttons sharing a
; border
LABEL_CHARS     = 20                    ; the longest label, Spanish
VALUE_CHARS     = 10                    ; the longest value, 'Українська'
LABEL_X         = GAP
VALUE_X         = LABEL_X + LABEL_CHARS*FONT_CHAR_W + GAP
VALUE_W         = GAP + VALUE_CHARS*FONT_CHAR_W + GAP
BUTTON1_X       = VALUE_X + VALUE_W
BUTTON2_X       = BUTTON1_X + ROW_H - 1
CLIENT_W        = BUTTON2_X + ROW_H + GAP

; the group of the checkboxes under the rows, GAP to the capitals of its
; title; the checkboxes GAP inside its lines and GAP under the title
GROUP_TOP       = GROUP_TITLE_BELOW + GAP ; from the top line to the checkboxes
GROUP_Y         = GAP + ROWS*ROW_STEP + GROUP_TITLE_ABOVE
GROUP_H         = GROUP_TOP + ROW_H + GAP + GROUP_FRAME_LINES
GROUP           RECT    GAP, GROUP_Y, CLIENT_W - GAP*2, GROUP_H
CHECK_Y         = GROUP.Y + GROUP_TOP   ; the row of the checkboxes
CHECK_SIZE      = 15
CHECK_BOX_Y     = CHECK_Y + (ROW_H - CHECK_SIZE)/2
CHECK_TEXT_X    = CHECK_SIZE + FONT_CHAR_W ; the label a character after the box

; the tick: from TICK_X1, TICK_Y1 down to TICK_X2, TICK_Y2, then up to
; TICK_X3, TICK_Y3, in the inside of the box
TICK_X1         = 2
TICK_X2         = 5
TICK_X3         = 10
TICK_Y1         = CHECK_BOX_Y + 1 + 6
TICK_Y2         = CHECK_BOX_Y + 1 + 9
TICK_Y3         = CHECK_BOX_Y + 1 + 4
ITEM_X          = GROUP.X + GROUP_FRAME_LINES + GAP
ITEM_END        = GROUP.X + GROUP.W - GROUP_FRAME_LINES - GAP
CLIENT_H        = GROUP.Y + GROUP.H + GAP

; window
WINDOW_W        = CLIENT_W + 9          ; borders 5 + 5, the kernel adds a pixel
WINDOW_H        = CLIENT_H + 4          ; border 5, the same pixel; redraw adds the skin
WINDOW_STYLE    = 0x34000000            ; skinned, fixed size, caption, client coordinates

; controls
ROWS            = 3
CHECKS          = 3
FIRST_ROW_BUTTON   = 10                 ; row N has '-' = 10 + N*2 and '+' = 11 + N*2
FIRST_CHECK_BUTTON = 20
FONT            = 0xB0000000            ; zero terminated, 8x16 UTF-8
GROUP_USES      = GROUP_USE_UTF8 + GROUP_USE_REGULAR + GROUP_USE_TOP ; what group_box assembles

; ====================================================================

@IMPORT:
library libini, "libini.obj"
import  libini, \
        ini.get_str, "ini_get_str", \
        ini.get_int, "ini_get_int", \
        ini.set_str, "ini_set_str", \
        ini.set_int, "ini_set_int"

; the group of the checkboxes, its title set for the window language
checkGroup:
        group_box_row GROUP.X, GROUP.Y, GROUP.W, GROUP.H, 0, sc, GROUP_FONT_UTF8

; a row of the window: the variable it shows and its limits
struct ROW_LIMITS
        variable        dd ?
        min             dd ?
        max             dd ?
ends
assert sizeof.ROW_LIMITS = 3*4

align 4
rows:
        dd      language,      0,          LANGUAGES - 1
        dd      fontSmoothing, 0,          2
        dd      fontHeight,    FONT_H_MIN, FONT_H_MAX

; a checkbox: its variable and the value of it that means 'not checked'
struct CHECK
        variable        dd ?
        unchecked       dd ?
ends
assert sizeof.CHECK = 8

checks:
        dd      lba,         0
        dd      pci,         0
        dd      speakerMute, 1
assert ($ - checks)/sizeof.CHECK = CHECKS

; the signs on the buttons of a row: two strings of 2 bytes, '-' and '+'
glyphs          db      '<', 0, '>', 0      ; the language
                db      '<', 0, '>', 0      ; the font smoothing
                db      '-', 0, '+', 0      ; the font height
assert $ - glyphs = ROWS*4

; every language in its own language
languageNames   db      'English', 0, 'Suomi', 0, 'Deutsch', 0, 'Русский', 0
                db      'Français', 0, 'Eesti', 0, 'Українська', 0, 'Italiano', 0
                db      'Vlaams', 0, 'Español', 0, 'Català', 0


; ====================================================================

; the texts of a window language: zero terminated strings in this order
; (the row labels, the group title, the checkbox labels, 'off', 'on',
; 'subpixel': the font smoothing values 0, 1, 2, the window title)
T_GROUP         = ROWS
T_CHECKS        = ROWS + 1
T_OFF           = ROWS + 1 + CHECKS
T_TITLE         = T_OFF + 3

; the texts of every kernel language, English where there is no translation
texts           dd      texteng, texteng, texteng, textrus, texteng, texteng
                dd      texteng, texteng, texteng, textspa, texteng
assert ($ - texts)/4 = LANGUAGES

texteng         db      'System language', 0, 'Font smoothing', 0, 'Font height', 0
                db      'Access settings', 0, 'LBA', 0, 'PCI', 0, 'Speaker', 0
                db      'Off', 0, 'On', 0, 'Subpixel', 0
                db      3, 'System settings', 0 ; 3: the title is UTF-8

textspa         db      'Idioma del sistema', 0, 'Suavizado de fuentes', 0, 'Altura de fuente', 0
                db      'Ajustes de acceso', 0, 'LBA', 0, 'PCI', 0, 'Altavoz', 0
                db      'No', 0, 'Sí', 0, 'Subpíxel', 0
                db      3, 'Ajustes del sistema', 0

textrus         db      'Язык системы', 0, 'Сглаживание шрифтов', 0, 'Высота шрифтов', 0
                db      'Настройки доступа', 0, 'LBA', 0, 'PCI', 0, 'Динамик', 0
                db      'Нет', 0, 'Да', 0, 'Субпиксель', 0
                db      3, 'Настройки системы', 0

; ====================================================================

; not stored in the program file: the variables of settings.inc, then
; these
IncludeUGlobals
text            dd      ?               ; the texts of the window language
checkStep       dd      ?               ; the x distance of two checkboxes
textColor       dd      ?               ; work_text with FONT
buttonTextColor dd      ?               ; work_button_text with FONT
buttonBorder    dd      ?               ; the border colour of a kernel button
sc              system_colors
valueText       rd      ROWS            ; the strings shown in the value boxes
heightText      rb      4
