; Функции работы с консолью для программ КолибриОС
; diamond, 2006-2008


format MS COFF

public EXPORTS

section '.flat' code readable align 16
include 'font.inc'
include 'conscrl.inc'
include '../../../struct.inc'

struct process_info
  cpu_usage              dd ?  ; +0
  window_stack_position  dw ?  ; +4
  window_stack_value     dw ?  ; +6
                         dw ?  ; +8
  process_name           rb 12 ; +10
  memory_start           dd ?  ; +22
  used_memory            dd ?  ; +26
  PID                    dd ?  ; +30
  box.x                  dd ?  ; +34
  box.y                  dd ?  ; +38
  box.width              dd ?  ; +42
  box.height             dd ?  ; +46
  slot_state             dw ?  ; +50
                         dw ?  ; +52
  client_box.x           dd ?  ; +54
  client_box.y           dd ?  ; +58
  client_box.width       dd ?  ; +62
  client_box.height      dd ?  ; +66
  wnd_state              db ?  ; +70
  rb (1024-71)
ends

OP_EXIT         = 1
OP_SET_TITLE    = 2
OP_REDRAW       = 3
OP_GETCH        = 4
OP_RESIZE       = 5

;void __stdcall START(dword state);
START:
; N.B. The current kernel implementation does not require
;      evident heap initialization, because if DLL is loaded, heap is already initialized
;      (if heap was not initialized, loader does this implicitly).
;      So this action does nothing useful, but nothing harmful.
        push    ebx
        push    68
        pop     eax
        push    11
        pop     ebx
        int     0x40
        pop     ebx
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
        pop     [con.main_scr_width]
        pop     [con.main_scr_height]
        pop     [con.title]
        push    eax

        push    ebx

        mov     [con.init_cmd],1

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

; Allocate memory for console data & bitmap data
; First, calculate required amount of bytes

; Main buffer
        mov     eax, [con.main_scr_width]
        mul     [con.main_scr_height]
;       2 bytes per on-screen character (1 flags and 1 ascii)
        lea     ecx, [eax+eax]

; Alternate buffer
        mov     [con.altbuffer], ecx
        mov     eax, [con.wnd_width]
        mul     [con.wnd_height]
;       2 bytes per on-screen character (1 flags and 1 ascii)
        lea     ecx, [ecx+2*eax]

; Bitmap data
        mov     eax, [con.wnd_width]
        mul     [con.wnd_height]
        imul    eax, font_width*font_height
        mov     ebx, eax
        push    ebx ecx
        add     ecx, eax

; malloc
        push    68
        pop     eax
        push    12
        pop     ebx
        int     0x40
        pop     ecx ebx
        mov     edx, con.nomem_err
        test    eax, eax
        jz      .fatal

; Set pointers to the buffers
        mov     [con.mainbuffer], eax
        add     [con.altbuffer], eax

; Set active buffer pointer and dimensions
        mov     [con.data], eax

        push    [con.main_scr_width]
        pop     [con.scr_width]

        push    [con.main_scr_height]
        pop     [con.scr_height]

; Clear text buffers
        push    edi
        mov     edi, eax
        shr     ecx, 1
        mov     ax, 0x0720
        rep     stosw

; Clear bitmap buffer
        mov     ecx, ebx
        mov     [con.image], edi
        xor     eax, eax
        rep     stosb
        pop     edi
        and     byte [con_flags+1], not 2

; Get parent TID
        mov     eax, 68  ; SF_SYS_MISC
        mov     ebx, 12  ; SSF_MEM_ALLOC
        mov     ecx, sizeof.process_info
        int     0x40

        mov     ebx, eax
        mov     eax, 9   ; SF_THREAD_INFO
        mov     ecx, -1
        int     0x40

        mov     eax, [ebx+process_info.PID]
        mov     [con.parent_tid], eax

        mov     eax, 68  ; SF_SYS_MISC
        mov     ecx, ebx
        mov     ebx, 13  ; SSF_MEM_FREE
        int     0x40

; Create console thread
        push    51
        pop     eax
        xor     ebx, ebx
        inc     ebx
        mov     ecx, con.thread
        mov     edx, con.stack_top
        int     0x40
        mov     edx, con.thread_err
        test    eax, eax
        js      .fatal
        mov     [con.console_tid], eax
        pop     ebx
        ret

  .fatal:
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
        jmp     .fatal
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

con_init_check:
        mov     ah, [con.init_cmd]
        test    ah, ah
        jne     .yes

        push    con.title_init_console
        push    -1
        push    -1
        push    -1
        push    -1

        call    con_init
  .yes:
        ret

; void __stdcall con_write_asciiz(const char* string);
con_write_asciiz:

        call    con_init_check

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

        call    con_init_check

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
        cmp     al, 7
        jz      .bell
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
  .bell:
        pusha
        push    55
        pop     eax
        mov     ebx, eax
        mov     esi, con.bell
        int     0x40
        popa
        ret
  .write_esc:
        mov     [con_esc], 1
        mov     [con_esc_attr_n], 1
        and     [con_esc_attrs], 0
        ret

  .esc_mode:
        cmp     [con_sci], 0
        jnz     .esc_sci
        cmp     al, '['         ; CSI - Control Sequence Introducer
        je      .esc_sqro
        cmp     al, ']'         ; OSC - Operating System Command
        je      .esc_sqrc
        cmp     al, '('         ; Designate G0 Character Set, VT100, ISO 2022.
        je      .esc_rndo
        cmp     al, '>'         ; Normal Keypad (DECKPNM), VT100.
        je      .keypm_norm
        cmp     al, '='         ; Application Keypad (DECKPAM).
        je      .keypm_alt
; Control characters
        cmp     al, 'G'
        je      .bell
        cmp     al, 'H'
        je      .write_bs
        cmp     al, 'I'
        je      .write_tab
        cmp     al, 'J'
        je      .write_lf
        cmp     al, 'M'
        je      .write_cr
; Unrecognized escape sequence, print it to screen
        push    eax
        mov     al, 27
        call    con.write_char
        pop     eax
        jmp     con.write_char

  .esc_sqro:
        mov     [con_sci], 1
        ret
  .esc_sqrc:
        mov     [con_sci], 2
        ret
  .esc_rndo:
        mov     [con_sci], 4
        ret

.keypm_norm:
; TODO: turn numlock on
        mov     [con_esc], 0
        ret

.keypm_alt:
; TODO: turn numlock off
        mov     [con_esc], 0
        ret

.esc_sci:
        cmp     [con_sci], 3
        je      .string
        cmp     [con_sci], 4
        je      .g0charset
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
.g0charset:
; Designate G0 Character Set
; Unimplemented: read and ignore.
        mov     [con_sci], 0
        mov     [con_esc], 0
        ret
.string:
        cmp     al, 0x07        ; bell
        je      .string_end
        cmp     al, 0x9C        ; string terminator
        je      .string_end
        push    ecx
        mov     ecx, [con_osc_strlen]
        cmp     ecx, 255
        jae     @f
        mov     [con_osc_str+ecx], al
        inc     [con_osc_strlen]
@@:
        pop     ecx
        ret
.string_end:
        mov     [con_sci], 0
        mov     [con_esc], 0
        pusha
        mov     ecx, [con_osc_strlen]
        mov     byte[con_osc_str+ecx], 0
        cmp     [con_esc_attrs+0], 0            ; Set Icon and Window Title
        je      .set_title
        cmp     [con_esc_attrs+0], 2            ; Set Window Title
        je      .set_title
        ret
.set_title:
        push    con_osc_str
        call    con_set_title
        popa
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
; Check for operating system command
        cmp     [con_sci], 2
        jne     @f
        cmp     [con_esc_attr_n], 2
        jne     @f
; Next argument is string
        mov     [con_sci], 3
        mov     [con_osc_strlen], 0
@@:
        pop     eax
        ret
.not_digit:
        mov     [con_esc], 0
        mov     [con_sci], 0    ; in any case, leave Esc mode

;        cmp     al, '@'
;        je      .insert_chars
        cmp     al, 'A'
        je      .cursor_up
        cmp     al, 'B'
        je      .cursor_down
        cmp     al, 'C'
        je      .cursor_right
        cmp     al, 'D'
        je      .cursor_left
;        cmp     al, 'E'
;        je      .cursor_next_line
;        cmp     al, 'F'
;        je      .cursor_prev_line
;        cmp     al, 'G'
;        je      .cursor_next_line
;        cmp     al, 'S'
;        je      .scroll_page_up
;        cmp     al, 'T'
;        je      .scroll_page_down
        cmp     al, 'H'
        je      .cursor_position
        cmp     al, 'J'
        je      .erase_in_display
        cmp     al, 'K'
        je      .erase_in_line
        cmp     al, 'L'
        je      .insert_lines
        cmp     al, 'M'
        je      .delete_lines
        cmp     al, 'P'
        je      .delete_chars
        cmp     al, 'X'
        je      .erase_chars

        cmp     al, 'd'
        je      .line_position_abs
;        cmp     al, 'e'
;        je      .line_position_rel
        cmp     al, 'f'
        je      .cursor_position
        cmp     al, 'h'
        je      .set_mode
        cmp     al, 'l'
        je      .reset_mode
        cmp     al, 'm'
        je      .set_attr
        cmp     al, 'r'
        je      .scroll_region
;        cmp     al, 's'
;        je      .save_cursor_pos
;        cmp     al, 't'
;        je      .window_manip
;        cmp     al, 'u'
;        je      .restore_cursor_pos

        ret     ; simply skip unknown sequences

.insert_lines:

        push    eax ebx ecx esi
        mov     eax, [con_esc_attrs+0]  ; amount of lines to scroll down
        test    eax, eax
        jnz     @f                      ; default is 1
        inc     eax
  @@:
; Check that we are inside the scroll region
        mov     ebx, [con.cur_y]
        cmp     ebx, [con.scroll_top]
        jb      .no_insert_lines
        add     ebx, eax
        cmp     ebx, [con.scroll_bot]
        ja      .no_insert_lines
; Move cursor to the left
        mov     [con.cur_x], 0
        call    con.get_data_ptr
; Calc amount of chars in requested numer of lines
        mov     ebx, [con.scr_width]
        imul    ebx, eax
; Move the lines down (in backwards order)
        push    edi
        mov     ecx, [con.scroll_bot]
        sub     ecx, [con.cur_y]
        sub     ecx, eax
        imul    ecx, [con.scr_width]
        lea     esi, [edi + 2*ecx - 2]
        lea     edi, [esi + 2*ebx]
        std
        rep     movsw
        cld
        pop     edi
; Insert empty lines
        push    edi
        mov     ecx, ebx
        mov     ah, byte[con_flags]
        mov     al, ' '
        rep     stosw
        pop     edi
.no_insert_lines:
        pop     esi ecx ebx eax
        ret

.delete_lines:

        push    eax ebx ecx esi
        mov     eax, [con_esc_attrs+0]  ; amount of lines to scroll up
        test    eax, eax
        jnz     @f                      ; default is 1
        inc     eax
  @@:
; Check that we are inside the scroll region
        mov     ebx, [con.cur_y]
        cmp     ebx, [con.scroll_top]
        jb      .no_delete_lines
        add     ebx, eax
        cmp     ebx, [con.scroll_bot]
        ja      .no_delete_lines
; Move cursor to the left
        mov     [con.cur_x], 0
        call    con.get_data_ptr
; Calc amount of chars in requested numer of lines
        mov     ebx, [con.scr_width]
        imul    ebx, eax
; Move the lines up
        mov     ecx, [con.scroll_bot]
        sub     ecx, [con.cur_y]
        imul    ecx, [con.scr_width]
        lea     esi, [edi + 2*ebx]
        rep     movsw
; Set new cursor row position
        add     [con.cur_y], eax
; Add empty lines till end of scroll region
        push    edi
        mov     ecx, ebx
        mov     ah, byte[con_flags]
        mov     al, ' '
        rep     stosw
        pop     edi
.no_delete_lines:
        pop     esi ecx ebx eax
        ret

.scroll_region:
        push    eax ebx
        cmp     [con_esc_attr_n], 2
        jb      .no_scroll_region
        mov     eax, [con_esc_attrs+0]  ; top
        dec     eax
        js      .no_scroll_region
        cmp     eax, [con.wnd_height]
        ja      .no_scroll_region

        mov     ebx, [con_esc_attrs+4]  ; bottom
        dec     ebx
        js      .no_scroll_region
        cmp     ebx, [con.wnd_height]
        ja      .no_scroll_region

        cmp     eax, ebx
        ja      .no_scroll_region

        mov     [con.scroll_top], eax
        mov     [con.scroll_bot], ebx

.no_scroll_region:
        pop     ebx eax
        ret

.reset_mode:
        mov     eax, [con_esc_attrs]
        cmp     eax, 0xffffffff
        jne     .no_dec_rst
        mov     eax, [con_esc_attrs+4]
        cmp     eax, 1
        je      .dec_rst_app_cursor_keys
;        cmp     eax, 7
;        je      .dec_rst_wraparound
;        cmp     eax, 12
;        je      .dec_rst_cursor_blink
        cmp     eax, 25
        je      .dec_rst_cursor
;        cmp     eax, 1000
;        je      .dec_rst_mouse
;        cmp     eax, 1002
;        je      .dec_rst_buttons
;        cmp     eax, 1006
;        je      .dec_rst_mouse_sgr
        cmp     eax, 1049
        je      .dec_rst_altbuff
;        cmp     eax, 2004
;        je      .dec_rst_brck_paste
.no_dec_rst:
;        cmp     eax, 2
;        je      .rst_keyb_action_mode
;        cmp     eax, 4
;        je      .set_replace_mode
        ret

.set_mode:
        mov     eax, [con_esc_attrs]
        cmp     eax, 0xffffffff
        jne     .no_dec_set
        mov     eax, [con_esc_attrs+4]
        cmp     eax, 1
        je      .dec_set_app_cursor_keys
;        cmp     eax, 7
;        je      .dec_set_wraparound
;        cmp     eax, 12
;        je      .dec_set_cursor_blink
        cmp     eax, 25
        je      .dec_set_cursor
;        cmp     eax, 1000
;        je      .dec_set_mouse
;        cmp     eax, 1002
;        je      .set_buttons
;        cmp     eax, 1006
;        je      .dec_rst_mouse_sgr
        cmp     eax, 1049
        je      .dec_set_altbuff
;        cmp     eax, 2004
;        je      .dec_set_brck_paste
.no_dec_set:
;        cmp     eax, 2
;        je      .set_keyb_action_mode
;        cmp     eax, 4
;        je      .set_insert_mode
        ret

.dec_set_app_cursor_keys:
        mov     [cursor_esc], 27 + ('O' shl 8)
        ret

.dec_rst_app_cursor_keys:
        mov     [cursor_esc], 27 + ('[' shl 8)
        ret

.dec_set_cursor:
        mov     [con.cursor_height], (15*font_height+50)/100    ; default height
        ret

.dec_rst_cursor:
        mov     [con.cursor_height], 0
        ret

.dec_set_altbuff:
; Switch buffer
        push    [con.altbuffer]
        pop     [con.data]
; Set new buffer size
        push    [con.wnd_width]
        pop     [con.scr_width]
        push    [con.wnd_height]
        pop     [con.scr_height]
; Save cursor
        push    [con.cur_x]
        pop     [con.main_cur_x]
        push    [con.cur_y]
        pop     [con.main_cur_y]
; Save window position
        push    [con.wnd_xpos]
        pop     [con.main_wnd_xpos]
        push    [con.wnd_ypos]
        pop     [con.main_wnd_ypos]
; Clear screen
        mov     edi, [con.altbuffer]
        mov     eax, [con.wnd_width]
        mul     [con.wnd_height]
        mov     ecx, eax
        mov     ah, byte[con_flags]
        mov     al, ' '
        rep     stosw
; Reset cursor position
        mov     [con.cur_x], 0
        mov     [con.cur_y], 0
; Reset window position
        mov     [con.wnd_xpos], 0
        mov     [con.wnd_ypos], 0
; Get new data ptr so we can sart writing to new buffer
        call    con.get_data_ptr
; Finally, tell the GUI the window has been resized
; (Redraw scrollbar and image)
        mov     [con.thread_op], OP_RESIZE
        jmp     con.wake

.dec_rst_altbuff:
; Switch buffer
        push    [con.mainbuffer]
        pop     [con.data]
; Set new buffer size
        push    [con.main_scr_width]
        pop     [con.scr_width]
        push    [con.main_scr_height]
        pop     [con.scr_height]
; Restore cursor
        push    [con.main_cur_x]
        pop     [con.cur_x]
        push    [con.main_cur_y]
        pop     [con.cur_y]
; Restore window position
        push    [con.main_wnd_xpos]
        pop     [con.wnd_xpos]
        push    [con.main_wnd_ypos]
        pop     [con.wnd_ypos]
; Get new data ptr so we can sart writing to new buffer
        call    con.get_data_ptr
; Finally, tell the GUI the window has been resized
; (Redraw scrollbar and image)
        mov     [con.thread_op], OP_RESIZE
        jmp     con.wake

.erase_chars:
        push    edi ecx
        mov     ecx, [con_esc_attrs]
        test    ecx, ecx
        jnz     @f
        inc     ecx
@@:
        mov     ah, byte[con_flags]
        mov     al, ' '
        rep     stosw
        pop     ecx edi
        ; Unclear where cursor should point to now..
        ret

.delete_chars:
        push    edi ecx
        mov     ecx, [con_esc_attrs]
        test    ecx, ecx
        jnz     @f
        inc     ecx
@@:
        sub     edi, 2
        mov     ah, byte[con_flags]
        mov     al, ' '
        std
        rep     stosw
        cld
        pop     ecx edi
        ret

.erase_in_line:
        mov     eax, [con_esc_attrs]
        test    eax, eax
        jz      .erase_after                    ; <esc>[0K (or <esc>[K)
        dec     eax
        jz      .erase_before                   ; <esc>[1K
        dec     eax
        je      .erase_current_line             ; <esc>[2K
        ret     ; unknown sequence

.erase_after:
        push    edi ecx
        mov     ecx, [con.scr_width]
        sub     ecx, [con.cur_x]
        mov     ah, byte[con_flags]
        mov     al, ' '
        rep     stosw
        pop     ecx edi
        ret

.erase_before:
        push    edi ecx
        mov     edi, [con.cur_y]
        imul    edi, [con.scr_width]
        shl     edi, 1
        add     edi, [con.data]
        mov     ecx, [con.cur_y]
        mov     ah, byte[con_flags]
        mov     al, ' '
        rep     stosw
        pop     ecx edi
        ret

.erase_current_line:
        push    edi ecx
        mov     edi, [con.cur_y]
        imul    edi, [con.scr_width]
        shl     edi, 1
        add     edi, [con.data]
        mov     ecx, [con.scr_width]
        mov     ah, byte[con_flags]
        mov     al, ' '
        rep     stosw
        pop     ecx edi
        ret

.erase_in_display:
        mov     eax, [con_esc_attrs]
        test    eax, eax
        jz      .erase_below            ; <esc>[0J (or <esc>[J)
        dec     eax
        jz      .erase_above            ; <esc>[1J
        dec     eax
        je      .erase_all              ; <esc>[2J
        ret     ; unknown sequence

.erase_below:
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

.erase_above:
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

.erase_all:   ; clear screen completely
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
.line_position_abs:
        mov     eax, [con_esc_attrs]
        dec     eax
        jns     @f
        inc     eax
@@:
        cmp     eax, [con.scr_height]
        jae     .nolinepos
        mov     [con.cur_y], eax
        jmp     con.get_data_ptr
.nolinepos:
        ret
.cursor_position:
; We always have at least one con_esc_attr, defaulting to 0
; Coordinates however are 1-based
; First comes Y (row) and then X (column)
        mov     eax, [con_esc_attrs]
        dec     eax
        jns     @f
        inc     eax
@@:
        cmp     eax, [con.scr_height]
        jae     .no_y
        mov     [con.cur_y], eax
.no_y:
        cmp     [con_esc_attr_n], 2
        jb      .no_x
        mov     eax, [con_esc_attrs+4]
        dec     eax
        jns     @f
        inc     eax
@@:
        cmp     eax, [con.scr_width]
        jae     .no_x
        mov     [con.cur_x], eax
.no_x:
.j_get_data:
        jmp     con.get_data_ptr
.cursor_up:
        mov     eax, [con_esc_attrs]
        test    eax, eax
        jnz     @f
        inc     eax     ; default = 1
@@:
        sub     [con.cur_y], eax
        jns     .j_get_data
        mov     [con.cur_y], 0
        jmp     .j_get_data
.cursor_down:
        mov     eax, [con_esc_attrs]
        test    eax, eax
        jnz     @f
        inc     eax     ; default = 1
@@:
        add     eax, [con.cur_y]
        cmp     eax, [con.scr_height]
        ja      @f
        mov     [con.cur_y], eax
        jmp     .j_get_data
@@:
        mov     eax, [con.scr_height]
        mov     [con.cur_y], eax
        jmp     .j_get_data
.cursor_right:
        mov     eax, [con_esc_attrs]
        test    eax, eax
        jnz     @f
        inc     eax     ; default = 1
@@:
        add     eax, [con.cur_x]
        cmp     eax, [con.scr_width]
        ja      @f
        mov     [con.cur_x], eax
        jmp     .j_get_data
@@:
        mov     eax, [con.scr_width]
        mov     [con.cur_x], eax
        jmp     .j_get_data
.cursor_left:
        test    eax, eax
        jnz     @f
        inc     eax     ; default = 1
@@:
        sub     [con.cur_x], eax
        jns     .j_get_data
        mov     [con.cur_x], 0
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
;        cmp     al, 8
;        jz      .attr_invisible
        cmp     al, 27
        jz      .attr_normal    ; FIXME: not inverse
;        cmp     al, 28
;        jz      .attr_visible

; Forground colors
        xor     edx, edx
        cmp     al, 30          ; Black
        jz      .attr_color
        mov     dl, 4
        cmp     al, 31          ; Red
        jz      .attr_color
        mov     dl, 2
        cmp     al, 32          ; Green
        jz      .attr_color
        mov     dl, 6
        cmp     al, 33          ; Yellow
        jz      .attr_color
        mov     dl, 1
        cmp     al, 34          ; Blue
        jz      .attr_color
        mov     dl, 5
        cmp     al, 35          ; Purple
        jz      .attr_color
        mov     dl, 3
        cmp     al, 36          ; Cyan
        jz      .attr_color
        mov     dl, 7
        cmp     al, 37          ; White
        jz      .attr_color
        mov     dl, 7
        cmp     al, 39          ; Default - White
        jz      .attr_color

; Background colors
        xor     edx, edx
        cmp     al, 40          ; Black
        jz      .attr_bgr_color
        mov     dl, 0x40
        cmp     al, 41          ; Red
        jz      .attr_bgr_color
        mov     dl, 0x20
        cmp     al, 42          ; Green
        jz      .attr_bgr_color
        mov     dl, 0x60
        cmp     al, 43          ; Yellow
        jz      .attr_bgr_color
        mov     dl, 0x10
        cmp     al, 44          ; Blue
        jz      .attr_bgr_color
        mov     dl, 0x50
        cmp     al, 45          ; Magenta
        jz      .attr_bgr_color
        mov     dl, 0x30
        cmp     al, 46          ; Cyan
        jz      .attr_bgr_color
        mov     dl, 0x70
        cmp     al, 47          ; White
        jz      .attr_bgr_color
        mov     dl, 0
        cmp     al, 49          ; Default - Black
        jz      .attr_bgr_color

; 16-color support, bright colors follow
; Foreground colors
        mov     dl, 0x08
        cmp     al, 90          ; Black
        jz      .attr_color
        mov     dl, 4 + 8
        cmp     al, 91          ; Red
        jz      .attr_color
        mov     dl, 2 + 8
        cmp     al, 92          ; Green
        jz      .attr_color
        mov     dl, 6 + 8
        cmp     al, 93          ; Yellow
        jz      .attr_color
        mov     dl, 1 + 8
        cmp     al, 94          ; Blue
        jz      .attr_color
        mov     dl, 5 + 8
        cmp     al, 95          ; Magenta
        jz      .attr_color
        mov     dl, 3 + 8
        cmp     al, 96          ; Cyan
        jz      .attr_color
        mov     dl, 7 + 8
        cmp     al, 97          ; White
        jz      .attr_color

; Background colors
        mov     dl, 0x80
        cmp     al, 100         ; Black
        jz      .attr_bgr_color
        mov     dl, 0x80 + 0x40
        cmp     al, 101         ; Red
        jz      .attr_bgr_color
        mov     dl, 0x80 + 0x20
        cmp     al, 102         ; Green
        jz      .attr_bgr_color
        mov     dl, 0x80 + 0x60
        cmp     al, 103         ; Yellow
        jz      .attr_bgr_color
        mov     dl, 0x80 + 0x10
        cmp     al, 104         ; Blue
        jz      .attr_bgr_color
        mov     dl, 0x80 + 0x50
        cmp     al, 105         ; Magenta
        jz      .attr_bgr_color
        mov     dl, 0x80 + 0x30
        cmp     al, 106         ; Cyan
        jz      .attr_bgr_color
        mov     dl, 0x80 + 0x70
        cmp     al, 107         ; White
        jnz     .attr_continue

.attr_bgr_color:
        mov     eax, [con_flags]
        and     al, 0x0F
        or      al, byte [con_flags_attr]
        or      al, dl
        mov     [con_flags], eax
        jmp     .attr_continue
.attr_color:
        mov     eax, [con_flags]
        and     al, 0xF0
        or      al, byte [con_flags_attr]
        or      al, dl
        mov     [con_flags], eax
        jmp     .attr_continue
.attr_normal:
        mov     byte [con_flags_attr], 0
        mov     byte [con_flags], 0x07
        jmp     .attr_continue
.attr_reversed:
        mov     byte [con_flags], 0x70
        jmp     .attr_continue
.attr_bold:
        or      byte [con_flags_attr], 0x08
        jmp     .attr_continue
.attr_bgr_bold:
        or      byte [con_flags_attr], 0x80
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
        
        mov     ah, [con.init_cmd]
        test    ah, ah
        je      .ret

        cmp     byte [esp+4], 0
        jz      .noexit
        mov     [con.thread_op], OP_EXIT
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
        mov     [con.thread_op], OP_SET_TITLE
        call    con.wake
        ret     4

; int __stdcall con_kbhit(void);
con_kbhit:
        test    byte [con_flags+1], 2
        jnz     @f
        mov     eax, [con.input_start]
        cmp     eax, [con.input_end]
        jnz     @f
        cmp     [con.entered_char], 0xffff
@@:
        setnz   al
        movzx   eax, al
        ret

con.force_entered_char:
        cmp     [con.entered_char], -1
        jnz     .ret
        mov     [con.thread_op], OP_GETCH
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
        call    con_init_check
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
        call    con_init_check
        call    con.force_entered_char
        test    byte [con_flags+1], 2
        jnz     con_getch_closed
        mov     eax, 0xFFFF
        xchg    ax, [con.entered_char]
        ret

; int __stdcall con_get_input(int *bufptr, int buflen);
con_get_input:
        call    con_init_check
; Wait for input available
        call    con.force_entered_char
        test    byte [con_flags+1], 2
        jnz     .none

        push    ebx
        mov     ebx, [esp+8]
  .check_more:
; Avoid buffer overflow
        cmp     dword[esp+12], 16
        jl      .no_more
; Check element available
        cmp     [con.entered_char], 0xFFFF
        je      .no_more
; Get an element from the input queue
        mov     eax, 0xFFFF
        xchg    ax, [con.entered_char]
; Function keys F1-F4
        cmp     ah, 0x3B
        jb      @f
        cmp     ah, 0x3E
        jbe     .f1_4
  @@:
; Function keys F5-F8
        cmp     ah, 0x3F
        jb      @f
        je      .f5
        cmp     ah, 0x42
        jbe     .f6_8
  @@:
; Function keys F9-F12
        cmp     ah, 0x43
        je      .f9
        cmp     ah, 0x44
        je      .f10
        cmp     ah, 0x57
        je      .f11
        cmp     ah, 0x58
        je      .f12
; Cursor keys
        cmp     ah, 0x47
        je      .home
        cmp     ah, 0x48
        je      .up
        cmp     ah, 0x49
        je      .pgup
;        cmp     ah, 0x4a
;        je      .minus
        cmp     ah, 0x4b
        je      .left
        cmp     ah, 0x4c
        je      .begin
        cmp     ah, 0x4d
        je      .right
;        cmp     ah, 0x4e
;        je      .plus
        cmp     ah, 0x4f
        je      .end
        cmp     ah, 0x50
        je      .down
        cmp     ah, 0x51
        je      .pgdown
        cmp     ah, 0x52
        je      .insert
        cmp     ah, 0x53
        je      .delete
; regular ASCII
        mov     byte[ebx], al
        mov     eax, 1
  .got_input:
        and     eax, 0xff
        sub     [esp+12], eax
        add     ebx, eax
        jmp     .check_more
  .no_more:
        mov     eax, ebx
        sub     eax, [esp+8]
        pop     ebx
        ret     8

  .none:
        xor     eax, eax
        ret     8

  .f1_4:
; F1 = SSR P, F2 = SS3 Q ..
; SS3 = 0x8f (8bit) or 0x1b + 'O' (7-bit)
        mov     word[ebx], 27 + ('O' shl 8)
        add     ah, 'P' - 59
        mov     byte[ebx+2], ah
        mov     al, 3
        jmp     .got_input
  .f5:
; CSI = 0x9b (8bit) or 0x1b + '[' (7-bit)
        mov     byte[ebx], 27
        mov     dword[ebx+1], '[15~'
        mov     al, 5
        jmp     .got_input
  .f6_8:
        mov     byte[ebx], 27
        xor     al, al
        shl     eax, 8
        add     eax, '[17~' - (0x40 shl 16)
        mov     dword[ebx+1], eax
        mov     al, 5
        jmp     .got_input
  .f9:
        mov     byte[ebx], 27
        mov     dword[ebx+1], '[20~'
        mov     al, 5
        jmp     .got_input
  .f10:
        mov     byte[ebx], 27
        mov     dword[ebx+1], '[21~'
        mov     al, 5
        jmp     .got_input
  .f11:
        mov     byte[ebx], 27
        mov     dword[ebx+1], '[23~'
        mov     al, 5
        jmp     .got_input
  .f12:
        mov     byte[ebx], 27
        mov     dword[ebx+1], '[24~'
        mov     al, 5
        jmp     .got_input
  .up:
        mov     eax, 'A' shl 16
        add     eax, [cursor_esc]
        mov     dword[ebx], eax
        mov     al, 3
        jmp     .got_input
  .down:
        mov     eax, 'B' shl 16
        add     eax, [cursor_esc]
        mov     dword[ebx], eax
        mov     al, 3
        jmp     .got_input
  .right:
        mov     eax, 'C' shl 16
        add     eax, [cursor_esc]
        mov     dword[ebx], eax
        mov     al, 3
        jmp     .got_input
  .left:
        mov     eax, 'D' shl 16
        add     eax, [cursor_esc]
        mov     dword[ebx], eax
        mov     al, 3
        jmp     .got_input
  .home:
        mov     eax, 'H' shl 16
        add     eax, [cursor_esc]
        mov     dword[ebx], eax
        mov     al, 3
        jmp     .got_input
  .end:
        mov     eax, 'F' shl 16
        add     eax, [cursor_esc]
        mov     dword[ebx], eax
        mov     al, 3
        jmp     .got_input
  .insert:
        mov     dword[ebx], 27 + ('[2~' shl 8)
        mov     al, 4
        jmp     .got_input
  .delete:
        mov     dword[ebx], 27 + ('[3~' shl 8)
        mov     al, 4
        jmp     .got_input
  .pgup:
        mov     dword[ebx], 27 + ('[5~' shl 8)
        mov     al, 4
        jmp     .got_input
  .pgdown:
        mov     dword[ebx], 27 + ('[6~' shl 8)
        mov     al, 4
        jmp     .got_input
  .begin:
        mov     dword[ebx], 27 + ('[E' shl 8)
        mov     al, 3
        jmp     .got_input



; char* __stdcall con_gets(char* str, int n);
con_gets:
        pop     eax
        push    0
        push    eax
; char* __stdcall con_gets2(con_gets2_callback callback, char* str, int n);
con_gets2:
        call    con_init_check
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
        mov     ah, [con.init_cmd]
        test    ah, ah
        je      cmd_init_no
                
        push    edi
        call    con.write_special_char.erase_all
        pop     edi
        call    con.update_screen
                
        ret
                
cmd_init_no:
                
        push    con.title_init_console
        push    -1
        push    -1
        push    -1
        push    -1
                
        call    con_init
                
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
        mov     [con.thread_op], OP_REDRAW

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
        push    0x80000067 ;  program dont getting events mouse, when it dont active
        pop     ebx
        int     0x40
        xor     ebx,ebx     ;clear ebx
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
        mov     eax, 18 ; SF_SYSTEM
        mov     ebx, 18 ; SSF_TERMINATE_THREAD_ID
        mov     ecx, [con.parent_tid]
        int     0x40    ; (kill the parent thread)

con.thread_exit:
        or      byte [con_flags+1], 2
        and     [con.console_tid], 0
        and     [con.entered_char], 0
        or      eax, -1
        int     0x40
con.key:
        mov     al, 2
        int     0x40
        and     eax, 0xffff ; supress scancodes
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
        cmp     dl, 0x4e
        je      .numpad
        cmp     dl, 0x4a
        je      .numpad
        push    66
        pop     eax
        push    3
        pop     ebx
        int     0x40    ; eax = control key state
        test    dh, dh
        jnz     .extended
        test    al, 0x80        ; numlock
        jnz     .numlock
        bt      [scan_has_ascii], edx
        jnc     .extended
        test    al, 0x30        ; alt
        jnz     .extended
        test    al, 0x80        ; numlock
        jz      .no_numlock
  .numlock:
        cmp     dl, 71
        jb      .no_numlock
        cmp     dl, 83
        ja      .no_numlock
  .numpad:
        mov     dh, [con.extended_numlock+edx-71]
        xchg    dl, dh
        jmp     .gotcode
  .no_numlock:
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

        cmp     dl, 28
        jne     @f
        shl     dx, 8
        mov     dl, 13
        jmp     .gotcode
@@:
        cmp     dl, 53
        jne     @f
        shl     dx, 8
        mov     dl, '/'
        jmp     .gotcode
@@:
        cmp     dl, 55
        jne     @f
        shl     dx, 8
        mov     dl, '*'
        jmp     .gotcode
@@:
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
        dec     eax
        jz      con.resize
        jmp     con.msg_loop
con.resize:
        push    48
        pop     eax
        push    4
        pop     ebx
        int     0x40

        mov     edx, [con.def_wnd_x-2]
        mov     edx, [con.wnd_width]
        imul    edx, font_width
        add     edx, 5+5-1

        mov     esi, [con.def_wnd_y-2]
        mov     esi, [con.wnd_height]
        imul    esi, font_height
        lea     esi, [eax + esi + 5-1]
; place for scrollbar
        mov     eax, [con.wnd_height]
        cmp     eax, [con.scr_height]
        jae     @f
        add     edx, con.vscroll_width
@@:
        push    67
        pop     eax
        mov     ebx, -1
        mov     ecx, ebx
        int     0x40
        call    con.draw_window
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
        push    37
        pop     eax
        push    7
        pop     ebx
        int     0x40
        test    eax, eax
        jz      .no_scrollmouse
        cwde
        add     eax, [con.wnd_ypos]
        jg      @f
        xor     eax, eax
@@:
        mov     ebx, [con.scr_height]
        sub     ebx, [con.wnd_height]
        cmp     eax, ebx
        jb      @f
        mov     eax, ebx
@@:
        mov     [con.wnd_ypos], eax
        jmp     con.redraw_image
.no_scrollmouse:
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

        mov     eax, 9
        mov     ebx, process_info_buffer
        mov     ecx, -1
        int     0x40
        test    [process_info_buffer.wnd_state], 110b   ; window is rolled up or minimized to panel
        jnz     .exit

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
con.extended_numlock:
        db      '7', '8', '9', '-'
        db      '4', '5', '6', '+'
        db      '1', '2', '3'
        db      '0', '.'


cursor_esc      dd 27 + ('[' shl 8)

; В текущей реализации значения по умолчанию таковы.
; В будущем они, возможно, будут считываться как параметры из ini-файла console.ini.
con.def_wnd_width   dd    80
con.def_wnd_height  dd    25
con.def_scr_width   dd    80
con.def_scr_height  dd    300
con.def_wnd_x       dd    200
con.def_wnd_y       dd    50

con.init_cmd db 0
con.title_init_console db "Console",0
con.vscroll_pt      dd    -1

align 16
EXPORTS:
        dd      szStart,                START
        dd      szVersion,              0x00020009
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
        dd      szcon_set_title,        con_set_title
        dd      szcon_get_input,        con_get_input
        dd      0

con_flags       dd      0x07    ; black on white
con_flags_attr  dd      0       ; Modifiers (for example, high intensity colors)
con.cursor_height dd    (15*font_height+50)/100
con.input_start dd      con.input_buffer
con.input_end   dd      con.input_buffer

con_esc_attr_n  dd      0
con_esc_attrs   dd      0,0,0,0
con_esc         db      0
con_sci         db      0
con_osc_str     rb      256
con_osc_strlen  dd      0

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
szcon_set_title         db 'con_set_title',0
szcon_get_input         db 'con_get_input',0

con.thread_err      db 'Cannot create console thread!',13,10,0
con.nomem_err       db 'Not enough memory!',13,10,0
con.aFinished       db ' [Finished]',0
con.aNull           db '(null)',0
con.beep            db 0x90, 0x3C, 0x00
con.bell            db 0x85, 0x25, 0x85, 0x40, 0x00
con.ipc_buf         dd 0,8,0,0
                    db 0

section '.data' data readable writable align 16

process_info_buffer         process_info

con.finished_title          rb 256

con.cur_x                   dd ?        ; Active cursor column (0 based)
con.cur_y                   dd ?        ; Active cursor row (0 based)
con.main_cur_x              dd ?        ; Saved cursor position for main buffer
con.main_cur_y              dd ?        ; Saved cursor position for main buffer
con.wnd_xpos                dd ?        ; Active window position in active buffer
con.wnd_ypos                dd ?        ; Active window position in active buffer
con.main_wnd_xpos           dd ?        ; Saved window position for main buffer
con.main_wnd_ypos           dd ?        ; Saved window position for main buffer
con.scroll_top              dd ?        ; VT100 scroll region
con.scroll_bot              dd ?        ; VT100 scroll region

con.wnd_width               dd ?        ; window width (= alt buffer width)
con.wnd_height              dd ?        ; window height (= alt buffer height)
con.main_scr_width          dd ?        ; main buffer width
con.main_scr_height         dd ?        ; main buffer height
con.scr_width               dd ?        ; active buffer width
con.scr_height              dd ?        ; active buffer height
con.title                   dd ?
con.data                    dd ?        ; active buffer ptr
con.mainbuffer              dd ?
con.altbuffer               dd ?
con.image                   dd ?
con.console_tid             dd ?
con.data_width              dw ?        ; width in pixels
con.data_height             dw ?        ; height in pixels
con.vscrollbar_size         dd ?
con.vscrollbar_pos          dd ?
con.up_first_time           dd ?
con.down_first_time         dd ?
con.scroll_up_first_time    dd ?
con.scroll_down_first_time  dd ?
con.bUpPressed_saved        db ?
con.bDownPressed_saved      db ?
con.bScrollingUp_saved      db ?
con.bScrollingDown_saved    db ?

con.parent_tid              dd ?
con.input_buffer            rw 128
con.input_buffer_end = $

con.kbd_layout              rb 128

con.thread_op               db ?
con.bUpPressed              db ?
con.bDownPressed            db ?
con.bScrollingUp            db ?
con.bScrollingDown          db ?

con.stack                   rb 1024
con.stack_top = $
