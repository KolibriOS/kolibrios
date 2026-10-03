; Drawing and formatting helpers; they keep all registers unless noted.

macro __mov reg, a, b {                 ; mcall loads small constants shorter
 if (~a eq) & (~b eq)
   mpack reg, a, b
 else if (~a eq) & (b eq)
   movi reg, a
 end if
}

; An item of a bar: a button with its color, a checkbox with its state or
; a gap
struct BAR
        kind            db ?
        text            dd ?            ; the caption
        arg             dd ?            ; -> the color, -> the state as a byte
ends
BAR_END         = 0
BAR_BUTTON      = 1
BAR_CHECK       = 2
BAR_GAP         = 3

; ebx = x shl 16 + y, ecx = color, edx -> string in UTF-8
draw_text:
        pusha
        or      ecx, TEXT_UTF8
        mcall   SF_DRAW_TEXT
        popa
        ret

; eax -> string; out: eax = width in pixels
text_width:
        push    ecx esi
        mov     esi, eax
        xor     ecx, ecx

.next:
        lodsb
        test    al, al
        jz      .done
        call    utf8_more
        je      .next
        add     ecx, 8
        jmp     .next

.done:
        xchg    eax, ecx
        pop     esi ecx
        ret

; eax -> caption; out: eax = width of the button
button_width:
        call    text_width
        add     eax, BTN_PAD*2
        ret

; Frame in the border color filled with edx. ebx = x shl 16 + w, ecx = y shl 16 + h
frame_rect:
        pusha
        push    edx
        mcall   SF_DRAW_RECT, , , [sc.work_graph]
        pop     edx

        add     ebx, 1 shl 16 - 2
        add     ecx, 1 shl 16 - 2
        mcall   SF_DRAW_RECT
        popa
        ret

; eax = icon, ebx = x shl 16 + y, edx = the background for its white
draw_icon:
        pusha
        mov     esi, [icons]
        test    esi, esi
        jz      .done

        imul    eax, ICON*ICON*4
        add     esi, eax
        mov     edi, icon_buf
        mov     ecx, ICON*ICON
@@:
        lodsd
        cmp     eax, -1
        jne     .pixel
        mov     eax, edx

.pixel:
        stosd
        loop    @b

        mov     edx, ebx
        mcall   SF_PUT_IMAGE_EXT, icon_buf, (ICON shl 16) + ICON, , 32, 0, 0

.done:
        popa
        ret

; eax, edx = colors; out: eax = the one half way between them, edx changed
mix:
        and     eax, 0xFEFEFE
        and     edx, 0xFEFEFE
        add     eax, edx
        shr     eax, 1
        ret

; eax -> caption, edx = id, esi = color
bar_button:
        pusha
        mov     edi, eax
        call    button_width
        call    bar_place

        dec     ebx                     ; the kernel draws one pixel more
        dec     ecx
        mcall   SF_DEFINE_BUTTON

        add     ebx, BTN_PAD shl 16
        mov     eax, [sc.work_button_text] ; then bar_text

; The end of the bar items: edi -> caption, eax = color, ebx = x shl 16,
; ecx = y shl 16 of the item
bar_text:
        shr     ecx, 16
        add     ecx, (BTN_H - TEXT_H)/2
        mov     bx, cx
        xchg    eax, ecx
        mov     edx, edi
        call    draw_text
        popa
        ret

; eax -> caption, edx = id, ebp = checked
bar_checkbox:
        pusha
        mov     edi, eax
        call    button_width            ; the box, PAD, the caption: 2 left
        call    bar_place
        add     [bar_left], 2*PAD + CHECK - 2*BTN_PAD   ; then more room

        or      edx, BT_HIDE + BT_NOFRAME
        mcall   SF_DEFINE_BUTTON

        mov     bx, CHECK
        add     ecx, ((BTN_H - CHECK)/2) shl 16
        mov     cx, CHECK
        mov     [cb.left_s], ebx        ; the box of box_lib, the caption
        mov     [cb.top_s], ecx         ; as the other ones

        mov     eax, [sc.work_light]
        mov     [cb.color], eax
        mov     eax, [sc.work_button]   ; the frame and the check
        mov     [cb.border_color], eax
        lea     eax, [ebp + ebp]        ; checked
        mov     [cb.flags], eax

        push    cb
        call    [check_box_draw2]

        add     ebx, (CHECK + PAD) shl 16
        sub     ecx, ((BTN_H - CHECK)/2) shl 16
        mov     eax, [sc.work_text]
        jmp     bar_text

; Place in the bottom bar: from the left end, or from the right one for ids
; with bit 3. eax = width, edx = id; out: ebx = x shl 16 + w, ecx = y shl 16 + h
bar_place:
        lea     ebx, [eax + PAD]
        test    dl, 8
        jnz     .right

        xchg    ebx, [bar_left]
        add     [bar_left], ebx
        jmp     @f

.right:
        sub     [bar_right], ebx
        mov     ebx, [bar_right]

@@:
        shl     ebx, 16
        mov     bx, ax

        mov     ecx, [bar_y]
        shl     ecx, 16
        mov     cx, BTN_H
        ret

; The items of a bar from the left, esi -> them; the buttons and the
; checkboxes get the ids from BTN_OWN on. Changes all registers.
bar_draw:
        mov     edx, BTN_OWN

.item:
        mov     bl, [esi + BAR.kind]
        mov     eax, [esi + BAR.text]
        mov     ecx, [esi + BAR.arg]
        add     esi, sizeof.BAR
        dec     bl
        js      .done
        jz      .button
        dec     bl
        jz      .check
        add     [bar_left], PAD         ; a gap
        jmp     .item

.done:
        ret

.button:
        push    esi
        mov     esi, [ecx]
        call    bar_button
        pop     esi
        inc     edx
        jmp     .item

.check:
        movzx   ebp, byte [ecx]
        call    bar_checkbox
        inc     edx
        jmp     .item


;-------------------------------------------------------------------------------
; Formatting: write to edi and leave it at the terminating zero.

; eax = number; printed as 8 hex digits
fmt_hex:
        pusha
        mov     ebx, 16
        mov     esi, 8
        jmp     fmt_num

; eax = number
fmt_uint:
        pusha
        mov     ebx, 10
        xor     esi, esi

fmt_num:                                ; ebx = base, esi = the fewest digits
        xor     ecx, ecx

@@:
        xor     edx, edx
        div     ebx
        push    edx
        inc     ecx
        test    eax, eax
        jnz     @b
        cmp     ecx, esi
        jb      @b

@@:
        pop     eax
        cmp     al, 10                  ; 0..15 to '0'..'9', 'A'..'F'
        sbb     al, 0x69
        das
        stosb
        loop    @b

        mov     [edi], cl
        mov     [esp + PUSHA_EDI], edi
        popa
        ret

; al = a byte of UTF-8; out: ZF = 1 if it goes on with a character
utf8_more:
        push    eax
        and     al, 0xC0
        cmp     al, 0x80
        pop     eax
        ret

; edx -> string; copies up to (ebx - eax) / 8 characters, TEXT_CHARS at
; most, and keeps edi
fmt_clip:
        pusha

        mov     esi, edx
        sub     ebx, eax
        shr     ebx, 3
        cmp     ebx, TEXT_CHARS
        jb      @f
        mov     ebx, TEXT_CHARS

@@:
        lodsb
        test    al, al
        jz      @f
        call    utf8_more
        je      .put
        dec     ebx
        js      @f

.put:
        stosb
        cmp     edi, text_buf + TEXT_CHARS*4
        jb      @b

@@:
        mov     byte [edi], 0
        popa
        ret

; esi -> string
fmt_str:
        push    eax esi
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        dec     edi
        pop     esi eax
        ret

; eax -> string
fmt_unit:
        xchg    esi, eax
        call    fmt_str
        xchg    esi, eax
        ret

; eax = KB; printed as "123 KB", "1.5 MB", "12.3 GB" etc.
fmt_size:
        pusha
        mov     ebx, 1024
        mov     esi, sz_units
        jmp     fmt_scaled

; eax = bits a second; printed as "500 bit/s", "1.5 Mbit/s" etc.
fmt_rate:
        pusha
        mov     ebx, 1000
        mov     esi, sz_bits            ; then fmt_scaled

; ebx = the step of the units, esi -> the units, each ended with 0: the
; number in the largest one it fills, small ones with a decimal but .0
fmt_scaled:
        xor     ecx, ecx
        xor     edx, edx

@@:
        cmp     eax, ebx
        jb      @f
        xor     edx, edx
        div     ebx
        inc     ecx
        jmp     @b

@@:
        call    fmt_uint

        cmp     eax, 100
        jae     .unit
        imul    eax, edx, 10
        xor     edx, edx
        div     ebx
        test    eax, eax
        jz      .unit

        mov     byte [edi], "."
        inc     edi
        call    fmt_uint

.unit:
        dec     ecx
        js      @f

.skip:
        lodsb
        test    al, al
        jnz     .skip
        jmp     .unit

@@:
        call    fmt_str
        mov     [esp + PUSHA_EDI], edi
        popa
        ret

; eax = seconds; printed as "h:mm:ss"
fmt_time:
        pusha
        xor     edx, edx
        mov     ecx, 3600
        div     ecx
        call    fmt_uint

        xchg    eax, edx
        mov     cl, 60
        div     cl
        movzx   edx, ah
        call    .two
        xchg    eax, edx
        call    .two

        mov     [esp + PUSHA_EDI], edi
        popa
        ret

.two:                                   ; eax = 0..99
        aam
        add     ax, "00"
        xchg    al, ah
        shl     eax, 8
        mov     al, ":"
        stosd
        dec     edi
        ret

; esi -> path: the program without parameters
run_plain:
        xor     edx, edx                ; then run_app

; esi -> path, edx -> parameters or 0
run_app:
        pusha
        mov     [app_info.Flags], edx
        mov     [app_info.FileName], esi
        mcall   SF_FILE, app_info
        popa
        ret
