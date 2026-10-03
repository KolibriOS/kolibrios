; List of a tab: rows in columns with a header, a selected row and a scrollbar on
; the panel. The columns go from the left, one of them, width 0, takes what
; the others leave. A click on a row selects it, then calls the trigger of
; that button; a key of the hotkeys calls its trigger. `list` is the list of
; the open tab; a closed tab keeps it in its descriptor.

struct LIST
        cols            dd ?            ; the columns
        cols_end        dd ?
        sort            dd ?            ; the sorted column
        left            dd ?            ; the triggers: a left click,
        right           dd ?            ; a right click,
        keys            dd ?            ; -> HOTKEYs, ended by 0
        dim             dd ?            ; a byte in the row: not 0, it is pale; 0: none
        count           dd ?
        sel             dd ?
        first           dd ?
        key             dd ?            ; the first dword of the selected row
        desc            dd ?            ; 1: the sorted column goes down
        view            dd ?            ; -> the pointers to the rows
ends

struct HOTKEY
        key             db ?
        trigger         dw ?
ends

struct COL
        caption         dd ?
        field           db ?            ; offset in the row
        kind            db ?
        width           db ?            ; 0: what the others leave
ends

; The list: the header, the rows and the scrollbar.
list_draw:
        mov     ebx, [area_w]           ; what the others leave to the widest
        sub     ebx, 1 + SB_W
        mov     esi, [list.cols]

@@:
        movzx   edx, byte [esi + COL.width]
        sub     ebx, edx
        add     esi, sizeof.COL
        cmp     esi, [list.cols_end]
        jb      @b
        mov     [col_rest], ebx

        mov     eax, [area_h]           ; the rows that fit
        sub     eax, 2 + HDR_H + 1
        xor     edx, edx
        mov     ecx, ROW_H
        div     ecx
        mov     [rows], eax

        mov     ebx, PAD shl 16         ; the header and the line under it,
        mov     bx, word [area_w]       ; left of the scrollbar
        sub     bx, SB_W - 1
        mov     ecx, (AREA_Y shl 16) + HDR_H + 2
        mov     edx, [mid]
        call    frame_rect

        xor     eax, eax
        mov     ebp, AREA_Y + 1 + (HDR_H - TEXT_H)/2
        call    list_line               ; then list_rows

; The rows and the scrollbar only, for new data.
list_rows:
        call    list_clamp
        mov     edi, [list.first]
        mov     ecx, ((AREA_Y + HDR_H + 2) shl 16) + ROW_H
        mov     ebp, [rows]

.row:
        mov     ebx, (PAD + 1) shl 16
        mov     bx, word [area_w]
        sub     bx, 1 + SB_W

        mov     edx, [sc.work_light]
        mov     esi, [sc.work_text]
        cmp     edi, [list.sel]
        jne     @f
        cmp     edi, [list.count]       ; a row there
        jae     @f
        mov     edx, [sc.work_button]
        mov     esi, [sc.work_button_text]

@@:
        mcall   SF_DRAW_RECT

        cmp     edi, [list.count]
        jae     @f

        pusha
        mov     eax, [list.view]
        mov     eax, [eax + edi*4]
        mov     ebp, ecx
        shr     ebp, 16
        add     ebp, (ROW_H - TEXT_H)/2
        mov     ecx, esi

        mov     esi, [list.dim]         ; pale: half way to the background
        test    esi, esi
        jz      .line
        cmp     byte [eax + esi], 0
        je      .line

        xchg    eax, ecx
        call    mix
        xchg    eax, ecx

.line:
        call    list_line
        popa

@@:
        add     ecx, ROW_H shl 16
        inc     edi
        dec     ebp
        jg      .row                    ; then sb_draw

; The scrollbar of box_lib at the right of the list, as high as it
sb_draw:
        call    sb_set
        push    sb
        call    [scrollbar_v_draw]
        ret

; One line of the list at y = ebp: the captions if eax = 0, lines between
; them, the sorted one with an arrow, otherwise the cells of row eax in color
; ecx on background edx. Left or right as col_left says.
list_line:
        pusha
        mov     ebx, PAD + 1            ; the left of the column
        mov     esi, [list.cols]

.col:
        pusha

        movzx   edx, byte [esi + COL.field]
        add     edx, eax
        test    eax, eax
        jnz     .cell

        mov     edx, [esi + COL.caption]
        mov     ecx, [sc.work_text]
        jmp     .draw

.cell:
        call    cell_text

.draw:
        call    col_width               ; the text in the cell
        lea     eax, [ebx + eax - CELL_PAD]
        xchg    eax, ebx
        add     eax, CELL_PAD
        mov     edi, text_buf
        call    fmt_clip

        mov     edx, edi
        call    col_left
        jc      @f
        mov     eax, edx
        call    text_width
        neg     eax
        add     eax, ebx

@@:
        shl     eax, 16
        mov     ax, bp
        xchg    eax, ebx
        call    draw_text

        cmp     dword [esp + PUSHA_EAX], 0      ; the captions
        jne     .picture

        cmp     esi, [list.cols]        ; a line left of the column, but the
        je      @f                      ; first one at the frame
        push    ebx
        mov     ebx, [esp + 4 + PUSHA_EBX]      ; the left
        shl     ebx, 16
        inc     ebx
        mov     ecx, ((AREA_Y + 1) shl 16) + HDR_H
        mcall   SF_DRAW_RECT, , , [sc.work_graph]
        pop     ebx

@@:
        cmp     esi, [list.sort]
        jne     .next
        mov     edi, [list.desc]

        sub     ebx, 5 shl 16           ; 2 pixels before the caption on the
        call    col_left                ; right, after the one on the left
        jnc     @f

        mov     eax, text_buf
        call    text_width
        add     eax, 11
        shl     eax, 16
        add     ebx, eax

@@:
        call    draw_arrow
        jmp     .next

.picture:                               ; a bar or an icon in a row
        movzx   edx, byte [esi + COL.field]
        add     edx, [esp + PUSHA_EAX]  ; the row
        mov     edx, [edx]
        call    col_width
        cmp     byte [esi + COL.kind], KIND_BAR
        jb      .next
        ja      .icon

        mov     ebx, [esp + PUSHA_EBX]  ; the bar across the cell
        add     ebx, CELL_PAD
        shl     ebx, 16
        sub     eax, 2*CELL_PAD
        mov     bx, ax
        movzx   ecx, bp
        shl     ecx, 16
        mov     cx, TEXT_H
        xchg    eax, edx
        mov     edx, [sc.work_light]
        call    frame_rect

        cmp     eax, -1
        je      .next
        add     ebx, 2 shl 16 - 4       ; a light line around it, from the
        add     ecx, 2 shl 16 - 4       ; selected row too
        movzx   edx, bx
        mul     edx
        shr     eax, 10
        mov     bx, ax
        mcall   SF_DRAW_RECT, , , [sc.work_button]      ; as the graphs
        jmp     .next

.icon:
        sub     eax, ICON               ; in the middle of the cell
        shr     eax, 1
        add     eax, [esp + PUSHA_EBX]
        shl     eax, 16
        lea     ecx, [ebp - (ICON - TEXT_H)/2]
        mov     ax, cx
        xchg    eax, ebx
        xchg    eax, edx
        mov     edx, [esp + PUSHA_SIZE + PUSHA_EDX]     ; of the first one:
                                        ; the background
        call    draw_icon

.next:
        call    col_width               ; the next column after this one
        add     [esp + PUSHA_EBX], eax  ; its left
        popa

        add     esi, sizeof.COL
        cmp     esi, [list.cols_end]
        jb      .col
        popa
        ret

; esi -> column, edx -> its field in the row; out: edx -> the text of the
; cell, "" for a bar or an icon
cell_text:
        cmp     byte [esi + COL.kind], KIND_STR
        jb      .done
        mov     edx, [edx]
        je      .done

        mov     eax, edx
        mov     edi, text_buf
        mov     edx, edi
        mov     byte [edi], 0           ; -1: unknown, nothing to show
        cmp     eax, -1
        je      .done

        cmp     byte [esi + COL.kind], KIND_SIZE
        jb      .uint
        ja      .hex
        jmp     fmt_size

.hex:
        cmp     byte [esi + COL.kind], KIND_HEX ; a bar or an icon: drawn below
        jne     .done
        jmp     fmt_hex

.uint:
        call    fmt_uint
        cmp     byte [esi + COL.kind], KIND_PCT
        jne     .done
        mov     word [edi], "%"

.done:
        ret

; esi -> column; out: CF = 1 if its caption and text go left: text and the
; widest column, but the bar
col_left:
        cmp     byte [esi + COL.kind], KIND_NUM ; text: CF = 1
        jb      .done
        cmp     byte [esi + COL.kind], KIND_BAR ; the bar and icons: CF = 0
        jae     .done
        cmp     byte [esi + COL.width], 1       ; the widest: CF = 1

.done:
        ret

; esi -> column; out: eax = its width
col_width:
        movzx   eax, byte [esi + COL.width]
        test    eax, eax
        jnz     @f
        mov     eax, [col_rest]

@@:
        ret

; The end of a scan: ebp -> the end of the view.
list_done:
        sub     ebp, [list.view]
        shr     ebp, 2
        mov     [list.count], ebp       ; then list_sort

; Sort the view by the sorted column and find the selected row again.
list_sort:
        mov     ebp, [list.view]
        mov     ecx, 1                  ; insertion sort, stable

.outer:
        cmp     ecx, [list.count]
        jae     .select
        mov     esi, [ebp + ecx*4]
        mov     ebx, ecx

.inner:
        mov     edi, [ebp + ebx*4 - 4]
        call    list_less
        jnc     .put
        mov     [ebp + ebx*4], edi
        dec     ebx
        jnz     .inner

.put:
        mov     [ebp + ebx*4], esi
        inc     ecx
        jmp     .outer

.select:
        mov     ecx, [list.count]
        mov     eax, [list.key]

@@:
        dec     ecx
        js      @f
        mov     esi, [ebp + ecx*4]
        cmp     [esi], eax
        jne     @b
        mov     [list.sel], ecx

@@:
        ret

; esi, edi -> rows; out: CF = 1 if esi goes before edi
list_less:
        pusha
        mov     ebx, [list.sort]
        movzx   ecx, byte [ebx + COL.field]
        add     esi, ecx
        add     edi, ecx

        cmp     [list.desc], 0
        je      @f
        xchg    esi, edi

@@:
        cmp     byte [ebx + COL.kind], KIND_STR
        ja      .num
        jb      @f
        mov     esi, [esi]              ; the text by its pointer
        mov     edi, [edi]

@@:
        lodsb                           ; the letters of either case alike
        mov     ah, [edi]
        inc     edi
        mov     dl, al
        or      ax, 0x2020
        cmp     al, ah
        jne     @f
        test    dl, dl
        jz      @f
        cmp     byte [edi - 1], 0       ; the end of edi, not a blank
        jne     @b

@@:
        popa
        ret

.num:                                   ; -1, unknown, before 0
        mov     eax, [esi]
        inc     eax
        mov     edx, [edi]
        inc     edx
        cmp     eax, edx
        popa
        ret

; An arrow up, or down if edi = 1: a triangle of 4 lines, line i of 2*i + 1
; pixels, its tip at x = ebx shr 16, around the middle of the capitals of
; text at y = bx.
draw_arrow:
        pusha
        lea     ecx, [edi*2 + edi + TEXT_H/2 - 2]       ; from the top if it
        add     cx, bx                  ; goes up, else from the bottom
        shl     ecx, 16
        inc     ecx

        shl     edi, 17                 ; the next line: 1 down or up
        neg     edi
        add     edi, 1 shl 16

        mov     bx, 1
        mov     esi, 4

@@:
        mcall   SF_DRAW_RECT, , , [sc.work_text]
        add     ebx, (-1 shl 16) + 2
        add     ecx, edi
        dec     esi
        jnz     @b

        popa
        ret

; eax = rows the wheel moves, esi = mouse buttons: box_lib moves the list,
; again while held; out: ebp = list.first before, drawn again if it changes
scroll_mouse:
        mov     ebp, [list.first]
        add     [list.first], eax

        call    sb_set

        xor     eax, eax
        mov     edi, [rows]             ; all rows seen: no thumb to drag
        cmp     edi, [list.count]
        jae     @f

        push    sb
        call    [scrollbar_v_mouse]

        mov     eax, [sb.position]
        xchg    eax, [list.first]
        sub     eax, [list.first]       ; moved while held: again after 10
        jz      @f                      ; waits of the main loop if pressed

        mov     eax, esi                ; now, else after 2
        and     eax, MB_LEFT
        jz      @f
        mov     al, 2
        test    esi, MB_LEFT_DOWN
        jz      @f
        mov     al, 10

@@:
        mov     [repeat_at], eax
        ret

sb_set:                                 ; the list state to it
        mov     eax, [area_w]
        add     eax, PAD - SB_W
        mov     [sb.x_pos], ax
        mov     eax, [area_h]
        mov     [sb.y_size], ax

        mov     [sb.all_redraw], 1
        mov     edi, sb.max_area

        mov     eax, [list.count]
        stosd
        mov     eax, [rows]
        stosd
        mov     eax, [list.first]
        stosd
        mov     eax, [sc.work_dark]
        stosd
        mov     eax, [mid]
        stosd
        mov     eax, [sc.work_graph]
        stosd
        ret

; Keep sel and first in range and remember the selected row.
list_clamp:
        pusha
        mov     ecx, [list.count]
        mov     edi, list.sel
        call    clamp

        jecxz   @f
        mov     edx, [list.view]
        mov     eax, [edx + eax*4]
        mov     eax, [eax]
        mov     [list.key], eax

@@:
        popa

        pusha                           ; first in range
        mov     ecx, [list.count]
        sub     ecx, [rows]
        inc     ecx
        mov     edi, list.first
        call    clamp
        popa
        ret

clamp:                                  ; [edi] into [0, ecx - 1]
        mov     eax, [edi]
        cmp     eax, ecx
        jl      @f
        lea     eax, [ecx - 1]

@@:
        test    eax, eax
        jns     @f
        xor     eax, eax

@@:
        stosd
        ret

list_key:                               ; al = key
        mov     esi, [list.keys]

.hotkey:
        mov     ah, [esi + HOTKEY.key]
        test    ah, ah
        jz      .move
        add     esi, sizeof.HOTKEY
        cmp     ah, al
        jne     .hotkey
        movzx   eax, [esi - sizeof.HOTKEY + HOTKEY.trigger]
        jmp     eax

.move:
        sub     al, KEY_DOWN
        cmp     al, KEY_PGUP - KEY_DOWN
        ja      .done

        movzx   eax, al
        movsx   edx, byte [.moves + eax*2 + 1]
        imul    edx, [rows]
        movsx   eax, byte [.moves + eax*2]
        add     edx, eax

        add     [list.sel], edx
        call    list_clamp

        mov     ebx, [list.sel]         ; scroll to the selection
        mov     ecx, [rows]
        sub     ebx, ecx
        inc     ebx
        sub     [list.first], ebx
        mov     edi, list.first
        call    clamp
        add     [list.first], ebx
        jmp     list_rows

.done:
        ret

; rows, pages for the keys KEY_DOWN..KEY_PGUP: down, up, right, home, end,
; del, pgdn, pgup
.moves  db      1, 0,  -1, 0,  0, 0,  0, -128,  0, 127,  0, 0,  0, 1,  0, -1

; Wheel scrolls; left button selects a row, sorts by a column or moves the
; scrollbar; right click opens the row.
list_mouse:
        call    get_mouse
        imul    eax, 3
        call    scroll_mouse            ; every event: it follows a drag

        test    esi, MB_LEFT + MB_RIGHT_DOWN
        jz      .moved

        sub     ebx, PAD                ; inside the list, left of the
        jle     .moved                  ; scrollbar
        mov     eax, [area_w]
        sub     eax, SB_W
        cmp     ebx, eax
        jge     .moved

        sub     ecx, AREA_Y + HDR_H + 2
        jl      .header

        test    esi, MB_LEFT_DOWN + MB_RIGHT_DOWN       ; a row: not dragged to
        jz      .moved

        xchg    eax, ecx
        xor     edx, edx
        mov     ecx, ROW_H
        div     ecx
        cmp     eax, [rows]
        jae     .moved

        add     eax, [list.first]
        cmp     eax, [list.count]
        jae     .moved

        mov     [list.sel], eax
        shr     esi, 9                  ; MB_RIGHT_DOWN: 0 left, 1 right
        and     esi, 1
        push    [list.left + esi*4]     ; the selection drawn, then list_rows
        jmp     list_rows               ; returns to the trigger

.moved:                                 ; a drag of the thumb keeps its pixels
        cmp     ebp, [list.first]
        jne     list_rows
        ret

.header:
        cmp     ecx, -(HDR_H + 1)
        jl      .moved
        test    esi, MB_LEFT_DOWN
        jz      .moved

        mov     esi, [list.cols]        ; the column under the cursor
        dec     ebx

.col:
        call    col_width
        sub     ebx, eax
        jl      .sort
        add     esi, sizeof.COL
        cmp     esi, [list.cols_end]
        jb      .col
        ret

.sort:                                  ; again: the other way, else its own
        xor     eax, eax
        cmp     byte [esi + COL.kind], KIND_PCT
        setae   al
        cmp     esi, [list.sort]
        jne     .order
        mov     eax, [list.desc]
        xor     al, 1

.order:
        mov     [list.desc], eax
        mov     [list.sort], esi
        call    list_sort
        jmp     list_draw

; out: esi = mouse buttons held, in bits 8.. the ones pressed, not held at
; the last event, and the mouse not moved since; ebx, ecx = x, y in the
; client area; eax = the wheel
get_mouse:
        mcall   SF_MOUSE_GET, SSF_BUTTON_EXT
        mov     esi, eax
        movzx   eax, al
        xchg    al, byte [buttons]
        shl     eax, 8
        not     eax
        and     esi, eax

        mcall   SF_MOUSE_GET, SSF_WINDOW_POSITION
        movsx   ecx, ax                 ; in the client area
        sar     eax, 16
        xchg    edx, eax

        mcall   SF_MOUSE_GET, SSF_SCROLL_DATA
        mov     ebx, edx
        movsx   eax, ax                 ; the wheel
        ret

no_keys         db      0               ; HOTKEYs of a list without them

uglobal
align 4
list            LIST
rows            dd      ?               ; that fit the panel
col_rest        dd      ?               ; the width of the widest column
repeat_at       dd      ?               ; waits till a held arrow repeats
endg
