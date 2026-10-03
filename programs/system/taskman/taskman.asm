; SPDX-License-Identifier: GPL-2.0-only
;
; Task manager. A tab is a row of the `tabs` table: its caption, list
; and handlers. To add a tab, add a row and an .inc file with the handlers.

include "../../KOSfuncs.inc"
include "../../macros.inc"
include "../../develop/libraries/box_lib/box_lib.mac"
include "../../develop/libraries/libs-dev/libio/libio.inc"

format meos 01
entry start

include "gui.inc"
include "hwmon.inc"
include "list.inc"
include "locale.inc"
include "sensors.inc"
include "stats.inc"
include "window.inc"

include "tabs/autorun.inc"
include "tabs/disks.inc"
include "tabs/drivers.inc"
include "tabs/perf.inc"
include "tabs/procs.inc"

;-------------------------------------------------------------------------------
start:
        mcall   SF_SET_EVENTS_MASK, EVENTS

        mcall   SF_SYS_MISC, SSF_LOAD_DLL, box_lib_path
        test    eax, eax
        jz      exit
        xchg    ecx, eax
        mov     ebp, box_lib            ; the names become the procs

.import:
        mov     ebx, ecx
.export:
        mov     esi, [ebx]
        test    esi, esi
        jz      exit
        add     ebx, 8
        mov     edi, [ebp]
@@:
        lodsb
        scasb
        jne     .export
        test    al, al
        jnz     @b
        mov     edx, [ebx - 4]
        mov     [ebp], edx
        add     ebp, 4
        cmp     dword [ebp], 0
        jne     .import

        mcall   SF_SYS_MISC, SSF_HEAP_INIT
        mcall   SF_SYS_MISC, SSF_MEM_OPEN, icons_name, 0, 0     ; of @RESHARE
        mov     [icons], eax

        call    sensors_init

reopen:
        call    open_tab
redraw:
        call    draw_window

still:
        mcall   SF_SYSTEM_GET, SSF_TIME_COUNT
        mov     ebx, [next_tick]
        sub     ebx, eax
        jg      .wait

        add     eax, TICK
        mov     [next_tick], eax
        call    sample

        mcall   SF_THREAD_INFO, pinfo, -1
        test    [pinfo.wnd_state], WND_MINIMIZED + WND_ROLLED_UP
        jnz     still
        mov     eax, TAB.update
        call    tab_call
        jmp     still

.wait:
        cmp     [repeat_at], 0          ; an arrow held: the mouse again soon
        je      @f
        mov     ebx, 4

@@:
        mcall   SF_WAIT_EVENT_TIMEOUT
        dec     eax                     ; EV_REDRAW
        jz      redraw
        dec     eax                     ; EV_KEY
        jz      key
        dec     eax                     ; EV_BUTTON
        jz      button

        cmp     al, EV_MOUSE - EV_BUTTON        ; the mouse, or the time with
        je      .mouse                          ; it held
        cmp     al, EV_IDLE - EV_BUTTON
        jne     still

        cmp     [repeat_at], 0          ; a held arrow when it is time
        je      still
        dec     [repeat_at]
        jnz     still

.mouse:
        mov     eax, TAB.mouse
        call    tab_call
        jmp     still

key:
        mcall   SF_GET_KEY
        shr     eax, 8

        cmp     al, KEY_ESC
        je      exit
        cmp     al, KEY_TAB             ; the next tab
        je      next_tab

        call    tab_list                ; other keys are for lists
        jz      still
        call    list_key
        jmp     still

button:
        mcall   SF_GET_BUTTON
        shr     eax, 8

        cmp     eax, 1
        je      exit

        cmp     eax, BTN_OWN
        jae     .own

        sub     eax, BTN_TAB
        cmp     eax, BTN_SHUTDOWN - BTN_TAB
        jb      switch_tab

        mov     esi, sz_end_path
        call    run_plain
        jmp     still

.own:
        xchg    eax, edx
        mov     eax, TAB.button
        call    tab_call
        jmp     still

next_tab:
        mov     eax, [cur_tab]
        inc     eax
        cmp     eax, NTABS
        jb      switch_tab
        xor     eax, eax                ; then switch_tab

switch_tab:                             ; eax = tab
        cmp     eax, [cur_tab]          ; the open one: nothing to do
        je      still
        push    eax
        call    tab_list                ; the old tab keeps its list
        mov     esi, list
        call    copy_list
        pop     [cur_tab]
        jmp     reopen

exit:
        call    stop_walker
        mcall   SF_TERMINATE_PROCESS

; Open the current tab: its list, then its activate handler.
open_tab:
        and     [repeat_at], 0
        call    stop_walker

        call    tab_list
        mov     esi, edi
        mov     edi, list
        call    copy_list

        mov     eax, TAB.activate
tab_call:                               ; eax = handler in TAB
        imul    ecx, [cur_tab], sizeof.TAB
        movzx   ecx, word [tabs + ecx + eax]
        jmp     ecx

nothing:
        ret

; out: edi -> list of the current tab or 0, ZF = 1 if 0
tab_list:
        imul    edi, [cur_tab], sizeof.TAB
        movzx   edi, word [tabs + edi + TAB.list]
        test    edi, edi
        ret

; Copy the list at esi to edi unless ZF = 1: the tab has none.
copy_list:
        jz      @f
        mov     ecx, sizeof.LIST / 4
        rep movsd
@@:
        ret

;-------------------------------------------------------------------------------
; Settings
MIN_W           = 640                   ; fits the longest bottom bar (ru_RU)
MIN_H           = 400                   ; four graphs are still readable
TICK            = 100                   ; statistics period, 1/100 s
HIST_LEN        = 1024                  ; power of 2, the bars of a 4K wide window
MAX_PROCS       = 256

; Layout
PAD             = 8                     ; margins and gaps
TOP             = 12                    ; above the tabs
TAB_H           = 24
AREA_Y          = TOP + TAB_H - 1       ; top of the panel, the bottom of the tabs
BTN_H           = 24
BTN_PAD         = 12                    ; text to the sides of a button
CHECK           = 14                    ; checkbox
ICON            = 18                    ; icons of @RESHARE
TEXT_CHARS      = 63                    ; the longest text drawn
TEXT_H          = 14                    ; 8x16 glyphs look centered as 14 lines

; The window and the text
EVENTS          = EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER
WIN_STYLE       = 0x73000000            ; skinned, a caption, coordinates in
                                        ; the client area, not filled: drawn
TEXT_UTF8       = 0xB0000000            ; asciiz, UTF-8, 8x16
WND_MAXIMIZED   = 1                     ; process_information.wnd_state
WND_MINIMIZED   = 2
WND_ROLLED_UP   = 4

; Lists
COL_W           = 112
ID_W            = 3*8 + 2*CELL_PAD      ; a number: ID, Nº or #, the arrow

KIND_TEXT       = 0                     ; zero ended text in the row, sorted ascending
KIND_STR        = 1                     ; pointer to text
KIND_NUM        = 2
KIND_PCT        = 3                     ; from here on sorted down at first
KIND_SIZE       = 4
KIND_HEX        = 5
KIND_BAR        = 6                     ; 1/1024 of the cell filled
KIND_ICON       = 7                     ; in ICONS18

HDR_H           = 24 - 2                ; 24 with its lines, as an arrow
ROW_H           = 20
SB_W            = 24                    ; scrollbar, as high as the list
CELL_PAD        = 8

; Keys and mouse buttons
KEY_TAB         = 9
KEY_ENTER       = 13
KEY_SPACE       = 32
KEY_ESC         = 27
KEY_DOWN        = 177                   ; the moves in the lists up to
KEY_DEL         = 182
KEY_PGUP        = 184                   ; here
MB_LEFT         = 1                     ; held
MB_LEFT_DOWN    = 0x100                 ; pressed, not held at the last event
MB_RIGHT_DOWN   = 0x200

; pusha: where it keeps the registers
PUSHA_EDI       = 0
PUSHA_EBX       = 16
PUSHA_EDX       = 20
PUSHA_ECX       = 24
PUSHA_EAX       = 28
PUSHA_SIZE      = 32

; Buttons
BTN_TAB         = 2
BTN_SHUTDOWN    = 8                     ; ids with bit 3 go to the right
BTN_OWN         = 16                    ; ids from here on belong to the tab

struct TAB                              ; a row of `tabs`
        caption         dw ?
        list            dw ?            ; LIST or 0
        activate        dw ?            ; the tab is opened
        draw            dw ?            ; full redraw of the panel
        update          dw ?            ; new statistics
        button          dw ?            ; edx = button id
        mouse           dw ?
        bar             dw ?            ; the items of its bottom bar
ends

;-------------------------------------------------------------------------------
tabs:
        TAB     sz_tab_procs, procs_list, nothing, procs_draw, procs_update, procs_button, list_mouse, procs_bar
        TAB     sz_tab_perf, 0, nothing, perf_draw, perf_draw, perf_cpuid, get_mouse, perf_bar
        TAB     sz_tab_disks, disks_list, disks_scan, list_draw, disks_update, disks_open, list_mouse, disks_bar
        TAB     sz_tab_drivers, drivers_list, drivers_scan, list_draw, nothing, drivers_load, list_mouse, drivers_bar
        TAB     sz_tab_autorun, autorun_list, autorun_load, list_draw, nothing, autorun_toggle, list_mouse, autorun_bar
NTABS = ($ - tabs) / sizeof.TAB

app_info        FileInfoBlock SSF_START_APP, 0, 0, 0, 0, 0, 0     ; the parameters, the path
sz_run_path     db      "/sys/RUN", 0
sz_end_path     db      "/sys/END", 0
sz_tinfo_path   db      "/sys/TINFO", 0
icons_name      db      "ICONS18", 0
box_lib_path    db      "/sys/lib/box_lib.obj", 0
align 4
box_lib:
import  box_lib, \
        scrollbar_v_draw, "scrollbar_v_draw", \
        scrollbar_v_mouse, "scrollbar_v_mouse", \
        check_box_draw2, "check_box_draw2"
sb      scrollbar SB_W - 1, 0, 0, AREA_Y, SB_W - 1, 0, 0, 0, 0, 0, 0, 1   ; the sizes - 1:
                                        ; box_lib draws a pixel more
cb      check_box2 0, 0, PAD, 0, 0, 0x80000000, cb.size_of_str, 0      ; no text: its length

IncludeUGlobals
align 4
sc              system_colors           ; the button color: the main one
green           dd      ?               ; of the button: run
red             dd      ?               ; and of high values, end
mid             dd      ?               ; the list header, the scrollbar
pinfo           process_information
next_tick       dd      ?
cur_tab         dd      ?
buttons         dd      ?               ; mouse buttons held at the last event
area_w          dd      ?               ; panel: x = PAD, y = AREA_Y
area_h          dd      ?
bar_y           dd      ?               ; bottom bar
bar_left        dd      ?
bar_right       dd      ?               ; left end of the right buttons
icons           dd      ?               ; ICONS18 or 0
icon_buf        rd      ICON*ICON
text_buf        rb      TEXT_CHARS * 4 + 1      ; UTF-8 takes up to 4 bytes

macro max_of name, [value] {
 common name = 0
 forward
  if value > name
   name = value
  end if
}
max_of TAB_DATA, PROCS_DATA, DISKS_DATA, DRIVERS_DATA, AUTORUN_DATA
align 4
tab_data        rb      TAB_DATA        ; data of the open tab
