; Функции работы с консолью для программ КолибриОС
; diamond, 2006-2008


format MS COFF

public EXPORTS

section '.flat' code readable align 16

include 'font.inc'
include 'conscrl.inc'

;void __stdcall START(dword state);
START:
; N.B. The current kernel implementation does not require
;      evident heap initialization, because if DLL is loaded, heap is already initialized
;      (if heap was not initialized, loader does this implicitly).
;      So this action does nothing useful, but nothing harmful.
        push    68
        pop     eax
        push    11
        pop     ebx
        int     0x40
        or      eax, -1
        ret     4

; Инициализация консоли
; void __stdcall con_init(dword wnd_width, dword wnd_height,
;       dword scr_width, dword scr_height, const char* title);

align 4
con_init:
        pop     eax
        pop     [con.wnd_width]
        pop     [con.wnd_height]
        pop     [con.scr_width]
        pop     [con.scr_height]
        pop     [con.title]
        push    eax

        push ebx

        mov     ecx, 4
        mov     eax, con.wnd_width
        mov     edx, con.def_wnd_width
.1:
        cmp     dword [eax], -1
        jnz     @f
        mov     ebx, [edx]
        mov     [eax], ebx
@@:
        add     eax, 4
        add     edx, 4
        loop    .1
; allocate memory for console data & bitmap data
        mov     eax, [con.scr_width]
        mul     [con.scr_height]
        lea     ecx, [eax+eax]
        mov     eax, [con.wnd_width]
        mul     [con.wnd_height]
        imul    eax, font_width*font_height
        mov     ebx, eax
        push    ebx ecx
        add     ecx, eax
        push    68
        pop     eax
        push    12
        pop     ebx
        int     0x40
        pop     ecx ebx
        mov     edx, con.nomem_err
        test    eax, eax
        jz      con.fatal
        mov     [con.data], eax
        push    edi
        mov     edi, eax
        shr     ecx, 1
        mov     ax, 0x0720
        rep     stosw
        mov     ecx, ebx
        mov     [con.image], edi
        xor     eax, eax
        rep     stosb
        pop     edi
        and     byte [con_flags+1], not 2
; create console thread
        push    51
        pop     eax
        xor     ebx, ebx
        inc     ebx
        mov     ecx, con.thread
        mov     edx, con.stack_top
        int     0x40
        mov     edx, con.thread_err
        test    eax, eax
        js      con.fatal
        mov     [con.console_tid], eax
        pop     ebx
        ret
con.fatal:
; output string to debug board and die
        mov     cl, [edx]
        test    cl, cl
        jz      @f
        push    63
        pop     eax
        xor     ebx, ebx
        inc     ebx
        int     0x40
        inc     edx
        jmp     con.fatal
@@:
        or      eax, -1
        int     0x40

; dword __stdcall con_get_flags(void);
con_get_flags:
        mov     eax, [con_flags]
        ret

; dword __stdcall con_set_flags(dword flags);
con_set_flags:
        mov     eax, [esp+4]
        and     ah, not 2
        xchg    eax, [con_flags]
        ret     4

; dword __stdcall con_get_font_height(void);
con_get_font_height:
        mov     eax, font_height
        ret

; int __stdcall con_get_cursor_height(void);
con_get_cursor_height:
        mov     eax, [con.cursor_height]
        ret

; int __stdcall con_set_cursor_height(int new_height);
con_set_cursor_height:
        mov     eax, [esp+4]
        cmp     eax, font_height
        jae     @f
        xchg    eax, [con.cursor_height]
        ret     4
@@:
        mov     eax, [con.cursor_height]
        ret     4

; void __stdcall con_write_asciiz(const char* string);
con_write_asciiz:
        push    ebx esi
        or      ebx, -1
        mov     esi, [esp+12]
        call    con.write
        pop     esi ebx
        ret     4

; void __stdcall con_write_string(const char* string, dword length);
con_write_length:
        push    ebx esi
        mov     esi, [esp+12]
        mov     ebx, [esp+16]
        call    con.write
        pop     esi ebx
        ret     8

; Каждый символ классифицируется как один из
con.printfc.normal = 0   ; нормальный символ
con.printfc.percent = 1  ; '%'
con.printfc.dot = 2      ; '.'
con.printfc.asterisk = 3 ; '*'
con.printfc.zero = 4     ; '0'
con.printfc.digit = 5    ; ненулевая цифра
con.printfc.plus = 6     ; '+'
con.printfc.minus = 7    ; '-'
con.printfc.sharp = 8    ; '#'
con.printfc.space = 9    ; ' '
con.printfc.long = 10    ; 'l' for 'long'
con.printfc.short = 11   ; 'h' for 'short'
con.printfc.dec = 12     ; 'd' = print decimal
con.printfc.oct = 13     ; 'o' = print octal
con.printfc.unsigned = 14 ; 'u' = print unsigned decimal
con.printfc.hex = 15     ; 'x' = print hexadecimal
con.printfc.pointer = 16 ; 'p' = print pointer
con.printfc.char = 17    ; 'c' = print char
con.printfc.string = 18  ; 's' = print string

macro set char,type
{store byte con.printfc.#type at con.charcodes + char - ' '}

con.charcodes:
times 'x'-' '+1         db      con.printfc.normal
        set     '%', percent
        set     '.', dot
        set     '*', asterisk
        set     '0', zero
        set     '1', digit
        set     '2', digit
        set     '3', digit
        set     '4', digit
        set     '5', digit
        set     '6', digit
        set     '7', digit
        set     '8', digit
        set     '9', digit
        set     ' ', space
        set     '#', sharp
        set     '+', plus
        set     '-', minus
        set     'X', hex
        set     'x', hex
        set     'c', char
        set     'd', dec
        set     'h', short
        set     'i', dec
        set     'l', long
        set     'o', oct
        set     'p', pointer
        set     's', string
        set     'u', unsigned
purge set
align 4
con.charjump:
        dd      con_printf.normal
        dd      con_printf.percent
        dd      con_printf.dot
        dd      con_printf.asterisk
        dd      con_printf.zero
        dd      con_printf.digit
        dd      con_printf.plus
        dd      con_printf.minus
        dd      con_printf.sharp
        dd      con_printf.space
        dd      con_printf.long
        dd      con_printf.short
        dd      con_printf.dec
        dd      con_printf.oct
        dd      con_printf.unsigned
        dd      con_printf.hex
        dd      con_printf.pointer
        dd      con_printf.char
        dd      con_printf.string

; int __cdecl con_printf(const char* format, ...)
con_printf:
        xor     eax, eax
        pushad
        call    con.get_data_ptr
        lea     ebp, [esp+20h+8]
        mov     esi, [ebp-4]
        sub     esp, 64         ; reserve space for buffer
.loop:
        xor     eax, eax
        lodsb
        test    al, al
        jz      .done
        cmp     al, '%'
        jz      .spec_begin
.normal:
        call    con.write_char_ex
        inc     dword [esp+64+28]
        jmp     .loop
.errspec:
.percent:
        add     esp, 12
        jmp     .normal
.spec_begin:
        xor     ebx, ebx
; bl = тип позиции:
; 0 = начало
; 1 = прочитан ведущий 0 в спецификации формата
; 2 = читаем поле ширины
; 3 = читаем поле точности
; 4 = прочитано поле размера аргумента
; 5 = читаем поле типа
; bh = флаги:
; 1 = флаг '#', выводить 0/0x/0X
; 2 = флаг '-', выравнивание влево
; 4 = флаг '0', дополнение нулями
; 8 = флаг 'h', короткий аргумент
        push    -1
; dword [esp+8] = precision
        push    -1
; dword [esp+4] = width
        push    0
; byte [esp] = флаг 0/'+'/' '
.spec:
        xor     eax, eax
        lodsb
        test    al, al
        jz      .done
        cmp     al, ' '
        jb      .normal
        cmp     al, 'x'
        ja      .normal
        movzx   ecx, byte [con.charcodes + eax - ' ']
        jmp     dword[con.charjump + ecx*4]

.sharp:
        test    bl, bl
        jnz     .errspec
        or      bh, 1
        jmp     .spec
.minus:
        test    bl, bl
        jnz     .errspec
        or      bh, 2
        jmp     .spec
.plus:
.space:
        test    bl, bl
        jnz     .errspec
        cmp     byte [esp], '+'
        jz      .spec
        mov     byte [esp], al
        jmp     .spec
.zero:
        test    bl, bl
        jnz     .digit
        test    bh, 2
        jnz     .spec
        or      bh, 4
        inc     ebx
        jmp     .spec
.digit:
        sub     al, '0'
        cmp     bl, 2
        ja      .precision
        mov     bl, 2
        xchg    eax, [esp+4]
        test    eax, eax
        js      .spec
        lea     eax, [eax*5]
        add     eax, eax
        add     [esp+4], eax
        jmp     .spec
.precision:
        cmp     bl, 3
        jnz     .errspec
        xchg    eax, [esp+8]
        lea     eax, [eax*5]
        add     eax, eax
        add     [esp+8], eax
        jmp     .spec
.asterisk:
        mov     eax, [ebp]
        add     ebp, 4
        cmp     bl, 2
        ja      .asterisk_precision
        test    eax, eax
        jns     @f
        neg     eax
        or      bh, 2
@@:
        mov     [esp+4], eax
        mov     bl, 3
        jmp     .spec
.asterisk_precision:
        cmp     bl, 3
        jnz     .errspec
        mov     [esp+8], eax
        inc     ebx
        jmp     .spec
.dot:
        cmp     bl, 2
        ja      .errspec
        mov     bl, 3
        and     dword [esp+8], 0
        jmp     .spec
.long:
        cmp     bl, 3
        ja      .errspec
        mov     bl, 4
        jmp     .spec
.short:
        cmp     bl, 3
        ja      .errspec
        mov     bl, 4
        or      bh, 8
        jmp     .spec
.unsigned:
.dec:
        push    10
        jmp     .write_number
.pointer:
        mov     dword [esp+12], 8
        or      bh, 4
        and     bh, not 8
.hex:
        push    16
        jmp     @f
.oct:
        push    8
@@:
        mov     byte [esp+4], 0
.write_number:
        pop     ecx
        push    edi
        lea     edi, [esp+16+64-1]      ; edi -> end of buffer
        mov     byte [edi], 0
        push    edx
        push    eax
        mov     eax, [ebp]
        add     ebp, 4
        test    bh, 8
        jz      @f
        movzx   eax, ax
        cmp     byte [esp], 'd'
        jnz     @f
        movsx   eax, ax
@@:
        xor     edx, edx
        test    eax, eax
        jns     @f
        cmp     byte [esp], 'd'
        jnz     @f
        inc     edx
        neg     eax
@@:
        push    edx
        xor     edx, edx
; число в eax, основание системы счисления в ecx
@@:
        cmp     dword [esp+16+8], 0
        jnz     .print_num
        test    eax, eax
        jz      .zeronum
.print_num:
        div     ecx
        xchg    eax, edx
        cmp     al, 10
        sbb     al, 69h
        das
        cmp     byte [esp+4], 'x'
        jnz     @f
        or      al, 20h
@@:
        dec     edi
        mov     [edi], al
        xor     eax, eax
        xchg    eax, edx
        test    eax, eax
        jnz     .print_num
.zeronum:
        push    0
        mov     edx, [esp+12]
        lea     eax, [esp+32+64-1]
        sub     eax, edi
        cmp     dword [esp+20+8], -1
        jz      .noprec1
        cmp     eax, [esp+20+8]
        jae     .len_found1
        mov     eax, [esp+20+8]
        jmp     .len_found1
.noprec1:
        test    bh, 4
        jnz     .do_print_num
.len_found1:
        test    bh, 2
        jnz     .do_print_num
        cmp     byte [esp+20], 0
        jz      @f
        inc     eax
@@:
        cmp     byte [esp+20], 0
        jnz     @f
        cmp     byte [esp+4], 0
        jz      @f
        inc     eax
@@:
        test    bh, 1
        jz      .nosharp1
        cmp     cl, 8
        jnz     @f
        inc     eax
        jmp     .nosharp1
@@:
        cmp     cl, 16
        jnz     .nosharp1
        inc     eax
        inc     eax
.nosharp1:
        cmp     dword [esp+20+4], -1
        jz      .do_print_num
        sub     eax, [esp+20+4]
        jae     .do_print_num
        push    ecx
        mov     ecx, eax
        mov     al, ' '
@@:
        xchg    edi, [esp+20]
        call    con.write_char_ex
        inc     dword [esp+24+12+64+28]
        xchg    edi, [esp+20]
        inc     dword [esp+4]
        inc     ecx
        jnz     @b
        pop     ecx
.do_print_num:
        mov     al, '-'
        cmp     byte [esp+4], 0
        jnz     .write_sign
        mov     al, [esp+20]
        test    al, al
        jz      .sign_written
.write_sign:
        call    .num_write_char
.sign_written:
        test    bh, 1
        jz      .nosharp2
        mov     al, '0'
        cmp     cl, 8
        jz      @f
        cmp     cl, 16
        jnz     .nosharp2
        call    .num_write_char
        mov     al, [esp+8]
@@:
        call    .num_write_char
.nosharp2:
        lea     ecx, [esp+32+64-1]
        sub     ecx, edi
        cmp     dword [esp+20+8], -1
        jz      .noprec2
        sub     ecx, [esp+20+8]
        jmp     .lead_zeroes
.noprec2:
        test    bh, 4
        jz      .do_print_num2
        add     ecx, [esp]
        sub     ecx, [esp+20+4]
.lead_zeroes:
        jae     .do_print_num2
@@:
        mov     al, '0'
        call    .num_write_char
        inc     ecx
        jnz     @b
.do_print_num2:
        mov     al, [edi]
        test    al, al
        jz      .num_written
        call    .num_write_char
        inc     edi
        jmp     .do_print_num2
.num_written:
        pop     ecx
        mov     edi, [esp+12]
        cmp     dword [esp+16+4], -1
        jz      .num_written2
@@:
        cmp     ecx, [esp+16+4]
        jae     .num_written2
        mov     al, ' '
        call    con.write_char
        inc     ecx
        jmp     @b
.num_written2:
        add     esp, 16
.spec_done:
        add     esp, 12
        jmp     .loop
.char:
        mov     ecx, [esp+4]
        cmp     ecx, -1
        jnz     @f
        inc     ecx
@@:
        test    ecx, ecx
        jnz     @f
        inc     ecx
@@:
        test    bh, 2
        jnz     .char_left_pad
        mov     al, ' '
        dec     ecx
        jz      .nowidth
        add     [esp+12+64+28], ecx
@@:
        call    con.write_char
        loop    @b
.nowidth:
        mov     al, [ebp]
        add     ebp, 4
        jmp     .percent
.char_left_pad:
        mov     al, [ebp]
        add     ebp, 4
        call    con.write_char_ex
        add     [esp+12+64+28], ecx
        dec     ecx
        jz      .nowidth2
        mov     al, ' '
@@:
        call    con.write_char
        loop    @b
.nowidth2:
        jmp     .spec_done
.string:
        push    esi
        mov     esi, [ebp]
        test    esi, esi
        jnz     @f
        mov     esi, con.aNull
@@:
        add     ebp, 4
        or      ecx, -1
@@:
        inc     ecx
        cmp     byte [esi+ecx], 0
        jnz     @b
        cmp     ecx, [esp+12]
        jb      @f
        mov     ecx, [esp+12]
@@:
        test    bh, 2
        jnz     .write_string
        cmp     dword [esp+8], -1
        jz      .write_string
        push    ecx
        sub     ecx, [esp+12]
        jae     .nospace
        mov     al, ' '
@@:
        call    con.write_char
        inc     dword [esp+20+64+28]
        inc     ecx
        jnz     @b
.nospace:
        pop     ecx
.write_string:
        jecxz   .string_written
        add     dword [esp+16+64+28], ecx
        push    ecx
@@:
        lodsb
        call    con.write_char_ex
        loop    @b
        pop     ecx
.string_written:
        pop     esi
        test    bh, 2
        jz      .spec_done
        cmp     dword [esp+4], -1
        jz      .spec_done
        sub     ecx, [esp+4]
        jae     .spec_done
        mov     al, ' '
@@:
        call    con.write_char
        inc     dword [esp+12+64+28]
        inc     ecx
        jnz     @b
        jmp     .spec_done
.done:
        add     esp, 64
        popad
        jmp     con.update_screen
.num_write_char:
        xchg    edi, [esp+20]
        call    con.write_char_ex
        inc     dword [esp+24+12+64+28]
        xchg    edi, [esp+20]
        inc     dword [esp+4]
        ret

con.write:
; esi = string, ebx = length (ebx=-1 for ASCIIZ strings)
        push    edi
        call    con.get_data_ptr
        test    ebx, ebx
        jz      .done
.loop:
        lodsb
        cmp     ebx, -1
        jnz     @f
        test    al, al
        jz      .done
@@:
        call    con.write_char_ex
.next:
        cmp     ebx, -1
        jz      .loop
        dec     ebx
        jnz     .loop
.done:
        pop     edi
        jmp     con.update_screen

con.get_data_ptr:
        mov     edi, [con.cur_y]
        imul    edi, [con.scr_width]
        add     edi, [con.cur_x]
        add     edi, edi
        add     edi, [con.data]
        ret

con.write_char_ex:
        test    byte [con_flags+1], 1
        jz      con.write_special_char

con.write_char:
        push    eax

        mov     eax, [con.cur_x]
        cmp     eax, [con.scr_width]
        jb      @f
        and     [con.cur_x], 0
        call    con.newline
@@:
        mov     eax, [esp]
        stosb
        mov     al, byte [con_flags]
        stosb

        mov     eax, [con.cur_x]
        inc     eax
        mov     [con.cur_x], eax

        pop     eax
        ret

con.write_special_char:
        cmp     [con_esc], 0
        jnz     .esc_mode
.normal_mode:
        cmp     al, 10
        jz      .write_lf
        cmp     al, 13
        jz      .write_cr
        cmp     al, 27
        jz      .write_esc
        cmp     al, 8
        jz      .write_bs
        cmp     al, 9
        jnz     con.write_char
.write_tab:
        mov     al, ' '
        call    con.write_char
        test    [con.cur_x], 7
        jnz     .write_tab
        ret
.write_cr:
        and     [con.cur_x], 0
        jmp     con.get_data_ptr
.write_lf:
        and     [con.cur_x], 0
        jmp     con.newline
.write_bs:
        cmp     [con.cur_x], 0
        jz      @f
        dec     [con.cur_x]
        dec     edi
        dec     edi
        ret
@@:
        push    eax
        mov     eax, [con.cur_y]
        dec     eax
        js      @f
        mov     [con.cur_y], eax
        mov     eax, [con.scr_width]
        dec     eax
        mov     [con.cur_x], eax
        dec     edi
        dec     edi
@@:
        pop     eax
        ret
.write_esc:
        mov     [con_esc], 1
        mov     [con_esc_attr_n], 1
        and     [con_esc_attrs], 0
        ret
.esc_mode:
        cmp     [con_sci], 0
        jnz     .esc_sci
        cmp     al, '['
        jnz     @f
        mov     [con_sci], 1
        ret
@@:
        push    eax
        mov     al, 27
        call    con.write_char
        pop     eax
        jmp     con.write_char
.esc_sci:
; this is real Esc sequence
        cmp     al, '?'         ; DEC private mode (DECSET/DECRST sequences)
        je      .questionmark
        cmp     al, ';'
        jz      .next_arg
        cmp     al, '0'
        jb      .not_digit
        cmp     al, '9'
        ja      .not_digit
        push    eax ecx edx
        sub     al, '0'
        movzx   eax, al
        mov     ecx, [con_esc_attr_n]
        mov     edx, [con_esc_attrs+(ecx-1)*4]
        lea     edx, [edx*5]
        lea     edx, [edx*2+eax]
        mov     [con_esc_attrs+(ecx-1)*4], edx
        pop     edx ecx eax
        ret
.questionmark:
        push    ecx
        mov     ecx, [con_esc_attr_n]
        mov     dword[con_esc_attrs+(ecx-1)*4], 0xffffffff
        pop     ecx
.next_arg:
        push    eax
        mov     eax, [con_esc_attr_n]
        inc     eax
        cmp     al, 4
        jbe     @f
        dec     eax
@@:
        mov     [con_esc_attr_n], eax
        and     [con_esc_attrs+(eax-1)*4], 0
        pop     eax
        ret
.not_digit:
        mov     [con_esc], 0
        mov     [con_sci], 0    ; in any case, leave Esc mode
        cmp     al, 'J'
        jz      .clear
        cmp     al, 'H'
        jz      .setcursor
        cmp     al, 'f'
        jz      .setcursor
        cmp     al, 'm'
        jz      .set_attr
        cmp     al, 'A'
        jz      .cursor_up
        cmp     al, 'B'
        jz      .cursor_down
        cmp     al, 'C'
        jz      .cursor_right
        cmp     al, 'D'
        jz      .cursor_left
        cmp     al, 'l'
        je      .dec_rst
        cmp     al, 'h'
        je      .dec_set
        ret     ; simply skip unknown sequences

.dec_rst:
        mov     eax, [con_esc_attrs]
        cmp     eax, 0xffffffff
        jne     .no_dec_rst
        mov     eax, [con_esc_attrs+4]
        cmp     eax, 25
        je      .hide_cursor
.no_dec_rst:
        ret
.hide_cursor:
        mov     [con.cursor_height], 0
        ret

.dec_set:
        mov     eax, [con_esc_attrs]
        cmp     eax, 0xffffffff
        jne     .no_dec_set
        mov     eax, [con_esc_attrs+4]
        cmp     eax, 25
        je      .show_cursor
.no_dec_set:
        ret

.show_cursor:
        mov     [con.cursor_height], (15*font_height+50)/100    ; default height
        ret
.clear:
        mov     eax, [con_esc_attrs]
        test    eax, eax
        jz      .clear_till_end_of_screen       ; <esc>[0J (or <esc>[J)
        dec     eax
        jz      .clear_till_start_of_screen     ; <esc>[1J
        dec     eax
        je      .cls                            ; <esc>[2J
        ret     ; unknown sequence

.clear_till_end_of_screen:
        push    edi ecx
        mov     ecx, [con.scr_width]
        imul    ecx, [con.scr_height]

        mov     edi, [con.cur_y]
        imul    edi, [con.scr_width]
        add     edi, [con.cur_x]

        sub     ecx, edi
        shl     edi, 1
        add     edi, [con.data]
        mov     ah, byte[con_flags]
        mov     al, ' '
        rep     stosw

        and     [con.cur_x], 0
        and     [con.cur_y], 0
        pop     ecx edi
        ret

.clear_till_start_of_screen:
        push    edi ecx
        mov     ecx, [con.cur_y]
        imul    ecx, [con.scr_width]
        add     ecx, [con.cur_x]
        mov     edi, [con.data]
        mov     ah, byte[con_flags]
        mov     al, ' '
        rep     stosw
        pop     ecx edi
        ret

.cls:   ; clear screen completely
        push    ecx
        and     [con.cur_x], 0
        and     [con.cur_y], 0
        mov     edi, [con.data]
        push    edi
        mov     ecx, [con.scr_width]
        imul    ecx, [con.scr_height]
        mov     ax, 0720h
        rep     stosw
        pop     edi ecx
.nosetcursor:
        ret
.setcursor:
        cmp     [con_esc_attr_n], 2
        je      @f
        xor     eax, eax
        mov     [con.cur_x], eax
        mov     [con.cur_y], eax
        jmp     .j_get_data
@@:
        mov     eax, [con_esc_attrs]
        cmp     eax, [con.scr_width]
        jae     @f
        mov     [con.cur_x], eax
@@:
        mov     eax, [con_esc_attrs+4]
        cmp     eax, [con.scr_height+4]
        jae     @f
        mov     [con.cur_y], eax
.j_get_data:
        jmp     con.get_data_ptr
.cursor_up:
        cmp     [con_esc_attr_n], 1
        jnz     .nosetcursor
        mov     eax, [con.cur_y]
        sub     eax, [con_esc_attrs]
        jnc     @f
        xor     eax, eax
@@:
        mov     [con.cur_y], eax
        jmp     .j_get_data
.cursor_down:
        cmp     [con_esc_attr_n], 1
        jnz     .nosetcursor
        mov     eax, [con.cur_y]
        add     eax, [con_esc_attrs]
        cmp     eax, [con.scr_height]
        jb      @f
        mov     eax, [con.scr_height]
        dec     eax
@@:
        mov     [con.cur_y], eax
        jmp     .j_get_data
.cursor_right:
        cmp     [con_esc_attr_n], 1
        jnz     .nosetcursor
        mov     eax, [con.cur_x]
        add     eax, [con_esc_attrs]
        cmp     eax, [con.scr_width]
        jb      @f
        mov     eax, [con.scr_width]
        dec     eax
@@:
        mov     [con.cur_x], eax
        jmp     .j_get_data
.cursor_left:
        cmp     [con_esc_attr_n], 1
        jnz     .nosetcursor
        mov     eax, [con.cur_x]
        sub     eax, [con_esc_attrs]
        jnc     @f
        xor     eax, eax
@@:
        mov     [con.cur_x], eax
        jmp     .j_get_data
.set_attr:
        push    eax ecx edx
        xor     ecx, ecx
.set_one_attr:
        mov     eax, [con_esc_attrs+ecx*4]
        cmp     al, 0
        jz      .attr_normal
        cmp     al, 1
        jz      .attr_bold
        cmp     al, 5
        jz      .attr_bgr_bold
        cmp     al, 7
        jz      .attr_reversed

        xor     edx, edx
        cmp     al, 30
        jz      .attr_color
        mov     dl, 4
        cmp     al, 31
        jz      .attr_color
        mov     dl, 2
        cmp     al, 32
        jz      .attr_color
        mov     dl, 6
        cmp     al, 33
        jz      .attr_color
        mov     dl, 1
        cmp     al, 34
        jz      .attr_color
        mov     dl, 5
        cmp     al, 35
        jz      .attr_color
        mov     dl, 3
        cmp     al, 36
        jz      .attr_color
        mov     dl, 7
        cmp     al, 37
        jz      .attr_color

        xor     edx, edx
        cmp     al, 40
        jz      .attr_bgr_color
        mov     dl, 0x40
        cmp     al, 41
        jz      .attr_bgr_color
        mov     dl, 0x20
        cmp     al, 42
        jz      .attr_bgr_color
        mov     dl, 0x60
        cmp     al, 43
        jz      .attr_bgr_color
        mov     dl, 0x10
        cmp     al, 44
        jz      .attr_bgr_color
        mov     dl, 0x50
        cmp     al, 45
        jz      .attr_bgr_color
        mov     dl, 0x30
        cmp     al, 46
        jz      .attr_bgr_color
        mov     dl, 0x70
        cmp     al, 47
        jz      .attr_bgr_color

        mov     dl, 0x08
        cmp     al, 90
        jz      .attr_color
        mov     dl, 4 + 8
        cmp     al, 91
        jz      .attr_color
        mov     dl, 2 + 8
        cmp     al, 92
        jz      .attr_color
        mov     dl, 6 + 8
        cmp     al, 93
        jz      .attr_color
        mov     dl, 1 + 8
        cmp     al, 94
        jz      .attr_color
        mov     dl, 5 + 8
        cmp     al, 95
        jz      .attr_color
        mov     dl, 3 + 8
        cmp     al, 96
        jz      .attr_color
        mov     dl, 7 + 8
        cmp     al, 97
        jz      .attr_color

        mov     dl, 0x80
        cmp     al, 100
        jz      .attr_bgr_color
        mov     dl, 0x80 + 0x40
        cmp     al, 101
        jz      .attr_bgr_color
        mov     dl, 0x80 + 0x20
        cmp     al, 102
        jz      .attr_bgr_color
        mov     dl, 0x80 + 0x60
        cmp     al, 103
        jz      .attr_bgr_color
        mov     dl, 0x80 + 0x10
        cmp     al, 104
        jz      .attr_bgr_color
        mov     dl, 0x80 + 0x50
        cmp     al, 105
        jz      .attr_bgr_color
        mov     dl, 0x80 + 0x30
        cmp     al, 106
        jz      .attr_bgr_color
        mov     dl, 0x80 + 0x70
        cmp     al, 107
        jnz     .attr_continue

.attr_bgr_color:
        mov     eax, [con_flags]
        and     al, 0x0F
        or      al, dl
        mov     [con_flags], eax
        jmp     .attr_continue
.attr_color:
        mov     eax, [con_flags]
        and     al, 0xF0
        or      al, dl
        mov     [con_flags], eax
        jmp     .attr_continue
.attr_normal:
        mov     byte [con_flags], 7
        jmp     .attr_continue
.attr_reversed:
        mov     byte [con_flags], 0x70
        jmp     .attr_continue
.attr_bold:
        or      byte [con_flags], 8
        jmp     .attr_continue
.attr_bgr_bold:
        or      byte [con_flags], 0x80
.attr_continue:
        inc     ecx
        cmp     ecx, [con_esc_attr_n]
        jb      .set_one_attr
        pop     edx ecx eax
        ret

con.newline:
        mov     eax, [con.cur_y]
        inc     eax
        mov     [con.cur_y], eax
        cmp     eax, [con.scr_height]
        jb      @f
        call    con.scr_scroll_up
@@:
        call    con.get_data_ptr
        ret

con.scr_scroll_up:
        pushad
        mov     edi, [con.data]
        mov     esi, edi
        add     esi, [con.scr_width]
        add     esi, [con.scr_width]
        dec     [con.cur_y]
        mov     ecx, [con.scr_height]
        dec     ecx
        imul    ecx, [con.scr_width]
        shr     ecx, 1
        rep     movsd
        adc     ecx, ecx
        rep     movsw
        mov     ax, 0x0720
        mov     ecx, [con.scr_width]
        rep     stosw
        popad
        ret

con.data2image:
        pushad
        mov     edi, [con.image]
        mov     esi, [con.data]
        mov     eax, [con.wnd_ypos]
        mul     [con.scr_width]
        add     eax, [con.wnd_xpos]
        lea     esi, [esi+eax*2]
        mov     ecx, [con.wnd_height]
.lh:
        push    ecx
        mov     ecx, [con.wnd_width]
.lw:
        push    ecx edi
        xor     eax, eax
        mov     al, [esi+1]
        push    eax
        and     al, 0xF
        mov     ebx, eax                ; цвет текста
        pop     eax
        shr     al, 4
        mov     ebp, eax                ; цвет фона
        sub     ebx, ebp
        lodsb
        inc     esi
if font_width > 8
        lea     edx, [eax+eax+font]
else
        lea     edx, [eax+font]
end if
.sh:
        mov     ecx, [edx]
repeat font_width
        shr     ecx, 1
        sbb     eax, eax
        and     eax, ebx
        add     eax, ebp
        mov     [edi+%-1], al
end repeat
        mov     eax, [con.wnd_width]
;        imul    eax, font_width
;        add     edi, eax
if font_width = 6
        lea     eax, [eax*2+eax]
        lea     edi, [edi+eax*2]
else if font_width = 7
        lea     edi, [edi+eax*8]
        sub     edi, eax
else if font_width = 8
        lea     edi, [edi+eax*8]
else if font_width = 9
        lea     edi, [edi+eax*8]
        add     edi, eax
else if font_width = 10
        lea     eax, [eax*4+eax]
        lea     edi, [edi+eax*2]
else
Unknown font_width value!
end if
if font_width > 8
        add     edx, 256*2
        cmp     edx, font+256*2*font_height
else
        add     edx, 256
        cmp     edx, font+256*font_height
end if
        jb      .sh
        pop     edi ecx
        add     edi, font_width
        sub     ecx, 1
        jnz     .lw
        mov     eax, [con.wnd_width]
        imul    eax, (font_height-1)*font_width
        add     edi, eax
        pop     ecx
        mov     eax, [con.scr_width]
        sub     eax, [con.wnd_width]
        lea     esi, [esi+eax*2]
        dec     ecx
        jnz     .lh
        mov     eax, [con.cur_y]
        sub     eax, [con.wnd_ypos]
        jb      .nocursor
        cmp     eax, [con.wnd_height]
        jae     .nocursor
        inc     eax
        mul     [con.wnd_width]
        imul    eax, font_height*font_width
        mov     edx, [con.cur_x]
        sub     edx, [con.wnd_xpos]
        jb      .nocursor
        cmp     edx, [con.wnd_width]
        jae     .nocursor
        inc     edx
        imul    edx, font_width
        add     eax, edx
        add     eax, [con.image]
        mov     edx, [con.wnd_width]
        imul    edx, font_width
        neg     edx
        mov     ecx, [con.cursor_height]
        jecxz   .nocursor
.cursor_loop:
        push    ecx
        mov     ecx, font_width
        add     eax, edx
        push    eax
@@:
        xor     byte [eax-1], 7
        dec     eax
        loop    @b
        pop     eax
        pop     ecx
        loop    .cursor_loop
.nocursor:
        popad
        ret

con_exit:
        cmp     byte [esp+4], 0
        jz      .noexit
        mov     [con.thread_op], 1
        call    con.wake
        ret     4
.noexit:
        push    esi
        mov     esi, [con.title]
        mov     edx, con.finished_title
        mov     ecx, 255
        call    .strcpy
        mov     esi, con.aFinished
        call    .strcpy
        mov     byte [edx], 0
        pop     esi
        and     [con.cursor_height], 0
        push    con.finished_title
        call    con_set_title
        ret     4
.strcpy:
        jecxz   .ret
@@:
        lodsb
        test    al, al
        jz      .ret
        mov     [edx], al
        inc     edx
        loop    @b
.ret:
        ret

con_set_title:
        mov     eax, [esp+4]
        mov     [con.title], eax
        mov     [con.thread_op], 2
        call    con.wake
        ret     4

; int __stdcall con_kbhit(void);
con_kbhit:
        test    byte [con_flags+1], 2
        jnz     @f
        mov     eax, [con.input_start]
        cmp     eax, [con.input_end]
@@:
        setnz   al
        movzx   eax, al
        ret

con.force_entered_char:
        cmp     [con.entered_char], -1
        jnz     .ret
        mov     [con.thread_op], 4
        call    con.wake
        test    byte [con_flags+1], 2
        jnz     .ret
; wait for response
        push    ebx
        push    5
        pop     eax
        push    2
        pop     ebx
@@:
        int     0x40
        cmp     [con.entered_char], -1
        jz      @b
        pop     ebx
.ret:
        ret

; int __stdcall con_getch(void);
con_getch:
        call    con.force_entered_char
        test    byte [con_flags+1], 2
        jnz     con_getch_closed
        movzx   eax, byte [con.entered_char]
        sar     [con.entered_char], 8
        mov     byte [con.entered_char+1], 0xFF
        test    al, al
        jz      @f
        mov     byte [con.entered_char], 0xFF
@@:
        ret

con_getch_closed:
        xor     eax, eax
        ret

; int __stdcall con_getch2(void);
con_getch2:
        call    con.force_entered_char
        test    byte [con_flags+1], 2
        jnz     con_getch_closed
        mov     eax, 0xFFFF
        xchg    ax, [con.entered_char]
        ret

; char* __stdcall con_gets(char* str, int n);
con_gets:
        pop     eax
        push    0
        push    eax
; char* __stdcall con_gets2(con_gets2_callback callback, char* str, int n);
con_gets2:
        mov     eax, [esp+8]            ; str
        pushad
        mov     esi, eax                ; str
        mov     ebx, [esp+20h+12]       ; n
        sub     ebx, 1
        jle     .ret
        mov     byte [esi], 0
        xor     ecx, ecx                ; длина уже введённой строки
        call    con.get_data_ptr
.loop:
        call    con_getch2
        test    al, al
        jz      .extended
        cmp     al, 8
        jz      .backspace
        cmp     al, 27
        jz      .esc
        cmp     al, 13
        jz      .enter
        cmp     al, 9
        jz      .tab
        inc     ecx
        mov     dl, al
        call    con.write_char_ex
        push    [con.cur_x]
        push    [con.cur_y]
        push    edi
        push    esi
@@:
        lodsb
        mov     [esi-1], dl
        mov     dl, al
        test    al, al
        jz      @f
        call    con.write_char_ex
        jmp     @b
@@:
        mov     [esi], dl
        pop     esi
        inc     esi
        pop     edi
        pop     [con.cur_y]
        pop     [con.cur_x]
.update_screen_and_loop:
        call    con.update_screen
        cmp     ecx, ebx
        jb      .loop
.ret_us:
        mov     edx, [con.cur_x]
@@:
        lodsb
        test    al, al
        jz      @f
        inc     edx
        cmp     edx, [con.scr_width]
        jb      @b
        xor     edx, edx
        call    con.newline
        jmp     @b
@@:
        mov     [con.cur_x], edx
        call    con.get_data_ptr
        call    con.update_screen
        jmp     .ret
.esc:
        mov     edx, [con.cur_x]
@@:
        lodsb
        test    al, al
        jz      @f
        inc     edx
        cmp     edx, [con.scr_width]
        jb      @b
        xor     edx, edx
        call    con.newline
        jmp     @b
@@:
        mov     [con.cur_x], edx
        call    con.get_data_ptr
        dec     esi
        xor     ecx, ecx
@@:
        mov     byte [esi], 0
        cmp     esi, [esp+20h+8]
        jbe     .update_screen_and_loop
        mov     al, 8
        call    con.write_special_char
        mov     al, ' '
        call    con.write_char
        mov     al, 8
        call    con.write_special_char
        dec     esi
        jmp     @b
.delete:
        cmp     byte [esi], 0
        jz      .loop
        lodsb
        call    con.write_char_ex
.backspace:
        cmp     esi, [esp+20h+8]
        jbe     .loop
        push    esi
        mov     edx, [con.cur_x]
@@:
        lodsb
        test    al, al
        jz      @f
        inc     edx
        cmp     edx, [con.scr_width]
        jb      @b
        xor     edx, edx
        call    con.newline
        jmp     @b
@@:
        mov     [con.cur_x], edx
        call    con.get_data_ptr
        dec     esi
        mov     al, 8
        call    con.write_special_char
        mov     al, ' '
        call    con.write_char
        mov     al, 8
        call    con.write_special_char
        mov     dl, 0
@@:
        cmp     esi, [esp]
        jbe     @f
        mov     al, 8
        call    con.write_special_char
        dec     esi
        xchg    dl, [esi]
        mov     al, dl
        call    con.write_char
        mov     al, 8
        call    con.write_special_char
        jmp     @b
@@:
        pop     esi
        dec     esi
        mov     [esi], dl
        dec     ecx
        jmp     .update_screen_and_loop
.enter:
        mov     edx, [con.cur_x]
@@:
        lodsb
        test    al, al
        jz      @f
        inc     edx
        cmp     edx, [con.scr_width]
        jb      @b
        xor     edx, edx
        call    con.newline
        jmp     @b
@@:
        mov     [con.cur_x], edx
        call    con.get_data_ptr
        mov     al, 10
        mov     [esi-1], al
        mov     byte [esi], 0
        call    con.write_special_char
        call    con.update_screen
        jmp     .ret
.tab:
        mov     al, 0
        mov     ah, 0xF
.extended:
        test    ah, ah
        jz      .closed
        xchg    al, ah
        cmp     al, 0x4B
        jz      .left
        cmp     al, 0x4D
        jz      .right
        cmp     al, 0x47
        jz      .home
        cmp     al, 0x4F
        jz      .end
        cmp     al, 0x53
        jz      .delete
; give control to callback function
        cmp     dword [esp+20h+4], 0
        jz      .loop
; remember length of text before and length of text after
; and advance cursor to the end of line
        push    ecx
        push    eax
        lea     edx, [esi+1]
@@:
        lodsb
        test    al, al
        jz      @f
        call    con.write_char_ex
        jmp     @b
@@:
        sub     esi, edx
        pop     eax
        push    esi
        dec     edx
        sub     edx, [esp+28h+8]
        push    edx
        push    esp             ; ppos
        mov     ecx, [esp+30h+4]
        lea     edx, [esp+30h+12]
        push    edx             ; pn
        lea     edx, [esp+34h+8]
        push    edx             ; pstr
        push    eax             ; keycode
        call    ecx
        call    con.get_data_ptr
        dec     eax
        js      .callback_nochange
        jz      .callback_del
        dec     eax
        jz      .callback_output
; callback returned 2 - exit
        add     esp, 12
        jmp     .ret
.callback_nochange:
; callback returned 0 - string was not changed, only restore cursor position
        pop     esi
        pop     ecx
        test    ecx, ecx
        jz      .cncs
@@:
        mov     al, 8
        call    con.write_special_char
        loop    @b
.cncs:
        pop     ecx
        add     esi, [esp+20h+8]
        jmp     .callback_done
.callback_del:
; callback returned 1 - string was changed, delete old string and output new
        mov     ecx, [esp+8]
        test    ecx, ecx
        jz      .cds
@@:
        mov     al, 8
        call    con.write_special_char
        mov     al, ' '
        call    con.write_char_ex
        mov     al, 8
        call    con.write_special_char
        loop    @b
.cds:
.callback_output:
; callback returned 2 - string was changed, output new string
        pop     edx
        pop     esi
        pop     ecx
        mov     esi, [esp+20h+8]
        xor     ecx, ecx
@@:
        lodsb
        test    al, al
        jz      @f
        call    con.write_char_ex
        inc     ecx
        jmp     @b
@@:
        dec     esi
        push    ecx
        sub     ecx, edx
        jz      .cos
@@:
        mov     al, 8
        call    con.write_special_char
        dec     esi
        loop    @b
.cos:
        pop     ecx
.callback_done:
        call    con.update_screen
        mov     ebx, [esp+20h+12]
        dec     ebx
        cmp     ecx, ebx
        jae     .ret_us
        jmp     .loop
.left:
        cmp     esi, [esp+20h+8]
        jbe     .loop
        dec     esi
        mov     al, 8
        call    con.write_special_char
        jmp     .update_screen_and_loop
.right:
        cmp     byte [esi], 0
        jz      .loop
        lodsb
        call    con.write_char_ex
        jmp     .update_screen_and_loop
.home:
        cmp     esi, [esp+20h+8]
        jz      .update_screen_and_loop
        dec     esi
        mov     al, 8
        call    con.write_special_char
        jmp     .home
.end:
        lodsb
        test    al, al
        jz      @f
        call    con.write_char_ex
        jmp     .end
@@:
        dec     esi
        jmp     .update_screen_and_loop
.closed:
        and     dword [esp+1Ch], 0
.ret:
        popad
        ret     12

; void __stdcall con_cls();
con_cls:
        push    edi
        call    con.write_special_char.cls
        pop     edi
        call    con.update_screen
        ret

; void __stdcall con_get_cursor_pos(int* px, int* py);
con_get_cursor_pos:
        push    eax ecx
        mov     eax, [esp+12]
        mov     ecx, [con.cur_x]
        mov     [eax], ecx
        mov     eax, [esp+16]
        mov     ecx, [con.cur_y]
        mov     [eax], ecx
        pop     ecx eax
        ret     8

; void __stdcall con_set_cursor_pos(int px, int py);
con_set_cursor_pos:
        push    eax
        mov     eax, [esp+8]
        cmp     eax, [con.scr_width]
        jae     @f
        mov     [con.cur_x], eax
@@:
        mov     eax, [esp+12]
        cmp     eax, [con.scr_height]
        jae     @f
        mov     [con.cur_y], eax
@@:
        pop     eax
        call    con.update_screen
        ret     8

con.update_screen:
        push    eax
        mov     eax, [con.cur_y]
        sub     eax, [con.wnd_ypos]
        jb      .up
        cmp     eax, [con.wnd_height]
        jb      .done
        mov     eax, [con.cur_y]
        sub     eax, [con.wnd_height]
        inc     eax
        jmp     .set
.up:
        mov     eax, [con.cur_y]
.set:
        mov     [con.wnd_ypos], eax
.done:
        pop     eax
        mov     [con.thread_op], 3

con.wake:
        pushad
        mov     al, [con.thread_op]
        cmp     al, byte [con.ipc_buf+0x10]
        jz      .ret
@@:
        push    60
        pop     eax
        push    2
        pop     ebx
        mov     ecx, [con.console_tid]
        jecxz   .ret
        mov     edx, con.thread_op
        push    1
        pop     esi
        int     0x40
        test    eax, eax
        jz      @f
        push    5
        pop     eax
        mov     bl, 1
        int     0x40
        jmp     @b
@@:
.ret:
        popad
        ret

; Поток окна консоли. Обрабатывает ввод и вывод.
con.thread:
; Поток реагирует на IPC, которое используется только для того, чтобы его можно было "разбудить"
        push    40
        pop     eax
        push    0x67
        pop     ebx
        int     0x40
        mov     al, 60
        mov     bl, 1
        mov     ecx, con.ipc_buf
        push    0x11
        pop     edx
        int     0x40
        mov     al, 66
        mov     bl, 1
        mov     ecx, ebx
        int     0x40
con.redraw:
        call    con.draw_window
con.msg_loop:
        cmp     dword [con.bUpPressed], 0
        jnz     .wait_timeout
        push    10
        pop     eax
        jmp     @f
.wait_timeout:
        push    23
        pop     eax
        push    5
        pop     ebx
@@:
        int     0x40
        dec     eax
        jz      con.redraw
        dec     eax
        jz      con.key
        dec     eax
        jz      con.button
        cmp     al, 4
        jz      con.ipc
        jmp     con.mouse
con.button:
; we have only one button, close
con.thread_exit:
        or      byte [con_flags+1], 2
        and     [con.console_tid], 0
        and     [con.entered_char], 0
        or      eax, -1
        int     0x40
con.key:
        mov     al, 2
        int     0x40
; ah = scancode
        cmp     ah, 0xE0
        jnz     @f
        mov     [con.bWasE0], 1
        jmp     con.msg_loop
@@:
        shr     eax, 8
        xchg    ah, [con.bWasE0]
        test    al, al
        jle     con.msg_loop
        cmp     al, 0x1D
        jz      con.msg_loop
        cmp     al, 0x2A
        jz      con.msg_loop
        cmp     al, 0x36
        jz      con.msg_loop
        cmp     al, 0x38
        jz      con.msg_loop
        cmp     al, 0x3A
        jz      con.msg_loop
        cmp     al, 0x45
        jz      con.msg_loop
        cmp     al, 0x46
        jz      con.msg_loop
        mov     edx, eax
        push    66
        pop     eax
        push    3
        pop     ebx
        int     0x40    ; eax = control key state
        test    dh, dh
        jnz     .extended
        bt      [scan_has_ascii], edx
        jnc     .extended
        test    al, 0x30
        jnz     .extended
; key has ASCII code
        push    eax edx
        push    2
        pop     ecx
        test    al, 3
        jnz     @f
        dec     ecx
@@:
        push    26
        pop     eax
        mov     bl, 2
        mov     edx, con.kbd_layout
        int     0x40
        pop     edx eax
        mov     dh, [con.kbd_layout+edx]
        test    al, 0xC
        jz      @f
        sub     dh, 0x60
        jmp     @f
.extended:
        mov     dh, 0   ; no ASCII code
@@:
; dh contains ASCII-code; now convert scancode to extended key code
        mov     ecx, con.extended_alt
        test    al, 0x30
        jnz     .xlat
        mov     ecx, con.extended_shift
        test    al, 3
        jnz     .xlat
        mov     ecx, con.extended_ctrl
        test    al, 0xC
        jnz     .xlat
        xchg    dl, dh
        cmp     dh, 0x57
        jz      @f
        cmp     dh, 0x58
        jnz     .gotcode
@@:
        add     dh, 0x85-0x57
        jmp     .gotcode
.xlat:
        movzx   eax, dl
        mov     dl, dh
        mov     dh, [eax+ecx]
.gotcode:
        test    dh, dh
        jz      con.msg_loop
        cmp     dh, 0x94
        jnz     @f
        mov     dl, 0
@@:
; dx contains full keycode
        cmp     [con.bGetchRequested], 0
        jz      @f
        mov     [con.entered_char], dx
        jmp     con.msg_loop
@@:
        mov     eax, [con.input_end]
        mov     ecx, eax
        add     eax, 2
        cmp     eax, con.input_buffer_end
        jnz     @f
        mov     eax, con.input_buffer
@@:
        cmp     eax, [con.input_start]
        jnz     @f
; buffer overflow, make beep and continue
        push    55
        pop     eax
        mov     ebx, eax
        mov     esi, con.beep
        int     0x40
        jmp     con.msg_loop
@@:
        mov     [ecx], dx
        mov     [con.input_end], eax
        jmp     con.msg_loop
con.ipc:
        movzx   eax, byte [con.ipc_buf+0x10]
        mov     byte [con.ipc_buf+4], 8
        mov     byte [con.ipc_buf+0x10], 0
        dec     eax
        jz      con.thread_exit
        dec     eax
        jz      con.set_title
        dec     eax
        jz      con.redraw_image
        dec     eax
        jz      con.getch
        jmp     con.msg_loop
con.set_title:
        push    71
        pop     eax
        push    1
        pop     ebx
        mov     ecx, [con.title]
        int     0x40
        jmp     con.msg_loop
con.redraw_image:
        call    con.data2image
        call    con.draw_image
        jmp     con.msg_loop
con.getch:
        mov     eax, [con.input_start]
        cmp     eax, [con.input_end]
        jz      .noinput
        mov     ecx, [eax]
        mov     [con.entered_char], cx
        inc     eax
        inc     eax
        cmp     eax, con.input_buffer_end
        jnz     @f
        mov     eax, con.input_buffer
@@:
        mov     [con.input_start], eax
        jmp     con.msg_loop
.noinput:
        mov     [con.bGetchRequested], 1
        jmp     con.msg_loop
con.mouse:
        xor     eax, eax
        xchg    eax, dword [con.bUpPressed]
        mov     dword [con.bUpPressed_saved], eax
        push    37
        pop     eax
        push    2
        pop     ebx
        int     0x40
        test    al, 1
        jnz     @f
        cmp     [con.vscroll_pt], -1
        jz      .redraw_if_needed
        or      [con.vscroll_pt], -1
.redraw_if_needed:
        cmp     dword [con.bUpPressed_saved], 0
        jnz     con.redraw_image
        jmp     con.msg_loop
@@:
        mov     al, 37
        dec     ebx
        int     0x40
        movsx   ebx, ax
        sar     eax, 16
        cmp     [con.vscroll_pt], -1
        jnz     .vscrolling
        test    ebx, ebx
        js      .redraw_if_needed
        sub     ax, [con.data_width]
        jb      .redraw_if_needed
        cmp     eax, con.vscroll_width
        jae     .redraw_if_needed
        cmp     ebx, con.vscroll_btn_height
        jb      .up
        sub     bx, [con.data_height]
        jae     .redraw_if_needed
        cmp     bx, -con.vscroll_btn_height
        jge     .down
        add     bx, [con.data_height]
        sub     bx, word [con.vscrollbar_pos]
        jl      .vscroll_up
        cmp     bx, word [con.vscrollbar_size]
        jl      .vscroll
.vscroll_down:
        cmp     [con.bScrollingDown_saved], 0
        jz      .vscroll_down_first
        cmp     [con.bScrollingDown_saved], 1
        jz      .vscroll_down_wasfirst
        mov     [con.bScrollingDown], 2
.vscroll_down_do:
        mov     eax, [con.wnd_ypos]
        add     eax, [con.wnd_height]
        dec     eax
        mov     ebx, [con.scr_height]
        sub     ebx, [con.wnd_height]
        cmp     eax, ebx
        jb      @f
        mov     eax, ebx
@@:
        mov     [con.wnd_ypos], eax
        jmp     con.redraw_image
.vscroll_down_first:
        push    26
        pop     eax
        push    9
        pop     ebx
        int     0x40
        mov     [con.scroll_down_first_time], eax
        mov     [con.bScrollingDown], 1
        jmp     .vscroll_down_do
.vscroll_down_wasfirst:
        push    26
        pop     eax
        push    9
        pop     ebx
        int     0x40
        sub     eax, [con.scroll_down_first_time]
        cmp     eax, 25
        jb      @f
        mov     [con.bScrollingDown], 2
        jmp     .vscroll_down_do
@@:
        mov     [con.bScrollingDown], 1
        jmp     con.msg_loop
.vscroll_up:
        cmp     [con.bScrollingUp_saved], 0
        jz      .vscroll_up_first
        cmp     [con.bScrollingUp_saved], 1
        jz      .vscroll_up_wasfirst
        mov     [con.bScrollingUp], 2
.vscroll_up_do:
        mov     eax, [con.wnd_ypos]
        inc     eax
        sub     eax, [con.wnd_height]
        jns     @f
        xor     eax, eax
@@:
        mov     [con.wnd_ypos], eax
        jmp     con.redraw_image
.vscroll_up_first:
        push    26
        pop     eax
        push    9
        pop     ebx
        int     0x40
        mov     [con.scroll_up_first_time], eax
        mov     [con.bScrollingUp], 1
        jmp     .vscroll_up_do
.vscroll_up_wasfirst:
        push    26
        pop     eax
        push    9
        pop     ebx
        int     0x40
        sub     eax, [con.scroll_up_first_time]
        cmp     eax, 25
        jb      @f
        mov     [con.bScrollingUp], 2
        jmp     .vscroll_up_do
@@:
        mov     [con.bScrollingUp], 1
        jmp     con.msg_loop
.up:
        cmp     [con.bUpPressed_saved], 0
        jz      .up_first
        cmp     [con.bUpPressed_saved], 1
        jz      .up_wasfirst
        mov     [con.bUpPressed], 2
.up_do:
        mov     eax, [con.wnd_ypos]
        dec     eax
        js      @f
        mov     [con.wnd_ypos], eax
@@:
        jmp     con.redraw_image
.up_first:
        push    26
        pop     eax
        push    9
        pop     ebx
        int     0x40
        mov     [con.up_first_time], eax
        mov     [con.bUpPressed], 1
        jmp     .up_do
.up_wasfirst:
        push    26
        pop     eax
        push    9
        pop     ebx
        int     0x40
        sub     eax, [con.up_first_time]
        cmp     eax, 25
        jb      @f
        mov     [con.bUpPressed], 2
        jmp     .up_do
@@:
        mov     [con.bUpPressed], 1
        jmp     con.msg_loop
.down:
        cmp     [con.bDownPressed_saved], 0
        jz      .down_first
        cmp     [con.bDownPressed_saved], 1
        jz      .down_wasfirst
        mov     [con.bDownPressed], 2
.down_do:
        mov     eax, [con.scr_height]
        sub     eax, [con.wnd_height]
        jbe     con.redraw_image
        cmp     [con.wnd_ypos], eax
        jae     con.redraw_image
        inc     [con.wnd_ypos]
        jmp     con.redraw_image
.down_first:
        push    26
        pop     eax
        push    9
        pop     ebx
        int     0x40
        mov     [con.down_first_time], eax
        mov     [con.bDownPressed], 1
        jmp     .down_do
.down_wasfirst:
        push    26
        pop     eax
        push    9
        pop     ebx
        int     0x40
        sub     eax, [con.down_first_time]
        cmp     eax, 25
        jb      @f
        mov     [con.bDownPressed], 2
        jmp     .down_do
@@:
        mov     [con.bDownPressed], 1
        jmp     con.msg_loop
.vscroll:
        mov     [con.vscroll_pt], ebx
        call    con.draw_image
        jmp     con.msg_loop
.vscrolling:
        sub     ebx, [con.vscroll_pt]
        sub     ebx, con.vscroll_btn_height
        jge     @f
        xor     ebx, ebx
@@:
        movzx   eax, [con.data_height]
        sub     eax, 2*con.vscroll_btn_height
        sub     eax, [con.vscrollbar_size]
        cmp     ebx, eax
        jb      @f
        lea     ebx, [eax-1]
@@:
        xchg    eax, ebx
        mov     edx, [con.scr_height]
        sub     edx, [con.wnd_height]
        inc     edx
        mul     edx
        div     ebx
        cmp     [con.wnd_ypos], eax
        jz      con.msg_loop
        mov     [con.wnd_ypos], eax
        jmp     con.redraw_image

con.draw_window:
        push    12
        pop     eax
        xor     ebx, ebx
        inc     ebx
        int     0x40
        mov     al, 48
        mov     bl, 4
        int     0x40
        mov     ebx, [con.def_wnd_x-2]
        mov     bx, word [con.wnd_width]
        imul    bx, font_width
        add     bx, 5+5-1
        mov     ecx, [con.def_wnd_y-2]
        mov     cx, word [con.wnd_height]
        imul    cx, font_height
        lea     ecx, [ecx+eax+5-1]
        mov     edx, 0x74000000
        mov     edi, [con.title]
; place for scrollbar
        mov     eax, [con.wnd_height]
        cmp     eax, [con.scr_height]
        jae     @f
        add     ebx, con.vscroll_width
@@:
        xor     eax, eax
        int     0x40
        ;Leency{
        mov     eax,9
        mov     ebx,process_info_buffer
        mov     ecx,-1
        int     0x40
        mov     eax,[ebx+70]
        mov     [window_status],eax
                test    [window_status],100b   ; window is rolled up
        jnz     .exit
        test    [window_status],10b    ; window is minimized to panel
        jnz     .exit
        ;}Leency - I'm in diamond code... 
        call    con.draw_image

.exit:
        push    12
        pop     eax
        push    2
        pop     ebx
        int     0x40
                
        ret

con.draw_image:
        xor     edx, edx
        mov     ecx, [con.wnd_width]
        imul    ecx, font_width
        mov     [con.data_width], cx
        shl     ecx, 16
        mov     cx, word [con.wnd_height]
        imul    cx, font_height
        mov     [con.data_height], cx
        mov     ebx, [con.image]
        push    65
        pop     eax
        xor     ebp, ebp
        mov     edi, con.colors
        push    8
        pop     esi
        int     0x40
        mov     al, 7
        mov     edx, [con.wnd_height]
        cmp     edx, [con.scr_height]
        jae     .skip_vscroll
        push    ecx
        mov     edx, ecx
        xor     dx, dx
        mov     ebx, con.vscroll_btn3
        cmp     [con.bUpPressed], 0
        jnz     @f
        mov     ebx, con.vscroll_btn1
@@:
        mov     ecx, con.vscroll_width*65536 + con.vscroll_btn_height
        int     0x40
        pop     edx
        sub     dx, con.vscroll_btn_height
        mov     ebx, con.vscroll_btn4
        cmp     [con.bDownPressed], 0
        jnz     @f
        mov     ebx, con.vscroll_btn2
@@:
        int     0x40
        push    edx
; Вычисляем высоту бегунка
        mov     ax, dx
        sub     eax, con.vscroll_btn_height
        mov     ecx, eax
        mul     [con.wnd_height]
        div     [con.scr_height]
        cmp     eax, 5
        jae     @f
        mov     al, 5
@@:
; eax = высота бегунка. Вычисляем положение бегунка
        mov     [con.vscrollbar_size], eax
        xchg    eax, ecx
        sub     eax, ecx
        mul     [con.wnd_ypos]
        mov     ebx, [con.scr_height]
        sub     ebx, [con.wnd_height]
        div     ebx
        pop     edx
        push    edx
; ecx = высота бегунка, eax = положение
        add     eax, con.vscroll_btn_height
        mov     [con.vscrollbar_pos], eax
        mov     ebx, con.vscroll_bgr2
        cmp     [con.bScrollingUp], 0
        jnz     @f
        mov     ebx, con.vscroll_bgr1
@@:
        mov     ecx, con.vscroll_width*65536 + con.vscroll_bgr_height
        push    eax
        push    7
        pop     eax
        mov     dx, con.vscroll_btn_height
        call    .vpattern
        mov     dx, word [con.vscrollbar_pos]
        add     dx, word [con.vscrollbar_size]
        mov     cx, con.vscroll_bgr_height
        mov     ebx, con.vscroll_bgr2
        cmp     [con.bScrollingDown], 0
        jnz     @f
        mov     ebx, con.vscroll_bgr1
@@:
        call    .vpattern
        mov     ecx, [con.vscrollbar_pos]
        mov     dx, cx
        add     ecx, [con.vscrollbar_size]
        sub     ecx, con.vscroll_bar_height3
        push    ecx
        mov     ebx, con.vscroll_bar1
        mov     ecx, con.vscroll_width*65536 + con.vscroll_bar_height1
        int     0x40
        add     dx, cx
        mov     cx, con.vscroll_bar_height2
        mov     ebx, con.vscroll_bar2
        call    .vpattern
        mov     ebx, con.vscroll_bar3
        mov     cx, con.vscroll_bar_height3
        int     0x40
.skip_vscroll:
        ret

.vpattern:
        push    edx
        add     dx, cx
        cmp     dx, [esp+8]
        pop     edx
        jbe     @f
        mov     cx, [esp+4]
        sub     cx, dx
        jz      .ret
@@:
        int     0x40
        add     dx, cx
        cmp     dx, [esp+4]
        jb      .vpattern
.ret:
        ret     4

align 4
con.colors      dd      0x000000, 0x000080, 0x008000, 0x008080
                dd      0x800000, 0x800080, 0x808000, 0xC0C0C0
                dd      0x808080, 0x0000FF, 0x00FF00, 0x00FFFF
                dd      0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF

scan_has_ascii:
        dd      11011111111111111111111111111110b
        dd      00000010001111111111101111111111b
        dd      00000000000000000000000000000000b
        dd      0

con.extended_alt:
        db      00h,01h,78h,79h,7Ah,7Bh,7Ch,7Dh,7Eh,7Fh,80h,81h,82h,83h,0Eh,0A5h
        db      10h,11h,12h,13h,14h,15h,16h,17h,18h,19h,1Ah,1Bh,1Ch,00h,1Eh,1Fh
        db      20h,21h,22h,23h,24h,25h,26h,27h,28h,29h,00h,2Bh,2Ch,2Dh,2Eh,2Fh
        db      30h,31h,32h,33h,34h,35h,00h,37h,00h,39h,00h,68h,69h,6Ah,6Bh,6Ch
        db      6Dh,6Eh,6Fh,70h,71h,00h,00h,97h,98h,99h,4Ah,9Bh,9Ch,9Dh,4Eh,9Fh
        db      0A0h,0A1h,0A2h,0A3h,00h,00h,00h,8Bh,8Ch,00h,00h,00h,00h,00h,00h,00h
        times 20h db 0
con.extended_ctrl:
        times 0Fh db %-1
        db      0x94
        times 2Bh db %-1
        db      5Eh,5Fh,60h,61h,62h,63h,64h,65h,66h,67h,00h,00h
        db      77h,8Dh,84h,8Eh,73h,8Fh,74h,90h,75h,91h,76h,92h,93h,00h,00h,00h,89h,8Ah
        times 0x80-0x59 db 0
con.extended_shift:
        times 3Bh db %-1
        db      54h,55h,56h,57h,58h,59h,5Ah,5Bh,5Ch,5Dh,00h,00h
        db      47h,48h,49h,4Ah,4Bh,4Ch,4Dh,4Eh,4Fh,50h,51h,52h,53h,00h,00h,00h,87h,88h
        times 0x80-0x59 db 0

; В текущей реализации значения по умолчанию таковы.
; В будущем они, возможно, будут считываться как параметры из ini-файла console.ini.
con.def_wnd_width   dd    80
con.def_wnd_height  dd    25
con.def_scr_width   dd    80
con.def_scr_height  dd    300
con.def_wnd_x       dd    200
con.def_wnd_y       dd    50


struc process_info
{
  cpu_usage               dd ?  ; +0
  window_stack_position   dw ?  ; +4
  window_stack_value      dw ?  ; +6
                          dw ?  ; +8
  process_name            rb 12 ; +10
  memory_start            dd ?  ; +22
  used_memory             dd ?  ; +26
  PID                     dd ?  ; +30
  box.x                   dd ?  ; +34
  box.y                   dd ?  ; +38
  box.width               dd ?  ; +42
  box.height              dd ?  ; +46
  slot_state              dw ?  ; +50
                          dw ?  ; +52
  client_box.x            dd ?  ; +54
  client_box.y            dd ?  ; +58
  client_box.width        dd ?  ; +62
  client_box.height       dd ?  ; +66
  wnd_state               db ?  ; +70
  rb (1024-71)
}
process_info_buffer process_info
window_status           rd 1

con.vscroll_pt      dd    -1

align 16
EXPORTS:
        dd      szStart,                START
        dd      szVersion,              0x00020007
        dd      szcon_init,             con_init
        dd      szcon_write_asciiz,     con_write_asciiz
        dd      szcon_write_string,     con_write_length
        dd      szcon_printf,           con_printf
        dd      szcon_exit,             con_exit
        dd      szcon_get_flags,        con_get_flags
        dd      szcon_set_flags,        con_set_flags
        dd      szcon_kbhit,            con_kbhit
        dd      szcon_getch,            con_getch
        dd      szcon_getch2,           con_getch2
        dd      szcon_gets,             con_gets
        dd      szcon_gets2,            con_gets2
        dd      szcon_get_font_height,  con_get_font_height
        dd      szcon_get_cursor_height,con_get_cursor_height
        dd      szcon_set_cursor_height,con_set_cursor_height
        dd      szcon_cls,              con_cls
        dd      szcon_get_cursor_pos,   con_get_cursor_pos
        dd      szcon_set_cursor_pos,   con_set_cursor_pos
        dd      0

con_flags       dd      7
con.cursor_height dd    (15*font_height+50)/100
con.input_start dd      con.input_buffer
con.input_end   dd      con.input_buffer

con_esc_attr_n  dd      0
con_esc_attrs   dd      0,0,0,0
con_esc         db      0
con_sci         db      0

con.entered_char dw     -1
con.bGetchRequested db  0
con.bWasE0       db     0

szStart                 db 'START',0

szcon_init              db 'con_init',0
szcon_write_asciiz      db 'con_write_asciiz',0
szcon_write_string      db 'con_write_string',0
szcon_printf            db 'con_printf',0
szcon_exit              db 'con_exit',0
szVersion               db 'version',0
szcon_get_flags         db 'con_get_flags',0
szcon_set_flags         db 'con_set_flags',0
szcon_kbhit             db 'con_kbhit',0
szcon_getch             db 'con_getch',0
szcon_getch2            db 'con_getch2',0
szcon_gets              db 'con_gets',0
szcon_gets2             db 'con_gets2',0
szcon_get_font_height   db 'con_get_font_height',0
szcon_get_cursor_height db 'con_get_cursor_height',0
szcon_set_cursor_height db 'con_set_cursor_height',0
szcon_cls               db 'con_cls',0
szcon_get_cursor_pos    db 'con_get_cursor_pos',0
szcon_set_cursor_pos    db 'con_set_cursor_pos',0

con.thread_err      db 'Cannot create console thread!',13,10,0
con.nomem_err       db 'Not enough memory!',13,10,0
con.aFinished       db ' [Finished]',0
con.aNull           db '(null)',0
con.beep                db      0x90, 0x3C, 0x00
con.ipc_buf         dd 0,8,0,0
                    db 0

section '.data' data readable writable align 16

con.finished_title          rb 256

con.cur_x                   rd 1
con.cur_y                   rd 1
con.wnd_xpos                rd 1
con.wnd_ypos                rd 1

con.wnd_width               rd 1
con.wnd_height              rd 1
con.scr_width               rd 1
con.scr_height              rd 1
con.title                   rd 1
con.data                    rd 1
con.image                   rd 1
con.console_tid             rd 1
con.data_width              rw 1
con.data_height             rw 1
con.vscrollbar_size         rd 1
con.vscrollbar_pos          rd 1
con.up_first_time           rd 1
con.down_first_time         rd 1
con.scroll_up_first_time    rd 1
con.scroll_down_first_time  rd 1
con.bUpPressed_saved        rb 1
con.bDownPressed_saved      rb 1
con.bScrollingUp_saved      rb 1
con.bScrollingDown_saved    rb 1

con.input_buffer                rw      128
con.input_buffer_end = $

con.kbd_layout          rb      128

; 1 = exit, 2 = set title, 3 = redraw, 4 = getch
con.thread_op               rb 1
con.bUpPressed              rb 1
con.bDownPressed            rb 1
con.bScrollingUp            rb 1
con.bScrollingDown          rb 1

con.stack                   rb 1024
con.stack_top = $
