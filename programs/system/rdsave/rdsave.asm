; SPDX-License-Identifier: NOASSERTION
;
; RDsave - save the RAM-disk to a real medium
;
; Authors: Mario79, Heavyiron, Lrz, Burer
;
; RDSAVE          the window: save to floppy A or to a file, the target of
;                 save.inc, kept for the next save and for End
; RDSAVE -then N  the question of End: after the save, or without it, the
;                 shutdown SSSF_SHUTDOWN_* N
;
; Texts are in the language of the kernel: the window UTF-8, @notify cp866

; ====================================================================

include "../../macros.inc"
include "../../KOSfuncs.inc"
include "../../encoding.inc"

format meos KX
entry START
stack 4096

include "../../proc32.inc"
include "../../string.inc"

include "group_box.inc"

; ====================================================================

START:
        ; the texts of the kernel language; an unknown id, 0 too (unsigned
        ; after dec), English
        mcall   SF_SYSTEM_GET, 5
        dec     eax                     ; the kernel counts languages from 1
        cmp     eax, LANGUAGES
        jb      .language
        xor     eax, eax

.language:
        mov     eax, [texts + eax*4]
        mov     [text], eax

        call    read_then
        call    rd_read_target

        call    rd_has_floppy
        mov     [hasFloppy], eax

        ; the kept target; without one floppy A if there is a drive, else
        ; a file
        cmp     [target], TARGET_NONE
        jne     .check_floppy
        mov     [target], TARGET_FLOPPY

.check_floppy:
        cmp     [target], TARGET_FLOPPY
        jne     .dialog
        cmp     [hasFloppy], 0
        jne     .dialog
        mov     [target], TARGET_FILE

        ; the dialog opens at the kept file
.dialog:
        stdcall string.copy, fname_buf, dialog_path
        stdcall string.copy, default_name, filename_area
        invoke  OpenDialog_Init, OpenDialog_data

; ====================================================================

redraw:
        call    draw_window

still:
        mcall   SF_WAIT_EVENT

        cmp     eax, EV_REDRAW
        je      redraw
        cmp     eax, EV_KEY
        je      key
        cmp     eax, EV_BUTTON
        je      button

        jmp     still

; ====================================================================

key:
        mcall   SF_GET_KEY

        cmp     ah, KEY_ESC
        je      quit
        cmp     ah, KEY_ENTER
        je      save
        cmp     ah, KEY_TAB
        je      browse

        jmp     still

; ====================================================================

button:
        mcall   SF_GET_BUTTON
        shr     eax, 8

        cmp     eax, BTN_CLOSE
        je      quit
        cmp     eax, BTN_CANCEL
        je      quit
        cmp     eax, BTN_SAVE
        je      save
        cmp     eax, BTN_DONT_SAVE
        je      shut_down
        cmp     eax, BTN_CHOOSE
        je      browse
        cmp     eax, BTN_TARGET + TARGET_FLOPPY
        je      select_floppy
        cmp     eax, BTN_TARGET + TARGET_FILE
        je      select_file

        jmp     still

quit:
        mcall   SF_TERMINATE_PROCESS

; ====================================================================

select_floppy:
        mov     [target], TARGET_FLOPPY
        call    draw_marks
        jmp     still

; the file target: with no file picked yet, the dialog
select_file:
        cmp     byte [fname_buf], 0
        je      browse

        mov     [target], TARGET_FILE
        call    draw_marks
        jmp     still

browse:
        call    pick_file
        jmp     redraw

save:
        cmp     [target], TARGET_FILE
        jne     .write
        cmp     byte [fname_buf], 0
        jne     .write

        ; no file yet: only the dialog; a picked file shows in the window
        ; and the next Save writes to it, as after Choose
        jmp     browse

        ; no file on the RAM-disk: the kernel would write the RAM-disk into
        ; itself, over the data it writes; then the target into app.ini
        ; before the save, so the saved image keeps it; "Saving" in the
        ; title while the save runs
.write:
        cmp     [target], TARGET_FILE
        jne     .keep
        call    on_ramdisk
        test    eax, eax
        jz      .keep
        mov     eax, ERR_ON_RAMDISK
        jmp     .failed

.keep:
        call    keep_target
        mov     ecx, T_SAVING
        call    get_text
        mcall   SF_SET_CAPTION, 1, edx

        call    rd_save
        test    eax, eax
        jnz     .failed

        cmp     [then], 0
        jne     shut_down

        mov     ecx, T_SAVED
        mov     edx, notify_ok
        call    notify
        jmp     quit

        ; the message of the error, the title back
.failed:
        mov     ecx, T_ON_RAMDISK
        cmp     eax, ERR_ON_RAMDISK
        je      .message
        mov     ecx, T_UNKNOWN
        cmp     eax, ERR_DEVICE
        ja      .message

        ; a file system error; one without a text of its own is unknown
        mov     ecx, T_ERRORS - 1
        add     ecx, eax
        mov     esi, ecx
        call    get_text
        mov     ecx, esi
        cmp     byte [edx], 0
        jne     .message
        mov     ecx, T_UNKNOWN

.message:
        mov     edx, notify_error
        call    notify
        mcall   SF_SET_CAPTION, 1, [title]
        jmp     still

; the shutdown End asked for, with or without the save
shut_down:
        mcall   SF_SYSTEM, SSF_SHUTDOWN, [then]
        jmp     quit

; ====================================================================

include "save.inc"

; "-then N" in the command line: [then] = N and [title] its question;
; else [then] stays 0 and [title] the name of the program
read_then:
        GetCommandLine esi
        mov     edi, then_key
        mov     ecx, then_key.len
        repe cmpsb
        jne     .done

        movzx   eax, byte [esi]
        sub     eax, '0'
        mov     edi, then_actions

.next:
        cmp     eax, [edi + THEN.action]
        je      .found

        add     edi, sizeof.THEN
        cmp     edi, then_actions_end
        jb      .next

        ret

.found:
        mov     [then], eax
        mov     ecx, [edi + THEN.title]
        call    get_text
        mov     [title], edx

.done:
        ret

; the dialog; a picked file is the target
pick_file:
        invoke  OpenDialog_Start, OpenDialog_data

        cmp     [OpenDialog_data.status], OPENDIALOG_OK
        jne     .done

        stdcall string.copy, dialog_path, fname_buf
        mov     [target], TARGET_FILE

.done:
        ret

; eax = 1 if fname_buf may be on the RAM-disk, else 0: a path without
; '/' first goes from the current folder, /sys/ (fs_lfn.inc
; get_full_file_name); one of ramdisk_prefixes first, in any case. /sys/ is
; the RAM-disk unless the loader gave another system folder: refused
; anyway
on_ramdisk:
        mov     eax, 1
        cmp     byte [fname_buf], '/'
        jne     .done

        mov     esi, ramdisk_prefixes

.prefix:
        mov     edi, fname_buf

.char:
        mov     al, [esi]
        test    al, al
        jz      .found                  ; the whole prefix matched

        ; the character of the path in lower case
        mov     ah, [edi]
        cmp     ah, 'A'
        jb      .compare
        cmp     ah, 'Z'
        ja      .compare
        add     ah, 'a' - 'A'

.compare:
        cmp     al, ah
        jne     .skip
        inc     esi
        inc     edi
        jmp     .char

        ; past the zero of this prefix; the list ends with an empty one
.skip:
        inc     esi
        cmp     byte [esi - 1], 0
        jne     .skip
        cmp     byte [esi], 0
        jne     .prefix

        xor     eax, eax
        ret

.found:
        mov     eax, 1

.done:
        ret

; zero terminated, lower case, the list ending with an empty one
ramdisk_prefixes db "/rd/", 0, "/sys/", 0, 0

; the target into app.ini
keep_target:
        invoke  ini_set_int, ini_file, ini_sec, key_target, [target]

        cmp     [target], TARGET_FILE
        jne     .done

        stdcall string.length, fname_buf
        invoke  ini_set_str, ini_file, ini_sec, key_path, fname_buf, eax

.done:
        ret

; ecx = the number of a cp866 text, edx -> the end of the parameters with
; the icon: @notify with the text under the name of the program
notify:
        mov     [notifyEnd], edx
        call    get_text
        mov     [notifyText], edx

        stdcall string.copy, notify_start, notify_buf
        stdcall string.concatenate, [notifyText], notify_buf
        stdcall string.concatenate, [notifyEnd], notify_buf
        mcall   SF_FILE, notify_run
        ret

; ====================================================================

draw_window:
        mcall   SF_REDRAW, SSF_BEGIN_DRAW
        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, sizeof.system_colors

        ; the text colours with the font flags
        mov     eax, [sc.work_text]
        or      eax, FONT
        mov     [textColor], eax
        mov     eax, [sc.work_button_text]
        or      eax, FONT
        mov     [buttonTextColor], eax

        ; the border colour of a kernel button, for the frames drawn over
        ; the buttons
        mov     eax, [sc.work_button]
        shr     eax, 1
        and     eax, 0x7F7F7F
        mov     [buttonBorder], eax

        ; the window in the centre of the screen, as SETUP; the kernel
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
        mov     edi, [title]
        mcall   SF_CREATE_WINDOW, , , , 0

        ; the targets in a group, its title in the window language
        mov     ecx, T_GROUP
        call    get_text
        mov     [group + GROUP_BOX.title], edx
        push    group
        call    group_box_draw

        ; floppy A after its mark, its capitals centred on it, greyed
        ; without a drive
        mov     ecx, T_FLOPPY
        call    get_text
        mov     ecx, [textColor]
        cmp     [hasFloppy], 0
        jne     .floppy
        mov     ecx, [sc.work_graph]
        or      ecx, FONT

.floppy:
        mcall   SF_DRAW_TEXT, <LABEL_X, FLOPPY_Y + MARK_TEXT_DY>

        ; the file after its mark; Choose at the right edge, a plain button
        ; as Save, its caption centred: x + (w - its width)/2
        call    draw_path

        mov     ebx, (CHOOSE_X shl 16) + CHOOSE_W
        mov     ecx, (FILE_Y shl 16) + ROW_H
        mov     edx, BTN_CHOOSE
        mov     esi, [sc.work_button]
        call    define_button

        mov     ecx, T_CHOOSE
        call    get_text
        call    text_chars
        imul    eax, FONT_CHAR_W
        mov     ebx, CHOOSE_W
        sub     ebx, eax
        shr     ebx, 1
        add     ebx, CHOOSE_X
        shl     ebx, 16
        add     ebx, FILE_Y + TEXT_DY
        mcall   SF_DRAW_TEXT, , [buttonTextColor]

        call    draw_marks
        call    draw_buttons

        mcall   SF_REDRAW, SSF_END_DRAW
        ret

; Save and Cancel; asked by End, also the shutdown without a save. Plain
; kernel buttons, their corners round as the kernel draws them: the
; square frame of button_box is for the small controls in the group
draw_buttons:
        mov     edi, buttons
        cmp     [then], 0
        je      .button
        mov     edi, then_buttons

.button:
        mov     ebx, [edi + BUTTON.x]
        shl     ebx, 16
        add     ebx, [edi + BUTTON.w]
        mov     ecx, (BTN_Y shl 16) + ROW_H
        mov     edx, [edi + BUTTON.id]
        mov     esi, [sc.work_button]
        call    define_button

        ; the caption centred: x + (w - its width)/2
        mov     ecx, [edi + BUTTON.caption]
        call    get_text
        call    text_chars
        imul    eax, FONT_CHAR_W
        mov     ebx, [edi + BUTTON.w]
        sub     ebx, eax
        shr     ebx, 1
        add     ebx, [edi + BUTTON.x]
        shl     ebx, 16
        add     ebx, BTN_Y + TEXT_DY
        mcall   SF_DRAW_TEXT, , [buttonTextColor]

        add     edi, sizeof.BUTTON
        cmp     [edi + BUTTON.x], BUTTONS_END
        jne     .button

        ret

; the marks of the targets, drawn again on a change, the old buttons
; deleted first (SF 8 with BT_DEL deletes every button of the id, the
; mark and the label at once); ebp = the target of the row:
; - the target: a button with a tick, as a checkbox of SETUP
; - another target: a sunken box under a hidden button
; - floppy A without a drive: an empty box, no buttons
; the labels, floppy A and the file, are hidden buttons with
; the id of their mark
draw_marks:
        mov     edi, options

.option:
        mov     ebp, [edi + OPTION.target]
        lea     edx, [BTN_TARGET + BT_DEL + ebp]
        mcall   SF_DEFINE_BUTTON

        mov     ecx, [edi + OPTION.mark_y]
        shl     ecx, 16
        add     ecx, MARK_SIZE
        mov     ebx, (MARK_X shl 16) + MARK_SIZE

        cmp     ebp, [target]
        je      .set

        mov     edx, [sc.work_graph]
        call    sunken_box

        ; no drive: no buttons, the label neither
        cmp     ebp, TARGET_FLOPPY
        jne     .hidden
        cmp     [hasFloppy], 0
        je      .next

        ; the hidden button inside the frame: the kernel inverts the light
        ; lines there, the grey frame would stay grey inverted
.hidden:
        mov     ecx, [edi + OPTION.mark_y]
        inc     ecx
        shl     ecx, 16
        add     ecx, MARK_SIZE - 2
        mov     ebx, ((MARK_X + 1) shl 16) + MARK_SIZE - 2
        lea     edx, [BTN_TARGET + BT_HIDE + ebp]
        call    define_button
        jmp     .label

        ; the tick: two strokes, each two lines a pixel apart, from the
        ; top of the mark
.set:
        lea     edx, [BTN_TARGET + ebp]
        mov     esi, [sc.work_button]
        call    button_box

        mov     esi, [edi + OPTION.mark_y]
        mov     edx, [sc.work_button_text]
        lea     ecx, [esi + TICK_Y1]
        shl     ecx, 16
        lea     ecx, [ecx + esi + TICK_Y2]
        mcall   SF_DRAW_LINE, <TICK_X1, TICK_X2>
        sub     ecx, (1 shl 16) + 1
        mcall   SF_DRAW_LINE
        lea     ecx, [esi + TICK_Y2]
        shl     ecx, 16
        lea     ecx, [ecx + esi + TICK_Y3]
        mcall   SF_DRAW_LINE, <TICK_X2, TICK_X3>
        sub     ecx, (1 shl 16) + 1
        mcall   SF_DRAW_LINE

        ; the label presses as the mark
.label:
        mov     ebx, [edi + OPTION.label_x_w]
        mov     ecx, [edi + OPTION.label_y_h]
        lea     edx, [BTN_TARGET + BT_HIDE + BT_NOFRAME + ebp]
        call    define_button

.next:
        add     edi, sizeof.OPTION
        cmp     edi, options_end
        jb      .option

        ret

; the file after its mark, in cp866: its end if it is longer than the
; place before Choose; "file not selected" without one
draw_path:
        cmp     byte [fname_buf], 0
        jne     .path

        mov     ecx, T_NO_FILE
        call    get_text
        mcall   SF_DRAW_TEXT, <LABEL_X, FILE_Y + TEXT_DY>, [textColor]
        ret

.path:
        stdcall string.length, fname_buf
        mov     edx, fname_buf
        mov     esi, eax
        cmp     esi, PATH_CHARS
        jbe     .draw

        add     edx, esi
        sub     edx, PATH_CHARS
        mov     esi, PATH_CHARS

.draw:
        mov     ecx, [sc.work_text]
        or      ecx, FONT_CP866
        mcall   SF_DRAW_TEXT, <LABEL_X, FILE_Y + TEXT_DY>
        ret

; ====================================================================

; ebx = x shl 16 + width, ecx = y shl 16 + height, edx = the id, esi =
; the colour: a kernel button with a square frame in the border colour of
; a button, as the kernel leaves its corners out; keeps ebx, ecx
button_box:
        call    define_button
        mov     edx, [buttonBorder]
        jmp     frame

; ebx = x shl 16 + width, ecx = y shl 16 + height, edx = the id and the
; flags, esi = the colour: a kernel button of this size; keeps ebx, ecx
define_button:
        dec     ebx                     ; the button is 1 pixel bigger than given
        dec     ecx
        mcall   SF_DEFINE_BUTTON
        inc     ebx
        inc     ecx
        ret

; ebx = x shl 16 + width, ecx = y shl 16 + height, edx = the colour of the
; frame: a field, the frame then sunken inside: the inner lines of a
; button reversed, three boxes, each 1 pixel smaller, leave darker lines
; left and top and lighter right and bottom
sunken_box:
        call    frame
        add     ebx, (1 shl 16) - 2
        add     ecx, (1 shl 16) - 2
        mov     edx, [sc.work_light]
        mov     eax, edx
        shr     eax, 3
        and     eax, 0x1F1F1F
        sub     edx, eax                ; 1/8 darker
        mcall   SF_DRAW_RECT
        mov     edx, [sc.work_light]
        or      edx, 0x1F1F1F           ; lighter
        add     ebx, (1 shl 16) - 1
        add     ecx, (1 shl 16) - 1
        mcall   SF_DRAW_RECT
        dec     ebx
        dec     ecx
        mcall   SF_DRAW_RECT, , , [sc.work_light]
        ret

; ebx = x shl 16 + width, ecx = y shl 16 + height, edx = the colour: a
; line of a pixel round the box; keeps ebx, ecx
frame:
        ; the top and the bottom line, the bottom one height - 1 lower
        push    ecx
        mov     cx, 1
        mcall   SF_DRAW_RECT
        mov     eax, [esp]
        shl     eax, 16
        sub     eax, 1 shl 16
        add     ecx, eax
        mcall   SF_DRAW_RECT
        pop     ecx

        ; the left and the right line, the right one width - 1 further
        push    ebx
        mov     bx, 1
        mcall   SF_DRAW_RECT
        mov     eax, [esp]
        shl     eax, 16
        sub     eax, 1 shl 16
        add     ebx, eax
        mcall   SF_DRAW_RECT
        pop     ebx
        ret

; edx -> a UTF-8 text: eax = the number of its characters, the bytes but
; the ones that go on with a character, 10xxxxxx; keeps edx
text_chars:
        mov     esi, edx
        xor     eax, eax

.byte:
        mov     cl, [esi]
        inc     esi
        test    cl, cl
        jz      .done
        and     cl, UTF8_TAIL_MASK
        cmp     cl, UTF8_TAIL
        je      .byte
        inc     eax
        jmp     .byte

.done:
        ret

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

; layout of the client area: GAP between all that is seen, to the window
; border too: the capitals of the title, the lines of the group, the
; marks, Choose, the buttons; controls ROW_H high, texts in cells of
; FONT_CHAR_W; a RECT is x, y, width, height
GAP             = 12
ROW_H           = 24
TEXT_DY         = (ROW_H - FONT_CAPS_H)/2 - FONT_CAPS_Y ; the capitals centred in a row

; the group of the targets: its capitals GAP under the border, GAP from
; them to the floppy mark, GAP to Choose, GAP from it to the lines;
; the buttons GAP under the group
GROUP_TOP       = GROUP_TITLE_BELOW + GAP ; from the top line to the floppy mark
GROUP           RECT    GAP, GAP + GROUP_TITLE_ABOVE, CLIENT_W - 2*GAP, \
                        GROUP_TOP + MARK_SIZE + GAP + ROW_H + GAP + GROUP_FRAME_LINES
ITEM_X          = GROUP.X + GROUP_FRAME_LINES + GAP
ITEM_END        = GROUP.X + GROUP.W - GROUP_FRAME_LINES - GAP
FLOPPY_Y        = GROUP.Y + GROUP_TOP   ; the top of the floppy mark
FILE_Y          = FLOPPY_Y + MARK_SIZE + GAP ; the top of the file row and of Choose
BTN_Y           = GROUP.Y + GROUP.H + GAP

; a target: its mark as a checkbox of SETUP, a character after it its
; label, the capitals centred on the mark: floppy A, or the file with
; Choose at the right edge, the mark centred on Choose
MARK_SIZE       = 15
MARK_X          = ITEM_X
MARK_TEXT_DY    = (MARK_SIZE - FONT_CAPS_H)/2 - FONT_CAPS_Y
FILE_MARK_Y     = FILE_Y + (ROW_H - MARK_SIZE)/2
LABEL_X         = MARK_X + MARK_SIZE + FONT_CHAR_W
CHOOSE_CHARS    = 10                    ; the longest caption of Choose, 'Выбрать...'
CHOOSE_W        = GAP + CHOOSE_CHARS*FONT_CHAR_W + GAP
CHOOSE_X        = ITEM_END - CHOOSE_W
PATH_CHARS      = (CHOOSE_X - GAP - LABEL_X)/FONT_CHAR_W

; the tick of a set mark, 9 x 7 pixels centred in it, as in End: from
; TICK_X1, TICK_Y1 down to TICK_X2, TICK_Y2, then up to TICK_X3, TICK_Y3;
; x in the window, y from the top of the mark
TICK_X          = MARK_X + (MARK_SIZE - 9)/2
TICK_Y          = (MARK_SIZE - 7)/2
TICK_X1         = TICK_X
TICK_X2         = TICK_X + 3
TICK_X3         = TICK_X + 8
TICK_Y1         = TICK_Y + 3
TICK_Y2         = TICK_Y + 6
TICK_Y3         = TICK_Y + 1

; the buttons over the whole width, two of them or three when End asks;
; the window fits three of the longest caption
BTN_CHARS       = 12                    ; the longest button caption, 'Не сохранять'
BTN_MIN_W       = GAP + BTN_CHARS*FONT_CHAR_W + GAP
BTN2_W          = (CLIENT_W - 3*GAP)/2
BTN3_W          = (CLIENT_W - 4*GAP)/3

; window
CLIENT_W        = GAP + 3*BTN_MIN_W + 2*GAP + GAP
CLIENT_H        = BTN_Y + ROW_H + GAP
assert PATH_CHARS >= 24
assert 2*BTN2_W + 3*GAP = CLIENT_W & 3*BTN3_W + 4*GAP = CLIENT_W
WINDOW_W        = CLIENT_W + 9          ; borders 5 + 5, the kernel adds a pixel
WINDOW_H        = CLIENT_H + 4          ; border 5, the same pixel; draw_window adds the skin height
WINDOW_STYLE    = 0x34000000            ; skinned, fixed size, caption, client coordinates

; text; FONT_CHAR_W and the capitals of 8x16 in group_box.inc
GROUP_USES      = GROUP_USE_UTF8 + GROUP_USE_REGULAR + GROUP_USE_TOP ; what group_box assembles
FONT            = 0xB0000000            ; zero terminated, 8x16 UTF-8
FONT_CP866      = 0x10000000            ; 8x16 cp866, esi characters
UTF8_TAIL_MASK  = 0xC0
UTF8_TAIL       = 0x80                  ; 10xxxxxx goes on with a character

; buttons; a target T is BTN_TARGET + T, its mark and its label
BTN_CLOSE       = 1
BTN_SAVE        = 2
BTN_DONT_SAVE   = 3
BTN_CANCEL      = 4
BTN_CHOOSE      = 5
BTN_TARGET      = 10

ERR_ON_RAMDISK  = -1                    ; a file on the RAM-disk itself, not saved

; keys
KEY_TAB         = 9
KEY_ENTER       = 13
KEY_ESC         = 27

LANGUAGES       = 11                    ; kernel ids 1..11 (data32.inc SYSLANG)
NOTIFY_MAX      = 256

; proc_lib OpenDialog
OPENDIALOG_SAVE = 1
OPENDIALOG_OK   = 1
DIALOG_X        = 200
DIALOG_Y        = 120
DIALOG_W        = 420
DIALOG_H        = 320
NAME_MAX        = 256
PROCINFO_SIZE   = 1024

; ====================================================================

data import
library libini, "libini.obj", proclib, "proc_lib.obj"
import  libini, \
        ini_get_str, "ini_get_str", \
        ini_get_int, "ini_get_int", \
        ini_set_str, "ini_set_str", \
        ini_set_int, "ini_set_int"
import  proclib, \
        OpenDialog_Init, "OpenDialog_init", \
        OpenDialog_Start, "OpenDialog_start"
end data

program_name    db "RDsave", 0

; a button of the bottom row: its x, width, id and the number of its
; caption; a table ends with an x of BUTTONS_END
struct BUTTON
        x               dd ?
        w               dd ?
        id              dd ?
        caption         dd ?
ends
BUTTONS_END     = 0

buttons:
        ;  x                         w       id             caption
        dd GAP,                      BTN2_W, BTN_SAVE,      T_SAVE
        dd GAP + BTN2_W + GAP,       BTN2_W, BTN_CANCEL,    T_CANCEL
        dd BUTTONS_END

then_buttons:
        dd GAP,                      BTN3_W, BTN_SAVE,      T_SAVE
        dd GAP + BTN3_W + GAP,       BTN3_W, BTN_DONT_SAVE, T_DONT_SAVE
        dd GAP + 2*(BTN3_W + GAP),   BTN3_W, BTN_CANCEL,    T_CANCEL
        dd BUTTONS_END

; the group of the targets, its title set for the window language
group:
        group_box_row GROUP.X, GROUP.Y, GROUP.W, GROUP.H, 0, sc, GROUP_FONT_UTF8

; a target: the top of its mark, the hidden button over its label
struct OPTION
        target          dd ?
        mark_y          dd ?
        label_x_w       dd ?            ; x shl 16 + width
        label_y_h       dd ?            ; y shl 16 + height
ends

options:
        ;  target         mark_y       label x and width                         label y and height
        dd TARGET_FLOPPY, FLOPPY_Y,    (LABEL_X shl 16) + ITEM_END - LABEL_X,    (FLOPPY_Y shl 16) + MARK_SIZE
        dd TARGET_FILE,   FILE_MARK_Y, (LABEL_X shl 16) + CHOOSE_X - GAP - LABEL_X, (FILE_Y shl 16) + ROW_H
options_end:

; the shutdown of "-then N" and the title of its question
struct THEN
        action          dd ?            ; SSSF_SHUTDOWN_*
        title           dd ?
ends

then_actions:
        ;  action                  title
        dd SSSF_SHUTDOWN_POWEROFF, T_BEFORE_OFF
        dd SSSF_SHUTDOWN_REBOOT,   T_BEFORE_REBOOT
        dd SSSF_SHUTDOWN_KERNEL,   T_BEFORE_KERNEL
then_actions_end:

then_key        db THEN_KEY
.len = $ - then_key

default_name    db "kolibri.img", 0

; @notify 'RDsave\ntext' -tO or -tE: the title, the ok or the error icon
notify_start    db "'RDsave", 10, 0
notify_ok       db "' -tO", 0
notify_error    db "' -tE", 0

notify_run:
        dd      SSF_START_APP
        dd      0
        dd      notify_buf
        dd      0, 0
        db      "/sys/@notify", 0

OpenDialog_data:
        .type           dd OPENDIALOG_SAVE
        .procinfo       dd procinfo
        .com_area_name  dd opendialog_area
        .com_area       dd 0
        .opendir_path   dd temp_dir_path
        .default_dir    dd opendialog_dir
        .start_path     dd opendialog_path
        .draw_window    dd draw_window
        .status         dd 0
        .openfile_path  dd dialog_path
        .filename_area  dd filename_area
        .filter_area    dd filter
        .x_size         dw DIALOG_W
        .x_start        dw DIALOG_X
        .y_size         dw DIALOG_H
        .y_start        dw DIALOG_Y

opendialog_area db "FFFFFFFF_open_dialog", 0
opendialog_path db "/sys/File Managers/opendial", 0
opendialog_dir  db "/", 0

; the size of the filter with this dword, the extensions, 0
filter:
        dd .end - filter
        db "IMG", 0
        db "IMA", 0
.end:
        db 0

; ====================================================================

; the texts of a language: zero terminated strings in this order. The
; window texts are UTF-8, a title starts with 3 (UTF-8 for SF 0 and 71);
; the @notify texts are cp866, no Spanish accents there: 'saved', the
; file system errors 1..11, an unknown error, a file on the RAM-disk. An
; error without a text shows the unknown one: 1 and 4 the kernel does not
; return (fs_lfn.inc ERROR_DISK_BASE, ERROR_PARTITION are not used), 6, 7
; and 9 give the user nothing to act on
T_GROUP         = 0
T_FLOPPY        = 1
T_NO_FILE       = 2
T_CHOOSE        = 3
T_SAVE          = 4
T_DONT_SAVE     = 5
T_CANCEL        = 6
T_SAVING        = 7
T_BEFORE_OFF    = 8
T_BEFORE_REBOOT = 9
T_BEFORE_KERNEL = 10
T_SAVED         = 11
T_ERRORS        = 12                    ; error N is T_ERRORS - 1 + N
T_UNKNOWN       = T_ERRORS + ERR_DEVICE
T_ON_RAMDISK    = T_UNKNOWN + 1         ; ERR_ON_RAMDISK

; the texts of every kernel language, English where there is no translation
texts           dd      texteng, texteng, texteng, textrus, texteng, texteng
                dd      texteng, texteng, texteng, textspa, texteng
assert ($ - texts)/4 = LANGUAGES

texteng         db      'Where to save the system image', 0
                db      'floppy A', 0, 'file not selected', 0, 'Choose...', 0
                db      'Save', 0, "Don't save", 0, 'Cancel', 0
                db      3, 'Saving the image...', 0
                db      3, 'Before power off', 0
                db      3, 'Before reboot', 0
                db      3, 'Before kernel restart', 0
                db      'The system image is saved', 0
                db      0                                       ; 1
                db      'The file system does not support saving', 0
                db      'Unknown file system', 0
                db      0                                       ; 4
                db      'Incorrect path', 0
                db      0                                       ; 6
                db      0                                       ; 7
                db      'Disk is full', 0
                db      0                                       ; 9
                db      'Access denied', 0
                db      'Device error', 0
                db      'Unknown error', 0
                db      'The image cannot be saved on the RAM-disk itself', 0

textspa         db      'Dónde guardar la imagen del sistema', 0
                db      'disquete A', 0, 'archivo no seleccionado', 0, 'Elegir...', 0
                db      'Guardar', 0, 'No guardar', 0, 'Cancelar', 0
                db      3, 'Guardando la imagen...', 0
                db      3, 'Antes de apagar', 0
                db      3, 'Antes de reiniciar', 0
                db      3, 'Antes de reiniciar el núcleo', 0
                cp866   'La imagen del sistema se ha guardado', 0
                db      0                                       ; 1
                cp866   'El sistema de archivos no permite guardar', 0
                cp866   'Sistema de archivos desconocido', 0
                db      0                                       ; 4
                cp866   'Ruta incorrecta', 0
                db      0                                       ; 6
                db      0                                       ; 7
                cp866   'Disco lleno', 0
                db      0                                       ; 9
                cp866   'Acceso denegado', 0
                cp866   'Error de dispositivo', 0
                cp866   'Error desconocido', 0
                cp866   'La imagen no se puede guardar en el propio disco RAM', 0

textrus         db      'Куда сохранить образ системы', 0
                db      'дискета A', 0, 'файл не выбран', 0, 'Выбрать...', 0
                db      'Сохранить', 0, 'Не сохранять', 0, 'Отмена', 0
                db      3, 'Сохранение образа...', 0
                db      3, 'Перед выключением', 0
                db      3, 'Перед перезагрузкой', 0
                db      3, 'Перед перезапуском ядра', 0
                cp866   'Образ системы сохранён', 0
                db      0                                       ; 1
                cp866   'Файловая система не поддерживает запись', 0
                cp866   'Неизвестная файловая система', 0
                db      0                                       ; 4
                cp866   'Неверный путь', 0
                db      0                                       ; 6
                db      0                                       ; 7
                cp866   'Диск переполнен', 0
                db      0                                       ; 9
                cp866   'Доступ запрещён', 0
                cp866   'Ошибка устройства', 0
                cp866   'Неизвестная ошибка', 0
                cp866   'Образ нельзя сохранить на сам RAM-диск', 0

; ====================================================================

align 4
text            dd ?                    ; the texts of the window language
then            dd 0                    ; SSSF_SHUTDOWN_* of "-then N", 0 without
title           dd program_name         ; the window title
target          dd ?
hasFloppy       dd 0
notifyText      dd ?
notifyEnd       dd ?
textColor       dd ?                    ; work_text with FONT
buttonTextColor dd ?                    ; work_button_text with FONT
buttonBorder    dd ?                    ; the border colour of a kernel button

sc              system_colors

notify_buf      rb NOTIFY_MAX
fname_buf       rb PATH_MAX
dialog_path     rb PATH_MAX
temp_dir_path   rb PATH_MAX
filename_area   rb NAME_MAX
procinfo        rb PROCINFO_SIZE
