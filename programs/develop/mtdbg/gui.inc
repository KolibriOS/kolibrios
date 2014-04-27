;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; GUI ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

macro DrawRectangle x, y, w, h, color
{
        mcall 13, x shl 16 + w,     y shl 16 + 1,     color   ; top
        mcall   , x shl 16 + 1,     y shl 16 + h,     color   ; left
        mcall   , (x+w) shl 16 +1,  y shl 16 + (h+1), color   ; right
        mcall   , x shl 16 + w,   (y+h) shl 16 + 1,   color   ; bottom
}

;-----------------------------------------------------------------------------
;                             Color scheme

BLACK_ON_WHITE  equ 0
MOVIEOS         equ 1
WHITE_ON_BLACK  equ 2

;                         format - 0xRRGGBB
if COLOR_THEME eq MOVIEOS

        COLOR_BG_NORMAL           = 0x1d272f
        COLOR_BG_BREAKPOINT       = 0x0000aa
        COLOR_BG_SELECTED         = 0xec9300
        COLOR_LINE                = 0x00b9a0
        COLOR_TXT_NORMAL          = 0xffffff
        COLOR_TXT_INACTIVE        = 0x8f7948
        COLOR_TXT_CHANGED         = 0xec9300
        COLOR_TXT_LABEL           = 0x22b14c
        COLOR_TXT_SELECTED        = 0x1d272f
        COLOR_TXT_HEX             = 0xec9300
        COLOR_TXT_BREAKPOINT      = 0xec9300

else if COLOR_THEME eq WHITE_ON_BLACK

        COLOR_BG_NORMAL           = 0x101010 ; dark grey
        COLOR_BG_BREAKPOINT       = 0xFF0000 ; red
        COLOR_BG_SELECTED         = 0x0000FF ; blue
        COLOR_LINE                = 0xFFFFFF ; white
        COLOR_TXT_NORMAL          = 0xFFFFFF ; white
        COLOR_TXT_INACTIVE        = 0x808080 ; grey
        COLOR_TXT_CHANGED         = 0x00AA00 ; green
        COLOR_TXT_LABEL           = COLOR_TXT_NORMAL
        COLOR_TXT_SELECTED        = 0xFFFFFF ; white
        COLOR_TXT_HEX             = COLOR_TXT_NORMAL
        COLOR_TXT_BREAKPOINT      = COLOR_TXT_NORMAL

else  ; BLACK ON WHITE

        COLOR_BG_NORMAL           = 0xffffff ; white
        COLOR_BG_BREAKPOINT       = 0xFF0000 ; red
        COLOR_BG_SELECTED         = 0x0000FF ; blue
        COLOR_LINE                = 0x000000 ; black
        COLOR_TXT_NORMAL          = 0x000000 ; black
        COLOR_TXT_INACTIVE        = 0x808080 ; grey
        COLOR_TXT_CHANGED         = 0x00AA00 ; green
        COLOR_TXT_LABEL           = COLOR_TXT_NORMAL
        COLOR_TXT_SELECTED        = 0xFFFFFF ; white
        COLOR_TXT_HEX             = COLOR_TXT_NORMAL
        COLOR_TXT_BREAKPOINT      = COLOR_TXT_NORMAL

end if

;-----------------------------------------------------------------------------

data_width      equ 80
data_x_pos      equ 12
data_x_size     equ data_width*6

title_x_pos     equ 30
title_y_pos     equ 32
title_y_size    equ 10

;dump_y_pos      equ (registers_y_pos + registers_y_size + 5)
dump_y_pos      equ (title_y_pos + title_y_size)
dump_height     equ 6
dump_y_size     equ (dump_height*10)

disasm_y_pos    equ (dump_y_pos + dump_y_size + 4)
disasm_height   equ 18
disasm_y_size   equ (disasm_height*10)

messages_width  equ data_width
messages_height equ 8
messages_x_pos  equ data_x_pos
messages_y_pos  equ (disasm_y_pos + disasm_y_size + 4)
messages_x_size equ messages_width*6
messages_y_size equ messages_height*10

cmdline_width   equ data_width
cmdline_x_pos   equ data_x_pos
cmdline_y_pos   equ (messages_y_pos + messages_y_size + 4)
cmdline_x_size  equ messages_x_size
cmdline_y_size  equ 10

registers_x_pos equ (data_x_pos + messages_x_size + 4)
registers_y_pos equ (title_y_pos + title_y_size - 3)
registers_x_size equ 134
registers_y_size equ (cmdline_y_pos + cmdline_y_size - registers_y_pos+1)

wnd_x_size      equ (data_x_pos + messages_x_size + data_x_pos + registers_x_size+3)
wnd_y_size      equ (cmdline_y_pos + cmdline_y_size + data_x_pos)

;-----------------------------------------------------------------------------
;                          Entry point

; TODO: split all gui part in independent function, move entry point into mtdbg.asm

start:
        ; initialize process heap
        mcall   68, 11

        push    eax
        fstcw   word [esp]
        or      word [esp], 3 shl 10    ; set round-to-zero mode
        fldcw   word [esp]
        pop     eax

        mov     edi, messages
        mov     ecx, messages_width*messages_height
        mov     al, ' '
        rep stosb
        xor     eax, eax
        mov     [messages_pos], eax
        mov     [cmdline_len], eax
        mov     [cmdline_pos], eax
        mov     edi, needzerostart
        mov     ecx, (needzeroend-needzerostart+3)/4
        rep stosd
        mov     esi, begin_str
        call    put_message_nodraw
        ; set event mask - default events and debugging events
        mcall   40, 0x107
        ; set debug messages buffer
        mov     ecx, dbgbufsize
        mov     dword [ecx], 256
        xor     ebx, ebx
        mov     [ecx+4], ebx
        mov     al, 69
        mcall
        mov     esi, i_param
        call    get_arg.skip_spaces
        test    al, al
        jz      dodraw
        push    esi
        call    draw_window
        pop     esi
        call    OnLoadInit
        jmp     waitevent

dodraw:
        call    draw_window

waitevent:
        mcall   10
        cmp     al, 9
        jz      debugmsg
        dec     eax
        jz      dodraw
        dec     eax
        jz      keypressed
        dec     eax
        jnz     waitevent
        ; button pressed - we have only one button (close)
        mcall   -1

; TODO: split in more independent function
keypressed:
        mov     al, 2
        mcall
        shr     eax, 8
        cmp     al, 8
        jz      .backspace
        cmp     al, 0xB0
        jz      .left
        cmp     al, 0xB3
        jz      .right
        cmp     al, 0x0D
        jz      .enter
        cmp     al, 0xB6
        jz      .del
        cmp     al, 0xB4
        jz      .home
        cmp     al, 0xB5
        jz      .end
        cmp     al, 0xB1
        jz      .down
        cmp     al, 0xB2
        jz      .up
        cmp     ah, 0x41
        jz      F7
        cmp     ah, 0x42
        jz      F8
        cmp     [cmdline_len], cmdline_width
        jae     waitevent
        push    eax
        call    clear_cmdline_end
        pop     eax
        mov     edi, cmdline
        mov     ecx, [cmdline_len]
        add     edi, ecx
        lea     esi, [edi-1]
        sub     ecx, [cmdline_pos]
        std
        rep movsb
        cld
        stosb
        inc     [cmdline_len]
        call    draw_cmdline_end
        inc     [cmdline_pos]
        call    draw_cursor
        jmp     waitevent

    .backspace:
        cmp     [cmdline_pos], 0
        jz      waitevent
        dec     [cmdline_pos]

    .delchar:
        call    clear_cmdline_end
        mov     edi, [cmdline_pos]
        dec     [cmdline_len]
        mov     ecx, [cmdline_len]
        sub     ecx, edi
        add     edi, cmdline
        lea     esi, [edi+1]
        rep movsb
        call    draw_cmdline_end
        call    draw_cursor
        jmp     waitevent

    .del:
        mov     eax, [cmdline_pos]
        cmp     eax, [cmdline_len]
        jae     waitevent
        jmp     .delchar

    .left:
        cmp     [cmdline_pos], 0
        jz      waitevent
        call    hide_cursor
        dec     [cmdline_pos]
        call    draw_cursor
        jmp     waitevent

    .right:
        mov     eax, [cmdline_pos]
        cmp     eax, [cmdline_len]
        jae     waitevent
        call    hide_cursor
        inc     [cmdline_pos]
        call    draw_cursor
        jmp     waitevent

    .home:
        call    hide_cursor
        and     [cmdline_pos], 0
        call    draw_cursor
        jmp     waitevent

    .end:
        call    hide_cursor
        mov     eax, [cmdline_len]
        mov     [cmdline_pos], eax
        call    draw_cursor

    .up:
    .down:
        jmp     waitevent

        ; We also trying to execute previous command, if empty command_line
    .enter:
        mov     ecx, [cmdline_len]
        test    ecx, ecx
        jnz     .exec_cur
        mov     cl, byte [cmdline_prev]
        cmp     cl, 0
        jz      waitevent

    .exec_prev:
        mov     esi, cmdline_prev
        jmp     .exec

    .exec_cur:
        mov     esi, cmdline

    .exec:
        mov     byte [esi+ecx], 0
        and     [cmdline_pos], 0
        push    esi
        call    clear_cmdline_end
        call    draw_cursor
        pop     esi
        and     [cmdline_len], 0
        ; skip leading spaces
        call    get_arg.skip_spaces
        cmp     al, 0
        jz      waitevent
        ; now esi points to command
        push    esi
        mov     esi, prompt
        call    put_message_nodraw
        pop     esi
        push    esi
        call    put_message_nodraw

; TODO: add meaningful name
z1:
        mov     esi, newline
        call    put_message
        pop     esi
        push    esi
        call    get_arg
        mov     [curarg], esi
        pop     edi
        mov     esi, commands
        call    find_cmd
        mov     eax, aUnknownCommand
        jc      .x11

        ; check command requirements
        ; flags field:
        ; &1: command may be called without parameters
        ; &2: command may be called with parameters
        ; &4: command may be called without loaded program
        ; &8: command may be called with loaded program
        mov     eax, [esi+8]
        mov     ecx, [curarg]
        cmp     byte [ecx], 0
        jz      .noargs
        test    byte [esi+16], 2
        jz      .x11
        jmp     @f

    .noargs:
        test    byte [esi+16], 1
        jz      .x11

    @@:
        cmp     [debuggee_pid], 0
        jz      .nodebuggee
        mov     eax, aAlreadyLoaded
        test    byte [esi+16], 8
        jz      .x11
        jmp     .x9

    .nodebuggee:
        mov     eax, need_debuggee
        test    byte [esi+16], 4
        jnz     .x9

    .x11:
        xchg    esi, eax
        call    put_message

        ; store cmdline for repeating
    .x10:
        mov     esi, cmdline
        mov     ecx, [cmdline_len]

    @@:
        cmp     ecx, 0
        jle     .we
        mov     al, [esi + ecx]
        mov     [cmdline_prev + ecx], al
        dec     ecx
        jmp     @b

    .we:
        mov     [cmdline_len], 0
        jmp     waitevent

    .x9:
        call    dword [esi+4]
        jmp     .x10

;-----------------------------------------------------------------------------
;                            Cmdline handling

clear_cmdline_end:
        mov     ebx, [cmdline_pos]
        mov     ecx, [cmdline_len]
        sub     ecx, ebx
        imul    ebx, 6
        imul    ecx, 6
        inc     ecx
        add     ebx, cmdline_x_pos
        shl     ebx, 16
        or      ebx, ecx
        mov     ecx, cmdline_y_pos*10000h + cmdline_y_size
        mov     edx, COLOR_BG_NORMAL
        ; draw container rectangle/box for cmdline
        mcall   13
        ret

draw_cmdline:
        xor     ebx, ebx
        jmp     @f

; TODO: make it local
draw_cmdline_end:
        mov     ebx, [cmdline_pos]

    @@:
        mov     esi, [cmdline_len]
        sub     esi, ebx

        mov     ecx, COLOR_TXT_NORMAL
        lea     edx, [cmdline+ebx]
        imul    ebx, 6
        add     ebx, cmdline_x_pos
        shl     ebx, 16
        or      ebx, cmdline_y_pos+1
        ; draw a text string in the window
        mcall   4
        ret

;-----------------------------------------------------------------------------
;                        Working with messages
; in: esi->ASCIIZ message
put_message_nodraw:
        mov     edx, [messages_pos]

    .m:
        lea     edi, [messages+edx]

    .l:
        lodsb
        cmp     al, 0
        jz      .done
        call    test_scroll
        cmp     al, 10
        jz      .newline
        cmp     al, '%'
        jnz     @f
        cmp     dword [esp], z1
        jnz     .format

    @@:
        stosb
        inc     edx
        jmp     .l

    .newline:
        push    edx
        mov     ecx, messages_width
        xor     eax, eax
        xchg    eax, edx
        div     ecx
        xchg    eax, edx
        pop     edx
        test    eax, eax
        jz      .m
        sub     edx, eax
        add     edx, ecx
        jmp     .m

    .done:
        mov     [messages_pos], edx
        ret

        ; at this moment all format specs must be %<digit>X
    .format:
        lodsb   ; get <digit>
        sub     al, '0'
        movzx   ecx, al
        lodsb
        pop     eax
        pop     ebp
        push    eax
        ; write number in ebp with ecx digits
        dec     ecx
        shl     ecx, 2

    .writenibble:
        push    ecx
        call    test_scroll
        pop     ecx
        mov     eax, ebp
        shr     eax, cl
        and     al, 0xF
        cmp     al, 10
        sbb     al, 69h
        das
        stosb
        inc     edx
        sub     ecx, 4
        jns     .writenibble
        jmp     .l

test_scroll:
        cmp     edx, messages_width*messages_height
        jnz     .ret
        push    esi
        mov     edi, messages
        lea     esi, [edi+messages_width]
        mov     ecx, (messages_height-1)*messages_width/4
        rep movsd
        push    eax
        mov     al, ' '
        push    edi
        push    messages_width
        pop     ecx
        sub     edx, ecx
        rep stosb
        pop     edi
        pop     eax
        pop     esi

    .ret:
        ret

;-----------------------------------------------------------------------------

put_message:
        call    put_message_nodraw

draw_messages:
        ; draw container rectangle/box
        mcall   13, messages_x_pos*10000h+messages_x_size, messages_y_pos*10000h+messages_y_size, COLOR_BG_NORMAL
        mov     edx, messages
        push    messages_width
        pop     esi
        mov     ecx, COLOR_TXT_NORMAL
        mov     ebx, messages_x_pos*10000h+messages_y_pos

    @@:
        ; display text string in the window
        mcall   4
        add     edx, esi
        add     ebx, 10
        cmp     edx, messages+messages_width*messages_height
        jb      @b
        ret

;-----------------------------------------------------------------------------
;                     Show/hide cursor in command line

; TODO: make it cursor.draw and cursor.hide ???
draw_cursor:
        mov     ecx, cmdline_y_pos*10001h+cmdline_y_size-1
        mov     ebx, [cmdline_pos]
        imul    ebx, 6
        add     ebx, cmdline_x_pos
        mov     edx, ebx
        shl     ebx, 16
        or      ebx, edx
        mov     edx, COLOR_TXT_NORMAL
        ; draw line
        mcall   38
        ret

hide_cursor:
        mov     ebx, [cmdline_pos]
        imul    ebx, 6
        add     ebx, cmdline_x_pos
        shl     ebx, 16
        inc     ebx
        mov     ecx, cmdline_y_pos*10000h + cmdline_y_size
        mov     edx, COLOR_BG_NORMAL
        ; draw container rectangle/box
        mcall   13
        mov     ebx, [cmdline_pos]
        cmp     ebx, [cmdline_len]
        jae     .ret
        ; setting up text color scheme and attributes
        mov     ecx, COLOR_TXT_NORMAL
        lea     edx, [cmdline+ebx]
        imul    ebx, 6
        add     ebx, cmdline_x_pos
        shl     ebx, 16
        or      ebx, cmdline_y_pos+1
        push    1
        pop     esi
        ; draw text string in the window
        mcall   4

    .ret:
        ret

;-----------------------------------------------------------------------------
;                       Draw program window title

; FIXME: something wrong here
redraw_title:
        ; draw container rectangle/box
        mcall   13, title_x_pos*10000h+data_x_pos+data_x_size-title_x_pos, title_y_pos*10000h+title_y_size, COLOR_BG_NORMAL

draw_title:
        mcall   38, (data_x_pos-2)*10000h+title_x_pos-5, (title_y_pos+5)*10001h, COLOR_LINE
        push    NoPrgLoaded_len
        pop     esi
        cmp     [debuggee_pid], 0
        jz      @f
        mov     esi, [prgname_len]

    @@:
        imul    ebx, esi, 6
        add     ebx, title_x_pos+4
        shl     ebx, 16
        mov     bx, data_x_pos+data_x_size-10-5-6*7
        cmp     [bSuspended], 0
        jz      @f
        add     ebx, 6

    @@:
        ; draw line with COLOR_LINE (in edx)
        mcall
        mov     ebx, (data_x_pos+data_x_size-10+4)*0x10000 + data_x_pos+data_x_size+2
        ; draw line with COLOR_LINE (in edx)
        mcall
        mov     al, 4
        mov     ebx, title_x_pos*10000h+title_y_pos
        ; setting up text color scheme and attributes
        mov     ecx, COLOR_TXT_NORMAL
        mov     edx, NoPrgLoaded_str
        cmp     [debuggee_pid], 0
        jz      @f
        mov     edx, [prgname_ptr]

    @@:
        ; draw text string in the window
        mcall
        cmp     [debuggee_pid], 0
        jz      .nodebuggee
        mov     ebx, (data_x_pos+data_x_size-10-6*7)*10000h + title_y_pos
        mov     edx, aRunning
        push    7
        pop     esi
        cmp     [bSuspended], 0
        jz      @f
        add     ebx, 6*10000h
        mov     edx, aPaused
        dec     esi

    @@:
        ; draw line with COLOR_LINE (in edx) in one case
        ; and draw text string with color COLOR_TXT_NORMAL (in ecx) in another
        mcall
        ret

    .nodebuggee:
        mov     al, 38
        mov     ebx, (data_x_pos+data_x_size-10-6*7-5)*0x10000 + data_x_pos+data_x_size+2
        mov     ecx, (title_y_pos+5)*10001h
        mov     edx, COLOR_LINE
        jmp     @b

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;; REGISTERS PANEL ;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;-----------------------------------------------------------------------------
;                      Display common register content

; TODO: add format support (e.g. numerical value, or address offset/pointer)

; in: esi->value, edx->string, ecx = string length, ebx = coord
draw_register:
        push    edx
        push    ecx
        push    esi
        mov     eax, esi
        mov     esi, ecx

        mov     ecx, (COLOR_TXT_INACTIVE or 0x40000000)
        cmp     [debuggee_pid], 0
        jz      .cd
        cmp     [bSuspended], 0
        jz      .cd

        mov     ecx, (COLOR_TXT_NORMAL or 0x40000000)
        push    edi
        mov     edi, [eax]
        cmp     dword [eax+oldcontext-context], edi
        pop     edi
        jz      .cd
        mov     ecx, (COLOR_TXT_CHANGED or 0x40000000)

    .cd:
        ; draw a text string in the window
        mcall   4
        imul    esi, 60000h
        lea     edx, [ebx+esi]
        mov     esi, ecx
        pop     ecx

        ; draw a number in the window
        mcall   47, 80101h
        lea     ebx, [edx+60000h*18]
        mov     esi, ecx
        pop     ecx
        pop     edx
        add     edx, ecx
        ret

;-----------------------------------------------------------------------------
;                  Display FPU register (ST0 - ST7) content
;
; in: esi->value, edx->string, ecx = string length, ebx = coord
draw_fpu_register:
        push    ebx
        push    edx
        push    ecx
        push    esi

        sub     esp, 8

        fld     tword [esi]
        fistp   qword [esp]

        mov     eax, esi
        mov     esi, ecx

        mov     ecx, (COLOR_TXT_INACTIVE or 0x40000000)
        cmp     [debuggee_pid], 0
        jz      .cd
        cmp     [bSuspended], 0
        jz      .cd

        mov     ecx, (COLOR_TXT_NORMAL or 0x40000000)
        push    edi
        mov     edi, [eax]
        cmp     dword [eax+oldcontext-context], edi
        pop     edi
        jnz     .scol
        push    edi
        mov     edi, [eax+4]
        cmp     dword [eax+oldcontext-context+4], edi
        pop     edi
        jz      .cd

    .scol:
        mov     ecx, (COLOR_TXT_CHANGED or 0x40000000)

    .cd:
        ; draw a text string in the window
        mcall   4
        imul    esi, 60000h
        lea     edx, [ebx+esi]
        mov     esi, ecx

        mov     ecx, esp

        ; draw a number in the window
        ; color is the same as for previous text draw function
        ; ebx : [20] show 16 chars set [30] bit - qword
        mcall   47, 40100101h
        add     esp, 8
        pop     ecx
        lea     ebx, [edx+60000h*18]
        mov     esi, ecx
        pop     ecx
        pop     edx
        pop     ebx
        add     edx, ecx
        ret

;-----------------------------------------------------------------------------
;                  Display FPU register (ST0 - ST7) content
;
; in: esi->value, ebx = coord
draw_fpu_register_2:

.str_buf  equ esp
.bcd_man  equ esp+32
.bcd_exp  equ esp+32+12
.exp_v    equ esp+32+12+12

        sub     esp, 32+12+12+4

        mov     eax, 0x20202020
        mov     edi, .str_buf
        stosd
        stosd
        stosd
        stosd

        mov     edx, ebp
        shl     edx, 4

        movzx   eax, word [_fsw]
        shr     eax, 11
        add     eax, ebp
        and     eax, 7

        bt      dword [_ftw], eax
        jc     .A6M

        mov     dword [.str_buf],' emp'
        mov     word [.str_buf+4],'ty'
        jmp     .display

        mov     cx, [_st0+edx+8]
        and     cx, 0x7FFF              ;clear sign flag
        jz      .A6M

        cmp     cx, 0x7FFF
        jne     .decode

        mov     dword [.str_buf], ' inv'
        mov     dword [.str_buf+4], 'alid'
        jmp     .display

.A6M:

        mov     eax, dword [_st0+edx]
        or      eax, dword [_st0+edx+4]
        jnz     .decode

        mov     dword [.str_buf], ' 0.0'
        jmp     .display

.decode:

        fld     tword [_st0+edx]
        fldlg2
        fld     tword [_st0+edx]
        bt      dword [_st0+edx+8], 15  ;check sign flag
        jnc @f
        fabs
@@:
        fyl2x
        frndint
        fist    dword [.exp_v]
        fld     st0
        fbstp   tword [.bcd_exp]
        fldl2t
        fmulp
        fld     st0
        frndint
        fxch
        fsub    st,st1

        f2xm1
        fld1
        faddp
        fscale
        fstp    st1
        fdivp
        fimul   dword [_10000000]
        fbstp   tword [.bcd_man]

        lea     esi, [.bcd_man-1]
        mov     edi, .str_buf

        mov     ecx, 9
        mov     eax, 0x10000

        mov     al, [esi+ecx+1]
        cmp     al, 0x80            ; check for sign
        jne     .mantis_2_str
        mov     al, '-'
        stosb

.mantis_2_str:

        mov     al, [esi+ecx]
        test    al, al
        jnz     @f

        bt      eax, 16
        jc      .skip_lb
@@:
        mov     ah, al
        shr     al, 4
        jnz     .write_h

        bt      eax, 16
        jc      .skip_hb

.write_h:
        add     al, 0x30
        stosb
        btr     eax, 16
        jnc     .skip_hb
        mov     al, '.'
        stosb

.skip_hb:
        mov     al, ah
        and     al, 0x0F
        jnz     .write_lb

        bt      eax, 16
        jc      .skip_lb

.write_lb:
        add     al,0x30
        stosb
        btr     eax, 16
        jnc     .skip_lb
        mov     al, '.'
        stosb

.skip_lb:
        loop    .mantis_2_str

        mov     eax, [.exp_v]
        test    eax, eax
        jz      .display

        mov     ax, ' e'
        stosw

        lea     esi, [.bcd_exp-1]
        mov     ecx, 9
        mov     eax,0x10000
        mov     al, [esi+ecx+1]
        cmp     al, 0x80
        jne     .exp_2_str
        mov     al, '-'
        stosb

.exp_2_str:
        mov     al, [esi+ecx]
        test    al, al
        jnz     @f

        bt      eax, 16
        jc      .skip_lb2
@@:
        mov     ah, al
        shr     al, 4
        jnz     .write_h2

        bt      eax, 16
        jc      .skip_hb2

.write_h2:
        add     al, 0x30
        stosb
        btr     eax, 16
        stosb

.skip_hb2:

        mov     al, ah
        and     al, 0x0F
        jnz     .write_lb2

        bt      eax, 16
        jc      .skip_lb2

.write_lb2:

        add     al, 0x30
        stosb
        btr     eax, 16

.skip_lb2:
        loop    .exp_2_str

.display:

        mov     ecx, (COLOR_TXT_INACTIVE or 0x40000000)
        cmp     [debuggee_pid], 0
        jz      .do_label
        cmp     [bSuspended], 0
        jz      .do_label

        mov     ecx, (COLOR_TXT_NORMAL or 0x40000000)

        mov     eax, dword [_st0+edx]
        cmp     eax, dword [_st0+(oldcontext-context)+edx]
        jne     .scol

        mov     eax, dword [_st0+edx+4]
        cmp     eax, dword [_st0+(oldcontext-context)+edx+4]
        jne     .scol

        mov     ax, word [_st0+edx+8]
        cmp     ax, word [_st0+(oldcontext-context)+edx+8]
        je      .do_label

.scol:
        mov     ecx, (COLOR_TXT_CHANGED or 0x40000000)

.do_label:
        ; draw a text string in the window

        mov     eax, 4
        mov     esi, eax
        lea     edx, [fpu_strs+ebp*4]
        mov     edi, COLOR_BG_NORMAL
        int     0x40

        mov     esi, 16
        mov     edx, .str_buf
        add     ebx, 0x180000
        int     0x40

        sub     ebx, 0x180000
        add     esp, 32+12+12+4

        ret


;-----------------------------------------------------------------------------
;                      Show FPU MMX register content
;
; in: esi->value, edx->string, ecx = string length, ebx = coord
draw_mmx_register:
        push    ebx
        push    edx
        push    ecx
        push    esi
        mov     eax, esi
        mov     esi, ecx

        mov     ecx, (COLOR_TXT_INACTIVE or 0x40000000)
        cmp     [debuggee_pid], 0
        jz      .cd
        cmp     [bSuspended], 0
        jz      .cd

        mov     ecx, (COLOR_TXT_NORMAL or 0x40000000)
        push    edi
        mov     edi, [eax]
        cmp     dword [eax+oldcontext-context], edi
        pop     edi
        jnz     .scol
        push    edi
        mov     edi, [eax+4]
        cmp     dword [eax+oldcontext-context+4], edi
        pop     edi
        jz      .cd

    .scol:
        mov     ecx, (COLOR_TXT_CHANGED or 0x40000000)

    .cd:
        ; draw a text string in the window
        mcall   4

        imul    esi, 60000h
        lea     edx, [ebx+esi]
        mov     esi, ecx
        pop     ecx
        ; draw a number in the window
        ; color is the same as for previous draw text function
        ; ebx : [20] show 16 chars set [30] bit - qword
        mcall   47, 40100101h
        lea     ebx, [edx+60000h*18]
        mov     esi, ecx
        pop     ecx
        pop     edx
        pop     ebx
        add     edx, ecx
        ret

; TODO add SSE registers
; TODO add AVX registers

;-----------------------------------------------------------------------------
;                   Display contents of EFLAGS register
draw_flag:
        movzx   edi, byte [edx+7]
        bt      [_eflags], edi
        jc      .on
        or      byte [edx], 20h
        jmp     .onoff

    .on:
        and     byte [edx], not 20h

    .onoff:
        mov     ecx, (COLOR_TXT_INACTIVE or 0x40000000)
        cmp     [debuggee_pid], 0
        jz      .doit
        cmp     [bSuspended], 0
        jz      .doit

        mov     ecx, (COLOR_TXT_NORMAL or 0x40000000)
        bt      [_eflags], edi
        lahf
        bt      dword [_eflags + oldcontext - context], edi
        rcl     ah, 1
        test    ah, 3
        jp      .doit
        mov     ecx, (COLOR_TXT_CHANGED or 0x40000000)

    .doit:
        mov     ah, 0
        mov     edi, COLOR_BG_NORMAL
        ; draw a text string in the window in one case
        ; and a number in another
        ; color scheme same as for previously called function (was in ecx)
        mcall
        ret

;-----------------------------------------------------------------------------
;                      Draw registers frame title

; Also show current register set (common + MMX, SSE or AVX)
draw_reg_title:
        mov     edi, COLOR_BG_NORMAL
        mov     ecx, (COLOR_TXT_NORMAL or 0x40000000)
        mov     esi, 7
        cmp     [reg_mode], REG_MODE_CPU
        jz      @f
        mov     ecx, (COLOR_TXT_INACTIVE or 0x40000000)
    @@:
        mov     edx, aMain
        ; draw a text string in the window
        mcall   4, (registers_x_pos+4)*10000h+registers_y_pos+2

        cmp     [reg_mode], REG_MODE_SSE
        jz      @f
        mov     ecx, (COLOR_TXT_INACTIVE or 0x40000000)
    @@:
        mov     edx, aSSE
        ; draw a text string in the window
        mcall   4, (registers_x_pos+46)*10000h+registers_y_pos+2

        cmp     [reg_mode], REG_MODE_AVX
        jz      @f
        mov     ecx, (COLOR_TXT_INACTIVE or 0x40000000)
    @@:
        mov     edx, aAVX
        ; draw a text string in the window
        mcall   4, (registers_x_pos+88)*10000h+registers_y_pos+2
        ret

;-----------------------------------------------------------------------------
;                Display common registers set + MMX + FPU

draw_main_registers:
; TODO: add support for FPU ST0-ST7 registers
        mov     edi, COLOR_BG_NORMAL
        mov     esi, _eax
        push    4
        pop     ecx
        mov     edx, regs_strs
        mov     ebx, (registers_x_pos+2)*10000h+registers_y_pos+22
        call    draw_register
        mov     ebx, (registers_x_pos+2)*10000h+registers_y_pos+32
        add     esi, _ebx-_eax
        call    draw_register
        mov     ebx, (registers_x_pos+2)*10000h+registers_y_pos+42
        add     esi, _ecx-_ebx
        call    draw_register
        mov     ebx, (registers_x_pos+2)*10000h+registers_y_pos+52
        add     esi, _edx-_ecx
        call    draw_register
        mov     ebx, (registers_x_pos+2)*10000h+registers_y_pos+62
        add     esi, _esi-_edx
        call    draw_register
        mov     ebx, (registers_x_pos+2)*10000h+registers_y_pos+72
        add     esi, _edi-_esi
        call    draw_register
        mov     ebx, (registers_x_pos+2)*10000h+registers_y_pos+82
        add     esi, _ebp-_edi
        call    draw_register
        mov     ebx, (registers_x_pos+2)*10000h+registers_y_pos+92
        add     esi, _esp-_ebp
        call    draw_register
        mov     ebx, (registers_x_pos+2)*10000h+registers_y_pos+102
        add     esi, _eip-_esp
        call    draw_register
        mov     ebx, (registers_x_pos+2)*10000h+registers_y_pos+112

        mov     cl, 7
        add     esi, _eflags-_eip
        call    draw_register
        mov     cl, 4

    ; MMX registers

        push    ebp

        push    8
        mov     edx, mmx_strs
        mov     ebx, (registers_x_pos+2)*10000h+registers_y_pos+142
        mov     esi, _mm0

align 4
.draw_mmx_regs:

        call    draw_mmx_register
        add     ebx, 10
        add     esi, 16
        dec     dword [esp]
        jnz     .draw_mmx_regs

;FPU registers

        ;int3
        nop

        mov     [esp], byte 8
        xor     ebp, ebp
        mov     ebx, (registers_x_pos+2)*10000h+registers_y_pos+232

align 4
.draw_fpu_regs:

        call    draw_fpu_register_2
        add     ebx, 10
        inc     ebp
        dec     dword [esp]
        jnz     .draw_fpu_regs
        pop     eax                         ;restore stack
        pop     ebp

        mov     ecx, COLOR_TXT_INACTIVE
        cmp     [debuggee_pid], 0
        jz      @f
        cmp     [bSuspended], 0
        jz      @f
        mov     ecx, COLOR_TXT_NORMAL
    @@:
        mov     edx, aColon
        xor     esi, esi
        inc     esi
        mcall   4, (registers_x_pos+10)*10000h+registers_y_pos+122
        mov     edx, flags

    @@:
        add     ebx, 2*6*10000h
        call    draw_flag
        inc     edx
        cmp     dl, flags_bits and 0xFF
        jnz     @b
        ret

;-----------------------------------------------------------------------------
;                  Draw SSE registers set

draw_sse_registers:

        ret

;-----------------------------------------------------------------------------
;                  Draw AVX registers set

draw_avx_registers:

        ret

;-----------------------------------------------------------------------------
;                 Draw all registers sets
draw_registers:

        ; draw container rectangle/box with COLOR_BG_NORMAL
        mcall   13, (registers_x_pos-1)*10000h+(registers_x_size+2), (registers_y_pos-1)*10000h+(registers_y_size+2), COLOR_BG_NORMAL
        call    draw_reg_title

    .redraw:
        cmp     [reg_mode], REG_MODE_CPU
        jnz     @f
        call    draw_main_registers
        ret

    @@:
        cmp     [reg_mode], REG_MODE_SSE
        jnz     @f
        call    draw_sse_registers
        ret

    @@:
        call    draw_avx_registers
        ret

;-----------------------------------------------------------------------------
;                     Display memory dump

draw_dump:
        ; draw container rectangle/box in the window
        mcall   13, data_x_pos*10000h+data_x_size, dump_y_pos*10000h+dump_y_size, COLOR_BG_NORMAL

    .redraw:
        ; addresses
        mov     ebx, 80100h
        mov     edx, data_x_pos*10000h + dump_y_pos
        mov     ecx, [dumppos]
        mov     edi, COLOR_BG_NORMAL
        mov     esi, (COLOR_TXT_INACTIVE or 0x40000000)
        cmp     [debuggee_pid], 0
        jz      @f
        cmp     [bSuspended], 0
        jz      @f
        mov     esi, (COLOR_TXT_NORMAL or 0x40000000)
    @@:
        ; draw a number in the window
        mcall   47
        add     ecx, 10h
        add     edx, 10
        cmp     dl, dump_y_pos + dump_y_size
        jb      @b
        ; hex dump of data
        mov     ecx, dumpdata
        push    ecx
        xor     ebx, ebx
        mov     edx, (data_x_pos+12*6)*10000h + dump_y_pos
        cmp     [dumpread], ebx
        jz      .hexdumpdone1

    .hexdumploop1:
        push    ebx
        mov     ebx, 20101h
        ; draw a number in the window
        mcall
        pop     ebx
        add     edx, 3*6*10000h
        inc     ecx
        inc     ebx
        test    bl, 15
        jz      .16
        test    bl, 7
        jnz     @f
        add     edx, 2*6*10000h - 10 + 6*(3*10h+2)*10000h

    .16:
        add     edx, 10 - 6*(3*10h+2)*10000h

    @@:
        cmp     ebx, [dumpread]
        jb      .hexdumploop1

    .hexdumpdone1:
        mov     al, 4
        ; copy color value from esi to ecx
        ; to draw text string with 'mcall 4'
        mov     ecx, esi
        xchg    ebx, edx
        push    2
        pop     esi

    .hexdumploop2:
        cmp     edx, dump_height*10h
        jae     .hexdumpdone2
        push    edx
        mov     edx, aQuests
        ; draw text string with color in ecx, copied from esi
        mcall
        pop     edx
        add     ebx, 3*6*10000h
        inc     edx
        test    dl, 15
        jz      .16x
        test    dl, 7
        jnz     .hexdumploop2
        add     ebx, 2*6*10000h - 10 + 6*(3*10h+2)*10000h

    .16x:
        add     ebx, 10 - 6*(3*10h+2)*10000h
        jmp     .hexdumploop2

    .hexdumpdone2:
        dec     esi
        ; colon, minus signs
        mov     ebx, (data_x_pos+8*6)*10000h + dump_y_pos
        mov     edx, aColon

    @@:
        mcall
        add     ebx, 10
        cmp     bl, dump_y_pos+dump_height*10
        jb      @b
        mov     ebx, (data_x_pos+(12+3*8)*6)*10000h + dump_y_pos
        mov     edx, aMinus

    @@:
        mcall
        add     ebx, 10
        cmp     bl, dump_y_pos+dump_height*10
        jb      @b
        ; ASCII data
        mov     ebx, (data_x_pos+(12+3*10h+2+2)*6)*10000h + dump_y_pos
        pop     edx
        push    dump_height*10h

    .asciiloop:
        push    edx
        cmp     byte [edx], 20h
        jae     @f
        mov     edx, aPoint

    @@:
        ; draw a text string in the window, color in ecx
        mcall
        pop     edx
        inc     edx
        add     ebx, 6*10000h
        dec     dword [esp]
        jz      .asciidone
        test    byte [esp], 15
        jnz     .asciiloop
        add     ebx, 10 - 6*10h*10000h
        jmp     .asciiloop

    .asciidone:
        pop     ecx
        ret

;-----------------------------------------------------------------------------
;                   Display disassembled code

draw_disasm:

        mov     eax, [disasm_start_pos]
        mov     [disasm_cur_pos], eax
        and     [disasm_cur_str], 0

    .loop:
        mov     eax, [disasm_cur_pos]
        call    find_symbol
        jc      .nosymb
        mov     ebx, [disasm_cur_str]
        imul    ebx, 10
        push    ebx
        lea     ecx, [ebx+disasm_y_pos-1]
        shl     ecx, 16
        mov     cl, 11
        ; setting up background color for disassembled text
        mov     edx, COLOR_BG_NORMAL
        ; draw container rectangle/box with color COLOR_BG_NORMAL (was 0xFFFFFF - white)
        mcall   13, data_x_pos*10000h+data_x_size
        pop     ebx
        ; copy color value from edx (COLOR_BG_NORMAL)
        mov     edi, edx
        add     ebx, (data_x_pos+6*2)*10000h+disasm_y_pos
        mov     edx, esi

    @@:
        lodsb
        test    al, al
        jnz     @b
        mov     byte [esi-1], ':'
        sub     esi, edx
        ; normal color
        ; was 0x40000000
        mov     ecx, (COLOR_TXT_LABEL or 0x40000000)
        mov     al, 4
        ; draw a text string in the window with color COLOR_TXT_NORMAL in ecx
        mcall
        mov     byte [esi+edx-1], 0
        lea     esi, [esi*3]
        movzx   ecx, bx
        shr     ebx, 16
        lea     ebx, [ebx+esi*2]
        shl     ecx, 16
        mov     cl, 10
        imul    ebx, 10001h
        sub     bx, data_x_pos+data_x_size
        neg     bx
        mov     al, 13
        ; copy color value from edi
        mov     edx, edi
        ; draw container rectangle/box for disassembled text, color in edx
        mcall
        inc     [disasm_cur_str]
        cmp     [disasm_cur_str], disasm_height
        jae     .loopend

    .nosymb:
        push    [disasm_cur_pos]
        call    disasm_instr
        pop     ebp
        jc      .loopend
        mov     edx, COLOR_BG_NORMAL
        mov     esi, COLOR_TXT_NORMAL
        mov     ebx, data_x_pos*10000h + data_x_size
        mov     ecx, [disasm_cur_str]
        imul    ecx, 10*10000h
        add     ecx, (disasm_y_pos-1)*10000h + 10
        mov     eax, ebp
        pushad
        call    find_enabled_breakpoint
        popad
        jnz     .nobp
        mov     edx, COLOR_BG_BREAKPOINT
        mov     esi, COLOR_TXT_BREAKPOINT
    .nobp:

        mov     eax, [_eip]
        cmp     eax, ebp
        jnz     .notcurrent
        mov     edx, COLOR_BG_SELECTED
        mov     esi, COLOR_TXT_SELECTED
    .notcurrent:
        push    esi     ; Save color value for disassembled text

        ; draw container rectangle/box for disassembled text
        ; color in edx
        mcall   13

        mov     edx, [disasm_cur_str]
        imul    edx, 10
        add     edx, data_x_pos*10000h + disasm_y_pos
        ; draw a number in the window, color in esi
        mcall   47, 80100h, ebp

        lea     ebx, [edx+8*6*10000h]
        mov     ecx, esi    ; text color
        push    2
        pop     esi
        mov     edx, aColon
        ; draw the colon
        mcall   4
        push    9
        pop     edi
        lea     edx, [ebx+2*6*10000h]
        mov     ecx, ebp
        sub     ecx, [disasm_start_pos]
        add     ecx, disasm_buffer

        mov     esi, COLOR_TXT_HEX
        mov     eax, [_eip]
        cmp     eax, ebp
        jnz     @f
        mov     esi, COLOR_TXT_SELECTED
  @@:
    .drawhex:
        ; draw a number in the window, color in esi
        mcall   47, 20101h
        add     edx, 6*3*10000h
        inc     ecx
        inc     ebp
        cmp     ebp, [disasm_cur_pos]
        jae     .hexdone
        dec     edi
        jnz     .drawhex
        push    esi
        mov     esi, [disasm_cur_pos]
        dec     esi
        cmp     esi, ebp
        pop     esi
        jbe     .drawhex

        lea     ebx, [edx-6*10000h]
        ; copy color value from esi
        mov     ecx, esi
        push    3
        pop     esi
        mov     edx, aDots
        ; draw a text string in the window, color in ecx
        mcall   4

    .hexdone:
        pop     esi
        xor     eax, eax
        mov     edi, disasm_string
        mov     edx, edi
        or      ecx, -1
        repnz scasb
        not     ecx
        dec     ecx
        xchg    ecx, esi
        mov     ebx, [disasm_cur_str]
        imul    ebx, 10
        add     ebx, (data_x_pos+6*40)*10000h+disasm_y_pos

        ; draw a text string in the window, color in ecx
        mcall   4
        inc     [disasm_cur_str]
        cmp     [disasm_cur_str], disasm_height
        jb      .loop

    .loopend:
        mov     ecx, disasm_height
        sub     ecx, [disasm_cur_str]
        jz      @f
        imul    ecx, 10
        inc     ecx
        mov     eax, disasm_y_pos + disasm_y_size
        sub     eax, ecx
        shl     eax, 16
        add     ecx, eax
        ; Draw filled rectangle
        mcall   13, data_x_pos*10000h+data_x_size, , COLOR_BG_NORMAL

    @@:
        ret

;-----------------------------------------------------------------------------

; TODO: cleanup of this function, make some global labels local
update_disasm_eip:
; test if instruction at eip is showed
        mov     ecx, disasm_height
        mov     eax, [disasm_start_pos]
        mov     [disasm_cur_pos], eax

    .l:
        mov     eax, [disasm_cur_pos]
        call    find_symbol
        jc      @f
        dec     ecx
        jz      .m

    @@:
        cmp     [_eip], eax
        jz      draw_disasm
        push    ecx
        call    disasm_instr
        pop     ecx
        jc      .m
        loop    .l

    .m:

update_disasm_eip_force:
        mov     eax, [_eip]
        mov     [disasm_start_pos], eax

update_disasm:
        cmp     [debuggee_pid], 0
        jz      .no

        mcall   69, 6, [debuggee_pid], 256, [disasm_start_pos], disasm_buffer
        cmp     eax, -1
        jnz     @f
        mov     esi, read_mem_err
        call    put_message

    .no:
        xor     eax, eax

    @@:
        mov     [disasm_buf_size], eax
        call    restore_from_breaks
        jmp     draw_disasm


;-----------------------------------------------------------------------------
;                               Draw main window

draw_window:
        ; start window redraw
        mcall   12, 1

        ; define window
        mcall   0, wnd_x_size, wnd_y_size, (COLOR_BG_NORMAL or 0x54000000), , caption_str

        ; clear unused areas
        ; get window skin height
        mcall   48, 4
        cmp     eax, title_y_pos
        jb      @f
        push    registers_y_pos
        pop     eax

    @@:
        push    registers_y_pos
        pop     ecx
        push    eax
        sub     ecx, eax
        shl     eax, 16
        add     ecx, eax
        mov     ebx, 5*10000h + (wnd_x_size-9)
        mov     edx, COLOR_BG_NORMAL
        ; draw container rectangle/box for registers information region
        mcall   13
        mov     ecx, (dump_y_pos+dump_y_size)*10000h + (disasm_y_pos-dump_y_pos-dump_y_size)
        ; draw container rectangle/box for dump memory region
        mcall
        mov     ecx, (disasm_y_pos-1+disasm_y_size)*10000h + (messages_y_pos-disasm_y_pos+1-disasm_y_size)
        ; draw container rectangle/box for disassembled code region
        mcall
        mov     ecx, (messages_y_pos+messages_y_size)*10000h + (wnd_y_size-messages_y_pos-messages_y_size-4)
        ; draw container rectangle/box for messages window region
        mcall
        mov     ebx, 5*10000h + (data_x_pos-5)
        pop     ecx
        imul    ecx, 10001h
        sub     cx, wnd_y_size-4
        neg     cx
        ; draw container rectangle/box
        mcall
        mov     ebx, (data_x_pos+data_x_size)*10000h + (wnd_x_size-data_x_pos-data_x_size-4)
        ; draw container rectangle/box
        mcall
        mov     ebx, 5*10000h + title_x_pos - 5
        mov     ecx, (title_y_pos)*10000h + (title_y_size)
        ; draw container rectangle/box for dump memory region title
        mcall

        ; messages frame
        mov     ebx, (messages_x_pos-2)*10000h + (messages_x_pos+messages_x_size+2)
        push    ebx
        mov     ecx, (messages_y_pos-2)*10001h
        mov     edx, COLOR_LINE
        mcall   38
        mov     ecx, (messages_y_pos+messages_y_size+2)*10001h
        mcall
        mov     ebx, (messages_x_pos-2)*10001h
        push    ebx
        mov     ecx, (messages_y_pos-2)*10000h + (messages_y_pos+messages_y_size+2)
        mcall
        mov     ebx, (messages_x_pos+messages_x_size+2)*10001h
        push    ebx
        mcall

        ; command line frame
        mov     ecx, (cmdline_y_pos-2)*10000h + (cmdline_y_pos+cmdline_y_size+2)
        pop     ebx
        mcall
        pop     ebx
        mcall
        pop     ebx
        mov     ecx, (cmdline_y_pos+cmdline_y_size+2)*10001h
        mcall
        mov     ecx, (cmdline_y_pos-2)*10001h
        mcall

        ; registers frame
        DrawRectangle (registers_x_pos-2), (registers_y_pos-2), (registers_x_size+3), (registers_y_size+3), COLOR_LINE
        ; draw container rectangle/box for registers information window region

        ; messages
        call    draw_messages

        ; command line & cursor
        call    draw_cmdline
        call    draw_cursor

        ; title & registers & dump & disasm
        mov     ebx, (data_x_pos-2)*10001h
        mov     ecx, (title_y_pos+5)*10000h + (messages_y_pos-2)
        mov     edx, COLOR_LINE
        mcall   38
        mov     ebx, (data_x_pos+data_x_size+2)*10001h
        mcall
        mov     ebx, (data_x_pos-2)*10000h + (data_x_pos+data_x_size+2)
        mov     ecx, (disasm_y_pos-4)*10001h
        mcall

        ; redraw whole window again
        call    redraw_title
        call    draw_registers
        call    draw_dump
        call    draw_disasm

        ; end of window redraw
        mcall   12, 2
        ret

; vim: ft=fasm tabstop=4

