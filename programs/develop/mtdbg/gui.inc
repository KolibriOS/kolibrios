;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; GUI ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;-----------------------------------------------------------------------------
;                             Color scheme

BLACK_ON_WHITE  = 0
MOVIEOS         = 1
WHITE_ON_BLACK  = 2

;-----------------------------------------------------------------------------
;                                Font

FONT            equ 1           ; 0 to 3
FONT_MULTIPLIER equ 1           ; 1 to 8

;-----------------------------------------------------------------------------
;                       GUI elements size and position

TEXT_Y_CLEARANCE        = 1                                                     ; in pixels

DATA_WIDTH              = 80                                                    ; in characters
DATA_X_POS              = 12                                                    ; in pixels

TITLE_X_POS             = 30                                                    ; in pixels
TITLE_Y_POS             = 8                                                     ; in pixels

DUMP_HEIGHT             = 6                                                     ; in text lines
MIN_DISASM_HEIGHT       = 18                                                    ; in text lines
MSG_HEIGHT              = 14                                                    ; in text lines

;-----------------------------------------------------------------------------

if (FONT eq 0)
        FONT_X_SIZE     = 6*FONT_MULTIPLIER
else
        FONT_X_SIZE     = 8*FONT_MULTIPLIER
end if

if (FONT eq 0)
        FONT_Y_SIZE     = 9*FONT_MULTIPLIER
else
        FONT_Y_SIZE     = 16*FONT_MULTIPLIER
end if

TITLE_Y_SIZE            = (FONT_Y_SIZE + TEXT_Y_CLEARANCE)
DUMP_Y_POS              = (TITLE_Y_POS + TITLE_Y_SIZE)
DUMP_Y_SIZE             = (DUMP_HEIGHT*(FONT_Y_SIZE + TEXT_Y_CLEARANCE))
DISASM_Y_POS            = (DUMP_Y_POS + DUMP_Y_SIZE + 4)

MSG_WIDTH               = DATA_WIDTH
MSG_X_POS               = DATA_X_POS
MSG_X_SIZE              = MSG_WIDTH*FONT_X_SIZE
MSG_Y_SIZE              = MSG_HEIGHT*(FONT_Y_SIZE + TEXT_Y_CLEARANCE)

CMD_WIDTH               = DATA_WIDTH
CMD_X_POS               = DATA_X_POS
CMD_Y_SIZE              = (FONT_Y_SIZE+TEXT_Y_CLEARANCE)

REGISTERS_Y_POS         = DUMP_Y_POS
REGISTERS_X_SIZE        = (22+2)*(FONT_X_SIZE)                                  ; 22 characters + 2 spacing

wnd_x_size              = (DATA_X_POS + MSG_X_SIZE + DATA_X_POS + REGISTERS_X_SIZE + 3) + 10
wnd_y_size              = (((DISASM_Y_POS + (MIN_DISASM_HEIGHT*(FONT_Y_SIZE + TEXT_Y_CLEARANCE)) + 4) + MSG_Y_SIZE + 4) + CMD_Y_SIZE + DATA_X_POS)

;-----------------------------------------------------------------------------

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

struc fpcvt
{
  .str_buf  rb 32
  .cvt_buf  rb 16
  .bcd_man  rb 10
  .bcd_exp  rb 10
  .exp      rd 1
  .digits   rq 1
  .sizeof:
}



;-----------------------------------------------------------------------------
;                          Entry point

; TODO: split all gui part in independent function, move entry point into mtdbg.asm

start:
        ; initialize process heap
        mcall   68, 11

        call	init_cmd_hist_buffer

        push    eax
        fstcw   word [esp]
        pop     eax
        and     ax, not (3 shl 10)
        or      ax, 1 shl 10    ; set round-to-minus infinity mode
        push    eax
        fldcw   word [esp]
        pop     eax

        mov     edi, messages
        mov     ecx, MSG_WIDTH*MSG_HEIGHT
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
        mcall   40, EVM_REDRAW or EVM_KEY or EVM_BUTTON or EVM_DEBUG
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
;button pressed
        mcall   17
        test    al, al
        jnz     waitevent
        movzx   eax, ah
        jmp     dword [.jmp_table+eax*4]

.close:
        mcall -1
.mmx:
        movzx   edx, [fpu_mode]
        xor     dl, 1
        mov     [fpu_mode], dl
        push    edx
        jnz     .fpu
        call    draw_mmx_regs
        jmp     .draw_label
.fpu:
        call    draw_fpu_regs
.draw_label:
        pop     edx
        mov     edx, [btn2_tab+edx*4]
        mov     edi, COLOR_BG_NORMAL
        mov     ecx, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_NORMAL or 0x40000000)
        mov     esi, 7
        mov     ebx, [registers_x_pos_dd]
        add     ebx, 4*10000h+REGISTERS_Y_POS
        mcall   4
        jmp     waitevent

align 4
.jmp_table:
        dd waitevent
        dd .close
        dd .mmx


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
        mov	[cmd_hist_buffer.tmp_line_flag], 0
        cmp     [cmdline_len], CMD_WIDTH
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
	mov	[cmd_hist_buffer.tmp_line_flag], 0
        cmp     [cmdline_pos], 0
        jz      waitevent
        dec     [cmdline_pos]

    .delchar:
	mov	[cmd_hist_buffer.tmp_line_flag], 0
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
		xor	edx, edx
		jmp	.hist

	.down:
		xor	edx, edx
		inc	edx

	.hist:
		cmp	[cmd_hist_buffer.tmp_line_flag], 1
		je	@f
		mov	eax, cmdline
		mov	ecx, [cmdline_len]
		mov	byte [eax + ecx], 0
		call	add_cmd_hist_tmp_line

	@@:
		test	edx, edx
		jnz	.hist.next
		cmp	[cmd_hist_buffer.n_flag], 0
		jne	@f
		call	get_cmd_hist_line
		inc	[cmd_hist_buffer.n_flag]
		jmp	.hist.cp_line

	@@:
		call	prev_cmd_hist_line
		jc	waitevent
		call	get_cmd_hist_line
		inc	[cmd_hist_buffer.n_flag]
		jmp	.hist.cp_line

	.hist.next:
		cmp	[cmd_hist_buffer.n_flag], 0
		je	waitevent

		call	next_cmd_hist_line
		call	get_cmd_hist_line
		jnc	.hist.cp_line
		call	get_cmd_hist_tmp_line
		mov	[cmd_hist_buffer.tmp_line_flag], 0
		mov	[cmd_hist_buffer.n_flag], 0

	.hist.cp_line:

		mov	esi, eax
		mov	edi, cmdline
		xor	ecx, ecx

	@@:
		inc	ecx
		lodsb
		stosb
		test	al, al
		jnz	@b

		dec	ecx
		mov	[cmdline_pos], 0
		call    clear_cmdline_end
		mov	[cmdline_len], ecx
		mov	[cmdline_pos], ecx
		call	draw_cmdline
		call    draw_cursor

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

        mov	eax, esi
        call	add_cmd_hist_line
        mov	[cmd_hist_buffer.n_flag], 0

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
	pusha

        mov     ebx, [cmdline_pos]
        mov     ecx, [cmdline_len]
        sub     ecx, ebx
        imul    ebx, FONT_X_SIZE
        imul    ecx, FONT_X_SIZE
        inc     ecx
        add     ebx, CMD_X_POS
        shl     ebx, 16
        or      ebx, ecx
        mov     ecx, [cmdline_y_pos_dd]
        mov     cx, CMD_Y_SIZE
        mov     edx, COLOR_BG_NORMAL
        ; draw container rectangle/box for cmdline
        mcall   13

        popa
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

        mov     ecx, (FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_NORMAL
        lea     edx, [cmdline+ebx]
        imul    ebx, FONT_X_SIZE
        add     ebx, CMD_X_POS
        shl     ebx, 16
        mov     bx, word[cmdline_y_pos_dd+4]
        inc     bx
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
        mov     ecx, MSG_WIDTH
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
        cmp     edx, MSG_WIDTH*MSG_HEIGHT
        jnz     .ret
        push    esi
        mov     edi, messages
        lea     esi, [edi+MSG_WIDTH]
        mov     ecx, (MSG_HEIGHT-1)*MSG_WIDTH/4
        rep movsd
        push    eax
        mov     al, ' '
        push    edi
        push    MSG_WIDTH
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
        mov     ebx, MSG_X_POS shl 16
        add     ebx, [messages_x_size_dd+4]
        mov     ecx, [messages_y_pos_dd]
        mov     cx, MSG_Y_SIZE
        mcall   13, , , COLOR_BG_NORMAL
        mov     edx, messages
        push    MSG_WIDTH
        pop     esi
        mov     ecx, (FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_NORMAL
        mov     ebx, MSG_X_POS*10000h
        mov     bx, word[messages_y_pos_dd+4]

    @@:
        ; display text string in the window
        mcall   4
        add     edx, esi
        add     ebx, FONT_Y_SIZE+TEXT_Y_CLEARANCE
        cmp     edx, messages+MSG_WIDTH*MSG_HEIGHT
        jb      @b
        ret

;-----------------------------------------------------------------------------
;                     Show/hide cursor in command line

; TODO: make it cursor.draw and cursor.hide ???
draw_cursor:
	pusha

        mov     ecx, [cmdline_y_pos_dd+2]
        add     cx, CMD_Y_SIZE-1
        mov     ebx, [cmdline_pos]
        imul    ebx, FONT_X_SIZE
        add     ebx, CMD_X_POS
        mov     edx, ebx
        shl     ebx, 16
        or      ebx, edx
        mov     edx, (FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_NORMAL
        ; draw line
        mcall   38

        popa
        ret

hide_cursor:
        mov     ebx, [cmdline_pos]
        imul    ebx, FONT_X_SIZE
        add     ebx, CMD_X_POS
        shl     ebx, 16
        inc     ebx
        mov     ecx, [cmdline_y_pos_dd]
        mov     cx, CMD_Y_SIZE
        mov     edx, COLOR_BG_NORMAL
        ; draw container rectangle/box
        mcall   13
        mov     ebx, [cmdline_pos]
        cmp     ebx, [cmdline_len]
        jae     .ret
        ; setting up text color scheme and attributes
        mov     ecx, (FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_NORMAL
        lea     edx, [cmdline+ebx]
        imul    ebx, FONT_X_SIZE
        add     ebx, CMD_X_POS
        shl     ebx, 16
        mov     bx, word[cmdline_y_pos_dd+4]
        inc     bx
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
        mov     ebx, [data_x_size_dd+4]
        add     ebx, TITLE_X_POS*10000h+DATA_X_POS-TITLE_X_POS
        mcall   13, , TITLE_Y_POS*10000h+TITLE_Y_SIZE, COLOR_BG_NORMAL

draw_title:
        mcall   38, (DATA_X_POS-2)*10000h+TITLE_X_POS-5, (TITLE_Y_POS+(FONT_Y_SIZE/2))*10001h, COLOR_LINE
        push    NoPrgLoaded_len
        pop     esi
        cmp     [debuggee_pid], 0
        jz      @f
        mov     esi, [prgname_len]

    @@:
        imul    ebx, esi, FONT_X_SIZE
        add     ebx, TITLE_X_POS+4
        shl     ebx, 16
        mov     bx, DATA_X_POS-10-5-FONT_X_SIZE*7
        add     bx, word[data_x_size_dd+4]
        cmp     [bSuspended], 0
        jz      @f
        add     ebx, FONT_X_SIZE

    @@:
        ; draw line with COLOR_LINE (in edx)
        mcall
        mov     ebx, [data_x_size_dd+2]
        add     ebx, (DATA_X_POS-10+4)*0x10000 + DATA_X_POS+2
        ; draw line with COLOR_LINE (in edx)
        mcall
        mov     al, 4
        mov     ebx, TITLE_X_POS*10000h+TITLE_Y_POS
        ; setting up text color scheme and attributes
        mov     ecx, (FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_NORMAL
        mov     edx, NoPrgLoaded_str
        cmp     [debuggee_pid], 0
        jz      @f
        mov     edx, [prgname_ptr]

    @@:
        ; draw text string in the window
        mcall
        cmp     [debuggee_pid], 0
        jz      .nodebuggee
        mov     ebx, [data_x_size_dd]
        add     ebx, (DATA_X_POS-10-FONT_X_SIZE*7)*10000h + TITLE_Y_POS
        mov     edx, aRunning
        push    7
        pop     esi
        cmp     [bSuspended], 0
        jz      @f
        add     ebx, FONT_X_SIZE*10000h
        mov     edx, aPaused
        dec     esi

    @@:
        ; draw line with COLOR_LINE (in edx) in one case
        ; and draw text string with color (FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_NORMAL (in ecx) in another
        mcall
        ret

    .nodebuggee:
        mov     al, 38
        mov     ebx, [data_x_size_dd+2]
        add     ebx, (DATA_X_POS-10-FONT_X_SIZE*7-5)*0x10000 + DATA_X_POS+2
        mov     ecx, (TITLE_Y_POS+(FONT_Y_SIZE/2))*10001h
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
        push    esi
        push    edx
        push    ecx
        push    ebp

        mov     ebp, ecx

        mov     eax, [esi]

        mov     ecx, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_INACTIVE or 0x40000000)
        cmp     [debuggee_pid], 0
        jz      .cd
        cmp     [bSuspended], 0
        jz      .cd

        mov     ecx, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_NORMAL or 0x40000000)
        cmp     eax, dword [esi+oldcontext-context]
        je      .cd
        mov     ecx, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_CHANGED or 0x40000000)
.cd:
        push    eax                       ;store reg value

        mov     esi, ebp
        ; draw a text string in the window
        mcall   4

        imul    esi, FONT_X_SIZE*10000h
        lea     edx, [ebx+esi]
        mov     esi, ecx
        pop     ecx

        ; draw a number in the window
        rol     ecx, 16
        mcall   47, 0x00040100

        shr     ecx, 16
        add     edx, (4*FONT_X_SIZE+3) shl 16
        mcall   47

        pop     ebp
        pop     ecx
        pop     edx
        pop     esi
        add     edx, ecx
        ret


; in: edi=cvt buffer, st0 = value

;align 4
;fp2str:



;-----------------------------------------------------------------------------
;                  Display FPU register (ST0 - ST7) content
;
; in: ebp->index, ebx = coord

draw_fpu_register_2:

.str_buf  equ esp
.cvt_buf  equ .str_buf+32
.bcd_man  equ .cvt_buf+16
.bcd_exp  equ .bcd_man+10
.exp      equ .bcd_exp+10
.tmp      equ .exp+4
.lcl_end  equ .tmp+4

        sub     esp, 32+16+10+10+4+4

        mov     edi, .str_buf
        shl     ebp, 16
        lea     eax, ['ST0:'+ebp]
        stosd
        mov     eax, 0x20202020
        stosd
        stosd
        stosd
        stosd
        stosd

        movzx   eax, word [_fsw]
        shr     eax, 11
        add     eax, ebp
        shr     ebp, 12
        and     eax, 7
        bt      dword [_ftw], eax
        jc     .exam

        mov     dword [.str_buf+8],' emp'
        mov     word [.str_buf+8+4],'ty'
        jmp     .display

.exam:
        fld     tword [_st0+ebp]
        fxam
        fstsw   ax
        fstp    st1
        sahf

        jz      .c3
        jp      .c2

;C0 leaf
        jc      .nan

; C3 = 0 C2 = 0 C0 = 0 - invalid
        mov     dword [.str_buf+6], ' inv'
        mov     dword [.str_buf+6+4], 'alid'
        jmp     .display

; C3 = 0 C2 = 0 C0 = 1 - Not a Number
.nan:
        mov     dword [.str_buf+10], ' NaN'
        jmp     .display

;C2 leaf - valid or infinity

.c2:
; C3 = 0 C2 = 1 C0 = 0 - valid number

        jnc     .decode

; C3 = 0 C2 = 1 C0 = 1 - Infinty

;check sign flag
        test    ah, 0x02
        jnz     @F
        mov     dword [.str_buf+10], '+Inf'
        jmp     .display
@@:
        mov     dword [.str_buf+10], '-Inf'
        jmp     .display

.c3:
        jp      .denormal
        jc      .empty

; C3 = 1 C2 = 1 C0 = 0 - Zero

        mov     dword [.str_buf+10], ' 0.0'
        jmp     .display

.empty:
        mov     dword [.str_buf+8],' emp'
        mov     word [.str_buf+8+4],'ty'
        jmp     .display

; C3 = 1 C2 = 1 C0 = 0 - Denormal number

.denormal:
        test    ah, 0x02
        jnz     @F

        mov     dword [.str_buf+6], '+den'
        mov     dword [.str_buf+6+4], 'orm '
        jmp     .display
@@:
        mov     dword [.str_buf+6], '-den'
        mov     dword [.str_buf+6+4], 'orm '
        jmp     .display

.decode:
        fld     tword [_st0+ebp]
        fabs
        fld     st0
        fldlg2
        fld     st1
        fyl2x
        frndint
        fist    dword [.exp]
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

        fist    dword [.tmp]
        cmp     dword [.tmp], 10
        jae     .fixup

        fstp    st1
        jmp     .done

.fixup:
        fstp st0

        inc     dword [.exp]
        fild    dword [.exp]
        fld     st0
        fbstp   tword [.bcd_exp]

        fldl2t
        fmulp
        fld st0
        frndint
        fxch
        fsub    st,st1

        f2xm1
        fld1
        faddp
        fscale
        fstp    st1
        fdivp
.done:
        fimul   dword [n_digits]
        fbstp   tword [.bcd_man]

        lea     edi, [.cvt_buf]
        mov     edx, dword [.bcd_man]
        mov     ecx, 8
@@:
        xor     eax, eax
        shld    eax, edx, 4
        stosb
        shl     edx, 4
        loop    @B

        lea     esi, [.cvt_buf+7]
        lea     edi, [.str_buf+13]
        mov     ecx, 7
        mov     ah, 0x30
        std
.skip_z:
        lodsb
        test    al, al
        jnz     .body
        loop    .skip_z
.body:
        add     al, ah
        stosb
        lodsb
        jcxz    .point
        loop    .body
.point:
        dec     edi
        add     al, ah
        mov     ah, '.'
        stosw

        bt      word [_st0+ebp+8], 15
        jnc     .m_sign
        mov     al, '-'
        mov     [edi+1], al

.m_sign:
        cld

        mov     dx, word [.bcd_exp]
        test    dx, dx
        jz      .display

        lea     edi, [.str_buf+15]
        mov     ax, 'E '
        cmp     byte [.bcd_exp+9], 0x80
        jne     .w_e_sign
        mov     ax, 'E-'

.w_e_sign:
        stosw

        mov     ecx, 4
.skip_lz:
        xor     eax, eax
        shld    ax, dx, 4
        shl     dx, 4
        test al, al
        jnz     .w_exp
        loop    .skip_lz
.w_exp:
        add     al, 0x30
        stosb
        xor     eax, eax
        shld    ax, dx, 4
        shl     dx,4
        loop    .w_exp

.display:

        mov     ecx, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_INACTIVE or 0x40000000)
        cmp     [debuggee_pid], 0
        jz      .do_label
        cmp     [bSuspended], 0
        jz      .do_label

        mov     ecx, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_NORMAL or 0x40000000)

        mov     eax, dword [_st0+ebp]
        cmp     eax, dword [_st0+(oldcontext-context)+ebp]
        jne     .scol

        mov     eax, dword [_st0+ebp+4]
        cmp     eax, dword [_st0+(oldcontext-context)+ebp+4]
        jne     .scol

        mov     ax, word [_st0+ebp+8]
        cmp     ax, word [_st0+(oldcontext-context)+ebp+8]
        je      .do_label

.scol:
        mov     ecx, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_CHANGED or 0x40000000)

.do_label:
        ; draw a text string in the window
        mov     esi, 23
        mov     edx, .str_buf
        mov     edi, COLOR_BG_NORMAL
        mcall   4

        shr     ebp, 4
        add     esp, 32+16+10+10+4+4

        ret


;-----------------------------------------------------------------------------
;                      Show FPU MMX register content
;
; in: ebp index, ebx = coord

draw_mmx_register_2:

        sub     esp, 24
        lea     edi, [esp+4]

        shl     ebp, 4
        mov     eax, ebp
        shl     eax, 16-4
        add     eax, 'MM0:'
        mov     [esp], eax

        mov     edx, dword [_mm0+ebp+4]
        call    .hex_2_str
        mov     al, ' '
        stosb
        call    .hex_2_str
        mov     al, ' '
        stosb

        mov     edx, dword [_mm0+ebp]
        call    .hex_2_str
        mov     al, ' '
        stosb
        call    .hex_2_str

        mov     ecx, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_INACTIVE or 0x40000000)
        cmp     [debuggee_pid], 0
        jz      .cd
        cmp     [bSuspended], 0
        jz      .cd

        mov     ecx, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_NORMAL or 0x40000000)

        mov     eax, dword [_mm0+ebp]
        cmp     eax, dword [_mm0+(oldcontext-context)+ebp]
        jne     .scol

        mov     eax, dword [_mm0+ebp+4]
        cmp     eax, dword [_mm0+(oldcontext-context)+ebp+4]
        je     .cd

.scol:
        mov     ecx, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_CHANGED or 0x40000000)
.cd:
        ; draw a text string in the window

        mov     esi, 23
        mov     edx, esp
        mov     edi, COLOR_BG_NORMAL
        mcall   4
        shr     ebp, 4
        add     esp, 24
        ret

align 4
.hex_2_str:
        mov     ecx, 4
@@:
        xor     eax, eax
        shld    eax, edx, 4
        aaa
        adc     al, 0x30
        aad     16
        shl     edx, 4
        stosb
        loop @B
        ret

align 4
draw_fpu_regs:
        push    ebp
        push    8
        xor     ebp, ebp
        mov     ebx, [registers_x_pos_dd]
        add     ebx, 2*10000h+REGISTERS_Y_POS+14*(TEXT_Y_CLEARANCE+FONT_Y_SIZE)
.draw_regs:
        call    draw_fpu_register_2
        add     ebx, FONT_Y_SIZE+TEXT_Y_CLEARANCE
        inc     ebp
        dec     dword [esp]
        jnz     .draw_regs
        pop     eax                         ;restore stack
        pop     ebp
        ret

align 4
draw_mmx_regs:
        push    ebp
        push    8
        xor     ebp, ebp
        mov     ebx, [registers_x_pos_dd]
        add     ebx, 2*10000h+REGISTERS_Y_POS+14*(TEXT_Y_CLEARANCE+FONT_Y_SIZE)
.draw_regs:
        call    draw_mmx_register_2
        add     ebx, FONT_Y_SIZE+TEXT_Y_CLEARANCE
        inc     ebp
        dec     dword [esp]
        jnz     .draw_regs
        pop     eax
        pop     ebp
        ret

align 4
draw_sse_regs:
        push    ebp
        push    8
        xor     ebp, ebp
        mov     ebx, [registers_x_pos_dd]
        add     ebx, 2*10000h+REGISTERS_Y_POS+14*(TEXT_Y_CLEARANCE+FONT_Y_SIZE)
.draw_regs:
     ;   call    draw_sse_register
        add     ebx, FONT_Y_SIZE+TEXT_Y_CLEARANCE
        inc     ebp
        dec     dword [esp]
        jnz     .draw_regs
        pop     eax
        pop     ebp
        ret

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
        mov     ecx, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_INACTIVE or 0x40000000)
        cmp     [debuggee_pid], 0
        jz      .doit
        cmp     [bSuspended], 0
        jz      .doit

        mov     ecx, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_NORMAL or 0x40000000)
        bt      [_eflags], edi
        lahf
        bt      dword [_eflags + oldcontext - context], edi
        rcl     ah, 1
        test    ah, 3
        jp      .doit
        mov     ecx, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_CHANGED or 0x40000000)

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
        mov     ecx, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_NORMAL or 0x40000000)
        mov     esi, 7
        cmp     [reg_mode], REG_MODE_CPU
        jz      @f
        mov     ecx, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_INACTIVE or 0x40000000)
    @@:
        movzx   edx, [fpu_mode]
        mov     edx, [btn2_tab+edx*4]

        ; draw a text string in the window
        mov     ebx, [registers_x_pos_dd]
        add     ebx, 4*10000h+REGISTERS_Y_POS
        mcall   4

        cmp     [reg_mode], REG_MODE_SSE
        jz      @f
        mov     ecx, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_INACTIVE or 0x40000000)
    @@:
        mov     edx, aSSE
        ; draw a text string in the window
        mov     ebx, [registers_x_pos_dd]
        add     ebx, (7*FONT_X_SIZE+4)*10000h+REGISTERS_Y_POS
        mcall   4

        cmp     [reg_mode], REG_MODE_AVX
        jz      @f
        mov     ecx, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_INACTIVE or 0x40000000)
    @@:
        mov     edx, aAVX
        ; draw a text string in the window
        mov     ebx, [registers_x_pos_dd]
        add     ebx, (14*FONT_X_SIZE+4)*10000h+REGISTERS_Y_POS
        mcall   4
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
        mov     ebx, [registers_x_pos_dd]
        add     ebx, 2*10000h+REGISTERS_Y_POS+2*(FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        call    draw_register
        mov     ebx, [registers_x_pos_dd]
        add     ebx, 2*10000h+REGISTERS_Y_POS+3*(FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        add     esi, _ebx-_eax
        call    draw_register
        mov     ebx, [registers_x_pos_dd]
        add     ebx, 2*10000h+REGISTERS_Y_POS+4*(FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        add     esi, _ecx-_ebx
        call    draw_register
        mov     ebx, [registers_x_pos_dd]
        add     ebx, 2*10000h+REGISTERS_Y_POS+5*(FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        add     esi, _edx-_ecx
        call    draw_register
        mov     ebx, [registers_x_pos_dd]
        add     ebx, 2*10000h+REGISTERS_Y_POS+6*(FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        add     esi, _esi-_edx
        call    draw_register
        mov     ebx, [registers_x_pos_dd]
        add     ebx, 2*10000h+REGISTERS_Y_POS+7*(FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        add     esi, _edi-_esi
        call    draw_register
        mov     ebx, [registers_x_pos_dd]
        add     ebx, 2*10000h+REGISTERS_Y_POS+8*(FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        add     esi, _ebp-_edi
        call    draw_register
        mov     ebx, [registers_x_pos_dd]
        add     ebx, 2*10000h+REGISTERS_Y_POS+9*(FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        add     esi, _esp-_ebp
        call    draw_register
        mov     ebx, [registers_x_pos_dd]
        add     ebx, 2*10000h+REGISTERS_Y_POS+10*(FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        add     esi, _eip-_esp
        call    draw_register
        mov     ebx, [registers_x_pos_dd]
        add     ebx, 2*10000h+REGISTERS_Y_POS+11*(FONT_Y_SIZE+TEXT_Y_CLEARANCE)

        mov     cl, 7
        add     esi, _eflags-_eip
        call    draw_register

        cmp     [fpu_mode], 1
        je      .fpu
        call    draw_mmx_regs
        jmp     @f
  .fpu:
        call    draw_fpu_regs
    @@:
        mov     ecx, (0 and 11b) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_INACTIVE
        cmp     [debuggee_pid], 0
        jz      @f
        cmp     [bSuspended], 0
        jz      @f
        mov     ecx, (0 and 11b) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_NORMAL
    @@:
        mov     edx, aColon
        xor     esi, esi
        inc     esi
        mov     ebx, [registers_x_pos_dd]
        add     ebx, 10*10000h+REGISTERS_Y_POS+12*(FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        mcall   4
        mov     edx, flags

    @@:
        add     ebx, 2*FONT_X_SIZE*10000h
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
        mov     ebx, [registers_x_pos_dd]
        add     ebx, (-1)*10000h+(REGISTERS_X_SIZE+2)
        mov     ecx, [registers_y_size_dd+4]
        add     ecx, ((TITLE_Y_POS+(FONT_Y_SIZE/2))+1)*10000h+2
        mcall   13, , , COLOR_BG_NORMAL
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
        mov     ebx, [data_x_size_dd+4]
        add     ebx, DATA_X_POS*10000h
        mcall   13, , DUMP_Y_POS*10000h+DUMP_Y_SIZE, COLOR_BG_NORMAL

    .redraw:
        ; addresses
        mov     ebx, 80100h
        mov     edx, DATA_X_POS*10000h + DUMP_Y_POS
        mov     ecx, [dumppos]
        mov     edi, COLOR_BG_NORMAL
        mov     esi, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_INACTIVE or 0x40000000)
        cmp     [debuggee_pid], 0
        jz      @f
        cmp     [bSuspended], 0
        jz      @f
        mov     esi, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_NORMAL or 0x40000000)
    @@:
        ; draw a number in the window
        mcall   47
        add     ecx, 10h
        add     edx, FONT_Y_SIZE + TEXT_Y_CLEARANCE
        cmp     dl, DUMP_Y_POS + DUMP_Y_SIZE
        jb      @b
        ; hex dump of data
        mov     ecx, dumpdata
        push    ecx
        xor     ebx, ebx
        mov     edx, (DATA_X_POS+12*FONT_X_SIZE)*10000h + DUMP_Y_POS
        cmp     [dumpread], ebx
        jz      .hexdumpdone1

    .hexdumploop1:
        push    ebx
        mov     ebx, 20101h
        ; draw a number in the window
        mcall
        pop     ebx
        add     edx, 3*FONT_X_SIZE*10000h
        inc     ecx
        inc     ebx
        test    bl, 15
        jz      .16
        test    bl, 7
        jnz     @f
        add     edx, 2*FONT_X_SIZE*10000h - (FONT_Y_SIZE+TEXT_Y_CLEARANCE) + FONT_X_SIZE*(3*10h+2)*10000h

    .16:
        add     edx, (FONT_Y_SIZE+TEXT_Y_CLEARANCE) - FONT_X_SIZE*(3*10h+2)*10000h

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
        cmp     edx, DUMP_HEIGHT*10h
        jae     .hexdumpdone2
        push    edx
        mov     edx, aQuests
        ; draw text string with color in ecx, copied from esi
        mcall
        pop     edx
        add     ebx, 3*FONT_X_SIZE*10000h
        inc     edx
        test    dl, 15
        jz      .16x
        test    dl, 7
        jnz     .hexdumploop2
        add     ebx, 2*FONT_X_SIZE*10000h - (FONT_Y_SIZE+TEXT_Y_CLEARANCE) + FONT_X_SIZE*(3*10h+2)*10000h

  .16x:
        add     ebx, (FONT_Y_SIZE+TEXT_Y_CLEARANCE) - FONT_X_SIZE*(3*10h+2)*10000h
        jmp     .hexdumploop2

  .hexdumpdone2:
        dec     esi
        ; colon, minus signs
        mov     ebx, (DATA_X_POS+8*FONT_X_SIZE)*10000h + DUMP_Y_POS
        mov     edx, aColon

    @@:
        mcall
        add     ebx, (FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        cmp     bl, DUMP_Y_POS+DUMP_HEIGHT*(FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        jb      @b
        mov     ebx, (DATA_X_POS+(12+3*8)*FONT_X_SIZE)*10000h + DUMP_Y_POS
        mov     edx, aMinus

    @@:
        mcall
        add     ebx, (FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        cmp     bl, DUMP_Y_POS+DUMP_HEIGHT*(FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        jb      @b
        ; ASCII data
        mov     ebx, (DATA_X_POS+(12+3*10h+2+2)*FONT_X_SIZE)*10000h + DUMP_Y_POS
        pop     edx
        push    DUMP_HEIGHT*10h

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
        add     ebx, FONT_X_SIZE*10000h
        dec     dword [esp]
        jz      .asciidone
        test    byte [esp], 15
        jnz     .asciiloop
        add     ebx, (FONT_Y_SIZE+TEXT_Y_CLEARANCE) - FONT_X_SIZE*10h*10000h
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
        imul    ebx, (FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        push    ebx
        lea     ecx, [ebx+DISASM_Y_POS-1]
        shl     ecx, 16
        mov     cl, FONT_Y_SIZE+TEXT_Y_CLEARANCE
        ; setting up background color for disassembled text
        mov     edx, COLOR_BG_NORMAL
        ; draw container rectangle/box with color COLOR_BG_NORMAL (was 0xFFFFFF - white)
        mov     ebx, [data_x_size_dd+4]
        add     ebx, DATA_X_POS*10000h
        mcall   13
        pop     ebx
        ; copy color value from edx (COLOR_BG_NORMAL)
        mov     edi, edx
        add     ebx, (DATA_X_POS+FONT_X_SIZE*2)*10000h+DISASM_Y_POS
        mov     edx, esi

    @@:
        lodsb
        test    al, al
        jnz     @b
        mov     byte [esi-1], ':'
        sub     esi, edx
        ; normal color
        ; was 0x40000000
        mov     ecx, ((FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_LABEL or 0x40000000)
        mov     al, 4
        ; draw a text string in the window with color COLOR_TXT_NORMAL in ecx
        mcall
        mov     byte [esi+edx-1], 0
;         lea     esi, [esi*3]
;         movzx   ecx, bx
;         shr     ebx, 16
;         lea     ebx, [ebx+esi*2]
;         shl     ecx, 16
;         mov     cl, 10
;         imul    ebx, 10001h
;         sub     bx, DATA_X_POS
;         sub     bx, word[data_x_size_dd+4]
;         neg     bx
;         mov     al, 13
;         copy color value from edi
;         mov     edx, edi
;         draw container rectangle/box for disassembled text, color in edx
;         mcall
        inc     [disasm_cur_str]
        mov     eax, [disasm_height_dd]
        cmp     [disasm_cur_str], eax
        jae     .loopend

    .nosymb:
        push    [disasm_cur_pos]
        call    disasm_instr
        pop     ebp
        jc      .loopend
        mov     edx, COLOR_BG_NORMAL
        mov     esi, (FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_NORMAL
        mov     ebx, DATA_X_POS*10000h
        add     ebx, [data_x_size_dd+4]
        mov     ecx, [disasm_cur_str]
        imul    ecx, (FONT_Y_SIZE+TEXT_Y_CLEARANCE)*10000h
        add     ecx, (DISASM_Y_POS-1)*10000h + (FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        mov     eax, ebp
        pushad
        call    find_enabled_breakpoint
        popad
        jnz     .nobp
        mov     edx, COLOR_BG_BREAKPOINT
        mov     esi, (FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_BREAKPOINT
    .nobp:

        mov     eax, [_eip]
        cmp     eax, ebp
        jnz     .notcurrent
        mov     edx, COLOR_BG_SELECTED
        mov     esi, (FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_SELECTED
    .notcurrent:
        push    esi     ; Save color value for disassembled text

        ; draw container rectangle/box for disassembled text
        ; color in edx
        mcall   13

        mov     edx, [disasm_cur_str]
        imul    edx, (FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        add     edx, DATA_X_POS*10000h + DISASM_Y_POS
        ; draw a number in the window, color in esi
        mcall   47, 80100h, ebp

        lea     ebx, [edx+8*FONT_X_SIZE*10000h]
        mov     ecx, esi    ; text color
        push    2
        pop     esi
        mov     edx, aColon
        ; draw the colon
        mcall   4
        push    9
        pop     edi
        lea     edx, [ebx+2*FONT_X_SIZE*10000h]
        mov     ecx, ebp
        sub     ecx, [disasm_start_pos]
        add     ecx, disasm_buffer

        mov     esi, (FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_HEX
        mov     eax, [_eip]
        cmp     eax, ebp
        jnz     @f
        mov     esi, (FONT) shl 28 or ((FONT_MULTIPLIER - 1) and 111b) shl 24 or COLOR_TXT_SELECTED
  @@:
    .drawhex:
        ; draw a number in the window, color in esi
        mcall   47, 20101h
        add     edx, FONT_X_SIZE*3*10000h
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

        lea     ebx, [edx-FONT_X_SIZE*10000h]
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
        imul    ebx, (FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        add     ebx, (DATA_X_POS+FONT_X_SIZE*40)*10000h+DISASM_Y_POS

        ; draw a text string in the window, color in ecx
        mcall   4
        inc     [disasm_cur_str]
        mov     eax, [disasm_height_dd]
        cmp     [disasm_cur_str], eax
        jb      .loop

    .loopend:
        mov     ecx, [disasm_height_dd]
        sub     ecx, [disasm_cur_str]
        jz      @f
        imul    ecx, (FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        inc     ecx
        mov     eax, DISASM_Y_POS
        add     eax, [disasm_y_size_dd+4]
        sub     eax, ecx
        shl     eax, 16
        add     ecx, eax
        ; Draw filled rectangle
        mov     ebx, [data_x_size_dd+4]
        add     ebx, DATA_X_POS*10000h
        mcall   13, , , COLOR_BG_NORMAL

    @@:
        ret

;-----------------------------------------------------------------------------

; TODO: cleanup of this function, make some global labels local
update_disasm_eip:
; test if instruction at eip is showed
        mov     ecx, [disasm_height_dd]
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
        mcall   0, wnd_x_size+10, wnd_y_size+30, (COLOR_BG_NORMAL or 0x33000000), ,caption_str

        ; Get actual window size
        mcall   9, thread_info, -1

        ; Check if window is rolled up
        test    [thread_info.wnd_state], 100b
        jnz     .done

        ; Check if window isnt smaller then permisseable
        mov     eax, [thread_info.box.width]            ; window xsize
        mov     ebx, [thread_info.box.height]           ; ysize
        mov     edx, [thread_info.client_box.width]     ; work area xsize
        mov     esi, [thread_info.client_box.height]    ; ysize
        sub     eax, edx
        sub     ebx, esi

        cmp     edx, wnd_x_size
        jae     .x_ok
        mov     edx, wnd_x_size
  .x_ok:
        add     edx, eax

        cmp     esi, wnd_y_size
        jae     .y_ok
        mov     esi, wnd_y_size
  .y_ok:
        add     esi, ebx
        mcall   67, -1, -1      ; set the new sizes

        ; (re)calculate coordinates of GUI elements
        mov     eax, [thread_info.client_box.width]
        sub     eax, DATA_X_POS + DATA_X_POS + REGISTERS_X_SIZE+3
        mov     [data_x_size_dd+4], eax
        mov     [messages_x_size_dd+4], eax
        shl     eax, 16
        mov     [data_x_size_dd], eax
        mov     [messages_x_size_dd], eax
        shr     eax, 16
        add     eax, DATA_X_POS + 4
        mov     [registers_x_pos_dd+4], eax
        shl     eax, 16
        mov     [registers_x_pos_dd], eax

        mov     eax, [thread_info.client_box.height]
        sub     eax, CMD_Y_SIZE + DATA_X_POS
        mov     [cmdline_y_pos_dd+4], eax
        shl     eax, 16
        mov     [cmdline_y_pos_dd], eax
        shr     eax, 16
        sub     eax, MSG_Y_SIZE + 4
        mov     [messages_y_pos_dd+4], eax
        shl     eax, 16
        mov     [messages_y_pos_dd], eax
        shr     eax, 16
        sub     eax, DISASM_Y_POS + 4
        mov     [disasm_y_size_dd+4], eax
        shl     eax, 16
        mov     [disasm_y_size_dd], eax
        shr     eax, 16
        mov     ecx, (FONT_Y_SIZE+TEXT_Y_CLEARANCE)
        xor     edx, edx
        div     ecx
        mov     [disasm_height_dd], eax

        mov     eax, [thread_info.client_box.height]
        sub     eax, DATA_X_POS + TITLE_Y_POS+(FONT_Y_SIZE/2)+2
        mov     [registers_y_size_dd+4], eax
        shl     eax, 16
        mov     [registers_y_size_dd], eax

        ; messages frame
        mov     ebx, [messages_x_size_dd+4]
        add     ebx, (MSG_X_POS-2)*10000h + (MSG_X_POS+2)
        push    ebx
        mov     ecx, [messages_y_pos_dd+2]
        add     ecx, -2*10001h
        mov     edx, COLOR_LINE
        mcall   38
        add     ecx, (MSG_Y_SIZE+2+2)*10001h
        mcall
        mov     ebx, (MSG_X_POS-2)*10001h
        push    ebx
        mov     ecx, [messages_y_pos_dd+2]
        add     ecx, (-2*10000h) + (MSG_Y_SIZE+2)
        mcall
        mov     ebx, [messages_x_size_dd+2]
        add     ebx, (MSG_X_POS+2)*10001h
        push    ebx
        mcall

        ; command line frame
        mov     ecx, [cmdline_y_pos_dd+2]
        add     ecx, (-2*10000h) + (CMD_Y_SIZE+2)
        pop     ebx
        mcall
        pop     ebx
        mcall
        pop     ebx
        mov     ecx, [cmdline_y_pos_dd+2]
        add     ecx, (CMD_Y_SIZE+2)*10001h

        mcall
        mov     ecx, [cmdline_y_pos_dd+2]
        add     ecx, (-2*10001h)
        mcall

        ; registers frame
        mov     ebx, [registers_x_pos_dd]
        add     ebx, (-2) shl 16 + (REGISTERS_X_SIZE+3)
        mov     ecx, (TITLE_Y_POS+(FONT_Y_SIZE/2)) shl 16 + 1
        mov     edx, COLOR_LINE
        mcall   13                      ; top
        mov     ebx, [registers_x_pos_dd]
        add     ebx, ((-2)+(REGISTERS_X_SIZE+3)) shl 16 + 1
        mov     ecx, [registers_y_size_dd+4]
        add     ecx, (TITLE_Y_POS+(FONT_Y_SIZE/2)) shl 16 + (+3+1)
        mcall                           ; right
        mov     ebx, [registers_x_pos_dd]
        add     ebx, (-2) shl 16 + (REGISTERS_X_SIZE+3+1)
        mov     ecx, [registers_y_size_dd]
        add     ecx, ((TITLE_Y_POS+(FONT_Y_SIZE/2))+4) shl 16 + 1
        mcall                           ; bottom

        ; messages
        call    draw_messages

        ; command line & cursor
        call    draw_cmdline
        call    draw_cursor

        ; title & registers & dump & disasm
        mov     ebx, (DATA_X_POS-2)*10001h
        mov     ecx, (TITLE_Y_POS+(FONT_Y_SIZE/2))*10000h + (-2)
        add     ecx, [messages_y_pos_dd+4]
        mov     edx, COLOR_LINE
        mcall   38
        mov     ebx, [data_x_size_dd+2]
        add     ebx, (DATA_X_POS+2)*10001h
        mcall
        mov     ebx, [data_x_size_dd+4]
        add     ebx, (DATA_X_POS-2)*10000h + (DATA_X_POS+2)
        mov     ecx, (DISASM_Y_POS-4)*10001h
        mcall

        mov     ebx, [registers_x_pos_dd]
        add     ebx, (-1) shl 16 + REGISTERS_X_SIZE + 1
        mov     ecx, (REGISTERS_Y_POS - 2) shl 16 + FONT_Y_SIZE + TEXT_Y_CLEARANCE
        mov     edx, (1 shl 30) + 2
        mov     esi, 0x00808080
        mcall   8


        ; redraw whole window again
        call    redraw_title
        call    draw_registers
        call    draw_dump
        call    draw_disasm


  .done:
        ; end of window redraw
        mcall   12, 2
        ret

; vim: ft=fasm tabstop=4
