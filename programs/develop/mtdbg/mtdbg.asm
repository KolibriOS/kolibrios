format binary
include '../../macros.inc'
use32
        db      'MENUET01'
        dd      1
        dd      start
        dd      i_end
        dd      used_mem
        dd      used_mem
        dd      i_param
        dd      0

;-----------------------------------------------------------------------------

REG_MODE_CPU equ 1
REG_MODE_MMX equ 2
REG_MODE_SSE equ 3
REG_MODE_AVX equ 4

;-----------------------------------------------------------------------------

include 'gui.inc' ; GUI routines

;-----------------------------------------------------------------------------
;                          Find command in list

find_cmd:
; all commands are case-insensitive
        push    edi

    .x4:
        mov     al, [edi]
        cmp     al, 0
        jz      .x5
        cmp     al, 'A'
        jb      @f
        cmp     al, 'Z'
        ja      @f
        or      al, 20h

    @@:
        stosb
        jmp     .x4

    ; find command
    .x5:
        pop     edi

    .x6:
        cmp     dword [esi], 0
        jz      .x7
        push    esi
        mov     esi, [esi]
        lodsb
        movzx   ecx, al
        push    edi
        repz cmpsb
        pop     edi
        pop     esi
        jz      .x8
        add     esi, 17
        jmp     .x6

    .x7:
        stc

    .x8:
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; DEBUGGING ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;-----------------------------------------------------------------------------
;                                 Help event

OnHelp:
        mov     esi, help_msg
        mov     edi, [curarg]
        cmp     byte [edi], 0
        jz      .x
        mov     esi, help_groups
        call    find_cmd
        jc      .nocmd
        mov     esi, [esi+12]

    .x:
        jmp     put_message

    .nocmd:
        mov     esi, aUnknownCommand
        jmp     .x

;-----------------------------------------------------------------------------
;                                Quit event
OnQuit:
        mcall  -1

;-----------------------------------------------------------------------------
;                        Working with debug context

get_new_context:
        mov     esi, context
        mov     edi, oldcontext
        mov     ecx, 10
        rep movsd

get_context:
        ;push    1
        ;pop     ebx
        ;push    69
        ;pop     eax
        ;mov     ecx, [debuggee_pid]
        ;mov     esi, context
        ;push    28h
        ;pop     edx
        mcall    69, 1, [debuggee_pid], 28h, context
        ret

set_context:
        ;push    2
        ;pop     ebx
        ;push    69
        ;pop     eax
        ;mov     ecx, [debuggee_pid]
        ;mov     esi, context
        ;push    28h
        ;pop     edx
        mcall    69, 2, [debuggee_pid], 28h, context
        ret

get_dump:
        mov     edi, dumpdata
        mov     esi, [edi-4]
        mov     edx, dump_height*10h
        mov     ecx, edx
        xor     eax, eax
        push    edi
        rep stosb
        pop     edi
        ;mov     ecx, [debuggee_pid]
        ;mov     al, 69
        ;push    6
        ;pop     ebx
        mcall    69, 6, [debuggee_pid]
        cmp     eax, -1
        jnz     @f
        mov     esi, read_mem_err
        call    put_message
        xor     eax, eax

    @@:
        mov     [edi-8], eax
;       call    restore_from_breaks
;       ret

; in: edi=buffer,eax=size,esi=address
restore_from_breaks:
        mov     ebx, breakpoints

    @@:
        test    byte [ebx+4], 1
        jz      .cont           ; ignore invalid
        test    byte [ebx+4], 2 or 8
        jnz     .cont           ; ignore disabled and memory breaks
        mov     ecx, [ebx]
        sub     ecx, esi
        cmp     ecx, eax
        jae     .cont
        mov     dl, [ebx+5]
        mov     [edi+ecx], dl

    .cont:
        add     ebx, 6
        cmp     ebx, breakpoints+breakpoints_n*6
        jb      @b
        ret

;-----------------------------------------------------------------------------
;                           Load executable event

OnLoad:
        mov     esi, [curarg]

OnLoadInit:
        mov     edi, loadname
        or      [prgname_len], -1
        mov     [prgname_ptr], edi

    .copyname:
        lodsb
        stosb
        inc     [prgname_len]
        cmp     al, '/'
        jnz     @f
        or      [prgname_len], -1
        mov     [prgname_ptr], edi

    @@:
        cmp     al, ' '
        ja      .copyname
        mov     byte [edi-1], 0
        and     [load_params], 0
        dec     esi
        call    get_arg.skip_spaces
        cmp     al, 0
        jz      @f
        mov     [load_params], esi

    @@:
        and     [dumppos], 0
        mov     ecx, [symbols]
        jecxz   do_reload
        mcall   68, 13
        and     [symbols], 0
        and     [num_symbols], 0

; TODO: make it local
do_reload:
        ;push    18
        ;pop     eax
        ;push    7
        ;pop     ebx
        mcall    18, 7
        mov     [dbgwnd], eax
        xchg    ecx, eax
        ;push    70
        ;pop     eax
        ;mov     ebx, fn70_load_block
        mcall    70, fn70_load_block
        test    eax, eax
        jns     .load_ok

    .load_err:
        push    eax
        mov     esi, load_err_msg
        call    put_message
        pop     eax
        not     eax
        cmp     eax, 0x20
        jae     .unk_err
        mov     esi, [load_err_msgs+eax*4]
        test    esi, esi
        jnz     put_message

    .unk_err:
        mov     esi, unk_err_msg
        inc     eax
        push    eax
        call    put_message_nodraw
        jmp     draw_messages

    .load_ok:
        mov     [debuggee_pid], eax
        mov     [bSuspended], 1
        push    ecx
        call    get_context
        mov     edi, oldcontext
        mov     ecx, 10
        rep movsd

    ; activate debugger window
        pop     ecx
        ;mov     bl, 3
        ;push    18
        ;pop     eax
        mcall    18, 3
        call    redraw_title
        call    draw_registers.redraw
    ; read and draw dump of memory
        call    get_dump
        call    draw_dump.redraw
        call    update_disasm_eip_force
        mov     esi, load_succ_msg
        push    [debuggee_pid]
        call    put_message_nodraw
        call    draw_messages
    ; try to load symbols
        mov     esi, loadname
        mov     edi, symbolsfile
        push    edi

    @@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        lea     ecx, [edi-1]

    @@:
        dec     edi
        cmp     edi, symbolsfile
        jb      @f
        cmp     byte [edi], '/'
        jz      @f
        cmp     byte [edi], '.'
        jnz     @b
        mov     ecx, edi

    @@:
        mov     dword [ecx], '.dbg'
        mov     byte [ecx+4], 0
        pop     esi
        mov     ebp, esi
        call    OnLoadSymbols.silent
    
    ; now test for packed progs
        cmp     [disasm_buf_size], 100h
        jz      @f
        ret

    @@:
        mov     esi, mxp_nrv_sig
        mov     ebp, disasm_buffer
        mov     edi, ebp
        push    3
        pop     ecx
        repz cmpsb
        jnz     .not_mxp_nrv
        cmpsb
        mov     cl, mxp_nrv_sig_size-4
        repz cmpsb
        mov     esi, mxp_nrv_name
        jz      .packed

    .not_mxp_nrv:
        mov     esi, mxp_sig
        mov     edi, ebp
        mov     cl, mxp_sig_size
        repz cmpsb
        mov     esi, mxp_name
        jz      .packed

    .not_mxp:
        mov     esi, mxp_lzo_sig1
        mov     edi, ebp
        mov     cl, mxp_lzo_sig1_size
        repz cmpsb
        mov     esi, mxp_lzo_name
        jz      .packed
        mov     esi, mxp_lzo_sig2
        mov     edi, ebp
        mov     cl, 8
        repz cmpsb
        jnz     .not_mxp_lzo
        cmpsb
        mov     cl, mxp_lzo_sig2_size - 9
        repz cmpsb
        mov     esi, mxp_lzo_name
        jz      .packed

    .not_mxp_lzo:
        mov     esi, mtappack_name
        cmp     dword [ebp], 0xBF5E246A
        jnz     .not_mtappack
        cmp     dword [ebp+8], 0xEC4E8B57
        jnz     .not_mtappack1
        cmp     dword [ebp+12], 0x8D5EA4F3
        jnz     .not_mtappack1
        cmp     byte [ebp+12h], 0xE9
        jz      .packed

    .not_mtappack1:
        cmp     word [ebp+8], 0xB957
        jnz     .not_mtappack
        cmp     dword [ebp+14], 0x575EA4F3
        jnz     .not_mtappack2
        cmp     byte [ebp+17h], 0xE9
        jz      .packed

    .not_mtappack2:
        cmp     dword [ebp+14], 0x5F8DA4F3
        jnz     .not_mtappack3
        cmp     word [ebp+18], 0xE9FC
        jz      .packed

    .not_mtappack3:
        cmp     word [ebp+14], 0xA4F3
        jnz     .not_mtappack
        cmp     byte [ebp+15h], 0xE9
        jz      .packed

    .not_mtappack:
        ret

    .packed:
        push    esi
        mov     esi, aPacked1
        call    put_message_nodraw
        pop     esi
        call    put_message_nodraw
        mov     esi, aPacked2
        call    put_message
        call    hide_cursor
        ;push    40
        ;pop     eax
        ;push    7
        ;pop     ebx
        mcall    40, 7

    .wait:
        ;push    10
        ;pop     eax
        mcall    10
        dec     eax
        jz      .redraw
        dec     eax
        jz      .key
        or      eax, -1
        mcall

    .redraw:
        call    draw_window
        call    hide_cursor
        jmp     .wait

    .key:
        mov     al, 2
        mcall
        cmp     ah, 'y'
        jz      .yes
        cmp     ah, 'Y'
        jz      .yes
        cmp     ah, 0xD
        jz      .yes
        cmp     ah, 'n'
        jz      .no
        cmp     ah, 'N'
        jnz     .wait

    .no:
        ;push    40
        ;pop     eax
        ;mov     ebx, 0x107
        mcall    40, 0x107
        call    draw_cursor
        mov     esi, aN_str
        jmp     put_message

    .yes:
        ;push    40
        ;pop     eax
        ;mov     ebx, 0x107
        mcall    40, 0x107
        call    draw_cursor
        mov     esi, aY_str
        call    put_message
        call    OnUnpack
        ret

;-----------------------------------------------------------------------------
;                       Searching signatures

mxp_nrv_sig:
        xor     eax, eax
        mov     ecx, 0x95       ; 0xA1 for programs with parameters
        mov     [eax], ecx
        add     ecx, [eax+24h]
        push    40h
        pop     esi
        mov     edi, [eax+20h]
        push    edi
        rep movsb
        jmp     dword [esp]
        pop     esi
        add     esi, [eax]
        xor     edi, edi

mxp_nrv_sig_size = $ - mxp_nrv_sig

mxp_sig:
        mov     ecx, 1CBh
        push    46h
        pop     esi
        mov     edi, [20h]
        rep movsb
        mov     ecx, [24h]
        rep movsb
        jmp     dword [20h]
        mov     eax, [20h]
        add     eax, 1CBh
        push    eax
        push    dword [24h]
        push    0
        push    8
        call    $+0x25

mxp_sig_size = $ - mxp_sig

mxp_lzo_sig1:
        xor     eax, eax
        mov     ebp, 0FFh
        mov     ecx, 175h
        mov     [eax], ecx
        add     ecx, [eax+24h]
        push    45h
        pop     esi
        mov     edi, [eax+20h]
        push    edi
        rep movsb
        jmp     dword [esp]
        pop     ebx
        add     ebx, [eax]
        xor     edi, edi
        cmp     byte [ebx], 11h
        jbe     $+0x1A

mxp_lzo_sig1_size = $ - mxp_lzo_sig1

mxp_lzo_sig2:
        xor     eax, eax
        mov     ebp, 0FFh
        mov     ecx, 188h       ; or 177h
        mov     [eax], ecx
        add     ecx, [eax+24h]
        push    44h
        pop     esi
        mov     edi, [eax+20h]
        rep movsb
        jmp     dword [eax+20h]
        mov     ebx, [eax+20h]
        add     ebx, [eax]

mxp_lzo_sig2_size = $ - mxp_lzo_sig2

;-----------------------------------------------------------------------------
;                         Reload executable event

OnReload:
        cmp     [debuggee_pid], 0
        jnz     terminate_reload
        mov     esi, need_debuggee
        cmp     byte [loadname], 0
        jnz     do_reload
        jz      put_message

; TODO: make it local
terminate_reload:
        mov     [bReload], 1

;-----------------------------------------------------------------------------
;                        Terminate process event

OnTerminate:
        ;mov     ecx, [debuggee_pid]
        ;push    8
        ;pop     ebx
        ;push    69
        ;pop     eax
        mcall    69, 8, [debuggee_pid]
        ret
;-----------------------------------------------------------------------------
;                         Suspend process event

AfterSuspend:
        mov     [bSuspended], 1
        call    get_new_context
        call    get_dump
        call    redraw_title
        call    draw_registers.redraw
        call    draw_dump.redraw
        call    update_disasm_eip
        ret

OnSuspend:
        ;mov     ecx, [debuggee_pid]
        ;push    4
        ;pop     ebx
        ;push    69
        ;pop     eax
        mcall    69, 4, [debuggee_pid]
        call    AfterSuspend
        mov     esi, aSuspended
        jmp     put_message

;-----------------------------------------------------------------------------
;                        Resume process event

DoResume:
        ;mov     ecx, [debuggee_pid]
        ;push    5
        ;pop     ebx
        ;push    69
        ;pop     eax
        mcall    69, 5, [debuggee_pid]
        mov     [bSuspended], 0
        ret

OnResume:
        mov     esi, [curarg]
        cmp     byte [esi], 0
        jz      GoOn
        call    calc_expression
        jc      .ret
        mov     eax, ebp
        push    eax
        call    find_enabled_breakpoint
        pop     eax
        jz      GoOn
        mov     bl, 5   ; valid enabled one-shot
        call    add_breakpoint
        jnc     GoOn
        mov     esi, aBreakpointLimitExceeded
        call    put_message

    .ret:
        ret

GoOn:
    ; test for enabled breakpoint at eip
        mov     eax, [_eip]
        call    find_enabled_breakpoint
        jnz     .nobreak
    ; temporarily disable breakpoint, make step, enable breakpoint, continue
        inc     eax
        mov     [temp_break], eax
        mov     [bAfterGo], 1
        dec     eax
        call    disable_breakpoint
        call    get_context
        or      byte [_eflags+1], 1             ; set TF
        call    set_context
        and     byte [_eflags+1], not 1
        call    DoResume
        ret

    .nobreak:
        call    DoResume
        call    redraw_title
        call    draw_registers.redraw
        call    draw_dump.redraw
        ret

;-----------------------------------------------------------------------------
;                        Detach process event

OnDetach:
        ;mov     ecx, [debuggee_pid]
        ;push    3
        ;pop     ebx
        ;push    69
        ;pop     eax
        mcall    69, 3, [debuggee_pid]
        and     [debuggee_pid], 0
        call    redraw_title
        call    draw_registers.redraw
        call    draw_dump.redraw
        call    free_symbols
        mov     esi, aContinued
        jmp     put_message

after_go_exception:
        push    eax
        mov     eax, [temp_break]
        dec     eax
        push    esi
        call    enable_breakpoint
    ; in any case, clear TF and RF
        call    get_new_context
        and     [_eflags], not 10100h           ; clear TF,RF
        call    set_context
        xor     edx, edx
        mov     [temp_break], edx
        xchg    dl, [bAfterGo]
        pop     esi
        pop     eax
        cmp     dl, 2
        jnz     @f
        lodsd
        push    esi
        call    get_dump
        jmp     exception.done

    @@:
        test    eax, eax
        jz      .notint1
    ; if exception is result of single step, simply ignore it and continue
        test    dword [esi], 0xF
        jnz     dbgmsgstart.5
        lodsd
        push    esi
        mov     esi, oldcontext
        mov     edi, context
        mov     ecx, 28h/4
        rep movsd
        call    DoResume
        jmp     dbgmsgend

    .notint1:
    ; in other case, work as without temp_break
        lodsd
        push    esi
        push    eax
        jmp     exception.4

    .notour:

; TODO: split it out
debugmsg:
        neg     [dbgbufsize]
        mov     esi, dbgbuf

; TODO: make it local
dbgmsgstart:
        lodsd
;       push    eax esi
;       push    dword [esi]
;       mov     esi, dbgmsg_str
;       call    put_message_nodraw
;       pop     esi eax
        add     esi, 4
        dec     eax
        jz      exception
        dec     eax
        jz      terminated
        mov     [bSuspended], 1
        cmp     [bAfterGo], 0
        jnz     after_go_exception
        push    esi
        call    get_new_context
        and     [_eflags], not 10100h           ; clear TF,RF
        call    set_context
        pop     esi

    ; TODO: WTF? Need for meaning label names
    .5:
        push    esi
        call    get_dump
        pop     esi
        lodsd
        xor     ecx, ecx

    .6:
        bt      eax, ecx
        jnc     .7
        mov     ebx, [drx_break+ecx*4]
        test    ebx, ebx
        jz      .7
        pushad
        dec     ebx
        push    ebx
        mov     esi, aBreakStop
        call    put_message_nodraw
        popad

    .7:
        inc     ecx
        cmp     cl, 4
        jb      .6
        push    esi
        jmp     exception.done_draw

; TODO: make it local
terminated:
        push    esi
        mov     esi, terminated_msg
        call    put_message
        and     [debuggee_pid], 0
        and     [temp_break], 0
        mov     [bAfterGo], 0
        xor     eax, eax
        mov     ecx, breakpoints_n*6/4+4
        mov     edi, breakpoints
        rep stosd
        cmp     [bReload], 1
        sbb     [bReload], -1
        jnz     exception.done
        call    free_symbols
        jmp     exception.done

exception:
        mov     [bSuspended], 1
        cmp     [bAfterGo], 0
        jnz     after_go_exception
        lodsd
        push    esi
        push    eax
        call    get_new_context
        and     [_eflags], not 10100h           ; clear TF,RF
        call    set_context

    ; TODO: fix for useful name
    .4:
        call    get_dump
        pop     eax
    ; int3 command generates exception 0D, #GP
        push    eax
        cmp     al, 0Dh
        jnz     .notdbg
    ; check for 0xCC byte at eip
        push    0
        ;push    69
        ;pop     eax
        ;push    6
        ;pop     ebx
        ;mov     ecx, [debuggee_pid]
        ;mov     edi, esp
        ;mov     esi, [_eip]
        ;push    1
        ;pop     edx
        mcall    69, 6, [debuggee_pid], 1, [_eip], esp
        pop     eax
        cmp     al, 0xCC
        jnz     .notdbg
    ; this is either dbg breakpoint or int3 cmd in debuggee
        mov     eax, [_eip]
        call    find_enabled_breakpoint
        jnz     .user_int3
    ; dbg breakpoint; clear if one-shot
        pop     ecx
        push    eax
        mov     esi, aBreakStop
        test    byte [edi+4], 4
        jz      .put_msg_eax
        pop     ecx
        call    clear_breakpoint
        jmp     .done

    .user_int3:
        mov     eax, [_eip]
        inc     [_eip]
        pop     ecx
        push    eax
        call    set_context
        mov     esi, aUserBreak
        jmp     .put_msg_eax

    .notdbg:
        mov     esi, aException

    .put_msg_eax:
        call    put_message_nodraw

    .done_draw:
        call    draw_messages

    .done:
        ;push    18
        ;pop     eax
        ;push    3
        ;pop     ebx
        ;mov     ecx, [dbgwnd]
        mcall    18, 3, [dbgwnd]    ; activate dbg window
        call    redraw_title
        call    draw_registers.redraw
        call    draw_dump.redraw
        call    update_disasm_eip

dbgmsgend:
        pop     esi
        mov     ecx, [dbgbuflen]
        add     ecx, dbgbuf
        cmp     esi, ecx
        jnz     dbgmsgstart
        and     [dbgbuflen], 0
        neg     [dbgbufsize]
        cmp     [bReload], 2
        jnz     @f
        mov     [bReload], 0
        call    do_reload

    @@:
        jmp     waitevent

; TODO: make it local
CtrlF7:
        cmp     [debuggee_pid], 0
        jz      .no
        call    OnStep

    .no:
        jmp     waitevent

; TODO: make it local
CtrlF8:
        cmp     [debuggee_pid], 0
        jz      CtrlF7.no
        call    OnProceed
        jmp     CtrlF7.no

;-----------------------------------------------------------------------------
;                       Step execution event

;Here we get [<number>] argument at do step <number> times
OnStepMultiple:
        cmp     [bSuspended], 0
        jz      OnStep.running
        mov     [step_num], 1
        mov     esi, [curarg]
        test    esi, esi
        jz      .do
        cmp     byte [esi], 0
        jz      .do
        call    get_hex_number
        jc      .ret
        cmp     eax, 0 ; check if lesser or equal than 0
        jle     .ret
        mov     [step_num], eax
.do:
        call    OnStep
        dec     [step_num]
        jnz     .do
.ret:
        ret

OnStep:
        cmp     [bSuspended], 0
        jz      .running
        call    get_context
        or      byte [_eflags+1], 1             ; set TF
        call    set_context
        and     byte [_eflags+1], not 1
    ; if instruction at eip is "int xx", set one-shot breakpoint immediately after
        mov     eax, [_eip]
        call    find_enabled_breakpoint
        jnz     @f
        cmp     byte [edi+5], 0xCD
        jz      .int

     @@:
        push    0
        ;push    69
        ;pop     eax
        ;push    6
        ;pop     ebx
        ;mov     ecx, [debuggee_pid]
        ;push    3
        ;pop     edx
        ;mov     edi, esp
        ;mov     esi, [_eip]
        mcall    69, 6, [debuggee_pid], 3, [_eip], esp
        cmp     eax, edx
        pop     eax
        jnz     .doit
        cmp     al, 0xCD
        jz      .int
        cmp     ax, 0x050F
        jz      .syscall
        cmp     ax, 0x340F
        jz      .sysenter

    ; resume process
    .doit:
        call    GoOn
        cmp     [bAfterGo], 0
        jz      @f
        mov     [bAfterGo], 2

    @@:
        ret

    ; return address is [ebp-4]
    .sysenter:
        push    0
        ;push    69
        ;pop     eax
        inc     edx     ; read 4 bytes
        mov     esi, [_ebp]
        sub     esi, 4
        mcall    69
        cmp     eax, edx
        pop     eax
        jnz     .syscall
        push    eax
        and     byte [_eflags+1], not 1
        call    set_context
        pop     eax
        jmp     @f

    .syscall:
        and     byte [_eflags+1], not 1 ; clear TF - avoid system halt (!)
        call    set_context

    .int:
        mov     eax, [_eip]
        inc     eax
        inc     eax

    @@:
        push    eax
        call    find_enabled_breakpoint
        pop     eax
        jz      .doit
    ; there is no enabled breakpoint yet; set temporary breakpoint
        mov     bl, 5
        call    add_breakpoint
        jmp     .doit

    .running:
        mov     esi, aRunningErr
        jmp     put_message

;-----------------------------------------------------------------------------
;                       Proceed process event

OnProceed:
        cmp     [bSuspended], 0
        jz      OnStep.running
        cmp     [proc_num], 0
        jg      .procone
        mov     esi, [curarg]
        cmp     esi, 0
        jz      .procone
        cmp     byte [esi], 0
        jz      .procone
        call    get_hex_number
        jc      .ret
        cmp     eax, 0 ; check if lesser than 0
        jle     .ret
        mov     [proc_num], eax
        mov     [curarg], 0

    .procone:
        mov     esi, [_eip]

    @@:
        call    get_byte_nobreak
        jc      OnStep
        inc     esi
    ; skip prefixes
        call    is_prefix
        jz      @b
        cmp     al, 0xE8        ; call
        jnz     @f
        add     esi, 4
        jmp     .doit

    ; A4,A5 = movs; A6,A7 = cmps
    @@:
        cmp     al, 0xA4
        jb      @f
        cmp     al, 0xA8
        jb      .doit

    ; AA,AB = stos; AC,AD = lods; AE,AF = scas
    @@:
        cmp     al, 0xAA
        jb      @f
        cmp     al, 0xB0
        jb      .doit

    ; E0 = loopnz; E1 = loopz; E2 = loop
    @@:
        cmp     al, 0xE0
        jb      .noloop
        cmp     al, 0xE2
        ja      .noloop
        inc     esi
        jmp     .doit

    ; FF /2 = call
    .noloop:
        cmp     al, 0xFF
        jnz     OnStep
        call    get_byte_nobreak
        jc      OnStep
        inc     esi
        mov     cl, al
        and     al, 00111000b
        cmp     al, 00010000b
        jnz     OnStep
    ; skip instruction
        mov     al, cl
        and     eax, 7
        shr     cl, 6
        jz      .mod0
        jp      .doit
        cmp     al, 4
        jnz     @f
        inc     esi

    @@:
        inc     esi
        dec     cl
        jz      @f
        add     esi, 3

    @@:
        jmp     .doit

    .mod0:
        cmp     al, 4
        jnz     @f
        call    get_byte_nobreak
        jc      OnStep
        inc     esi
        and     al, 7

    @@:
        cmp     al, 5
        jnz     .doit
        add     esi, 4

    .doit:
    ; insert one-shot breakpoint at esi and resume
        call    get_byte_nobreak
        jc      OnStep
        mov     eax, esi
        call    find_enabled_breakpoint
        jz      @f
        mov     eax, esi
        mov     bl, 5
        call    add_breakpoint
        jmp     OnStep.doit

    @@:
        mov     eax, [proc_num]
        dec     eax
        cmp     eax, 0
        jle     .ret
        mov     [proc_num], eax
        jmp     .procone

    .ret:
        mov     [proc_num], 0
        ret

;-----------------------------------------------------------------------------
;                        Read next byte of machine code

get_byte_nobreak:
        mov     eax, esi
        call    find_enabled_breakpoint
        jnz     .nobreak
        mov     al, [edi+5]
        clc
        ret
    
    .nobreak:
        ;push    69
        ;pop     eax
        ;push    6
        ;pop     ebx
        ;mov     ecx, [debuggee_pid]
        xor     edx, edx
        push    edx
        inc     edx
        mov     edi, esp
        mcall    69, 6, [debuggee_pid]
        dec     eax
        clc
        jz      @f
        stc
    
    @@:
        pop     eax
        ret

include 'parser.inc'

;-----------------------------------------------------------------------------
;                        Calculate expression event

OnCalc:
        mov     esi, [curarg]
        call    calc_expression
        jc      .ret
        push    ebp
        mov     esi, calc_string
        call    put_message_nodraw
        jmp     draw_messages
    
    .ret:
        ret

;-----------------------------------------------------------------------------
;                            Dump memory event

OnDump:
        mov     esi, [curarg]
        cmp     byte [esi], 0
        jnz     .param
        add     [dumppos], dump_height*10h
        jmp     .doit
    
    .param:
        call    calc_expression
        jc      .ret
        mov     [dumppos], ebp

    .doit:
        call    get_dump
        call    draw_dump.redraw

    .ret:
        ret

;-----------------------------------------------------------------------------
;                   Dissassemble block of executable event

OnUnassemble:
        mov     esi, [curarg]
        cmp     byte [esi], 0
        jnz     .param
        mov     eax, [disasm_start_pos]
        mov     ecx, disasm_height
        mov     [disasm_cur_pos], eax

    .l:
        mov     eax, [disasm_cur_pos]
        call    find_symbol
        jc      @f
        dec     ecx
        jz      .m

    @@:
        push    ecx
        call    disasm_instr
        pop     ecx
        jc      .err
        loop    .l

    .m:
        mov     eax, [disasm_cur_pos]
        jmp     .doit

    .param:
        call    calc_expression
        jc      .ret
        mov     eax, ebp

    .doit:
        push    eax
        push    [disasm_start_pos]
        mov     [disasm_start_pos], eax
        call    update_disasm
        pop     [disasm_start_pos]
        pop     eax
        cmp     [disasm_cur_str], 0
        jz      @f
        mov     [disasm_start_pos], eax

    .ret:
        ret

    @@:
        call    update_disasm

    .err:
        mov     esi, aInvAddr
        jmp     put_message

;-----------------------------------------------------------------------------
;                       Access to register value event

OnReg:
        mov     esi, [curarg]
        call    get_arg.skip_spaces
        call    find_reg
        jnc     @f

    .err:
        mov     esi, RSyntax
        jmp     put_message

    @@:
        call    get_arg.skip_spaces
        test    al, al
        jz      .err
        cmp     al, '='
        jnz     @f
        inc     esi
        call    get_arg.skip_spaces
        test    al, al
        jz      .err

    @@:
        push    edi
        call    calc_expression
        pop     edi
        jc      .ret
    ; now edi=register id, ebp=value
        cmp     [bSuspended], 0
        mov     esi, aRunningErr
        jz      put_message
        xchg    eax, ebp
        cmp     edi, 24
        jz      .eip
        sub     edi, 4
        jb      .8lo
        sub     edi, 4
        jb      .8hi
        sub     edi, 8
        jb      .16
        mov     [_eax+edi*4], eax
        jmp     .ret

    .16:
        mov     word [_eax+(edi+8)*4], ax
        jmp     .ret

    .8lo:
        mov     byte [_eax+(edi+4)*4], al
        jmp     .ret

    .8hi:
        mov     byte [_eax+(edi+4)*4+1], al
        jmp     .ret

    .eip:
        mov     [_eip], eax
        call    update_disasm_eip

    .ret:
        call    set_context
        jmp     draw_registers.redraw

;-----------------------------------------------------------------------------
;                        Breakpoints manipulation
OnBp:
        mov     esi, [curarg]
        call    calc_expression
        jc      .ret
        xchg    eax, ebp
        push    eax
        call    find_breakpoint
        inc     eax
        pop     eax
        jz      .notfound
        mov     esi, aDuplicateBreakpoint
        jmp     .sayerr

    .notfound:
        mov     bl, 1
        call    add_breakpoint
        jnc     .ret
        mov     esi, aBreakpointLimitExceeded

    .sayerr:
        call    put_message

    .ret:
        jmp     draw_disasm.redraw

OnBpmb:
        mov     dh, 0011b
        jmp     DoBpm

OnBpmw:
        mov     dh, 0111b
        jmp     DoBpm

OnBpmd:
        mov     dh, 1111b

DoBpm:
        mov     esi, [curarg]
        cmp     byte [esi], 'w'
        jnz     @f
        and     dh, not 2
        inc     esi

    @@:
        push    edx
        call    calc_expression
        pop     edx
        jnc     @f
        ret

    ; ebp = expression, dh = flags
    @@:
        movzx   eax, dh
        shr     eax, 2
        test    ebp, eax
        jz      @f
        mov     esi, aUnaligned
        jmp     put_message

    @@:
        mov     eax, ebp
        mov     bl, 0Bh
        call    add_breakpoint
        jnc     @f
        mov     esi, aBreakpointLimitExceeded
        jmp     put_message

    ; now find index
    @@:
        push    eax
        xor     ecx, ecx

    .l1:
        cmp     [drx_break+ecx*4], 0
        jnz     .l2
        ;push    69
        ;pop     eax
        push    ecx
        mov     dl, cl
        ;mov     ecx, [debuggee_pid]
        mov     esi, ebp
        ;push    9
        ;pop     ebx
        mcall    69, 9, [debuggee_pid]
        test    eax, eax
        jz      .ok
        pop     ecx

    .l2:
        inc     ecx
        cmp     ecx, 4
        jb      .l1
        pop     eax
        call    clear_breakpoint
        mov     esi, aBreakpointLimitExceeded
        jmp     put_message

    .ok:
        pop     ecx
        pop     eax
        and     byte [edi], not 2       ; breakpoint is enabled
        shl     dl, 6
        or      dl, dh
        mov     byte [edi+1], dl
        inc     eax
        mov     [drx_break+ecx*4], eax
        ret

OnBc:
        mov     esi, [curarg]

    @@:
        call    get_hex_number
        jc      OnBp.ret
        call    clear_breakpoint
        jmp     @b

OnBd:
        mov     esi, [curarg]

    @@:
        call    get_hex_number
        jc      OnBp.ret
        call    disable_breakpoint
        jmp     @b

OnBe:
        mov     esi, [curarg]

    @@:
        call    get_hex_number
        jc      OnBp.ret
        push    eax
        call    find_enabled_breakpoint
        pop     eax
        jz      .err
        call    enable_breakpoint
        jmp     @b

    .err:
        mov     esi, OnBeErrMsg
        jmp     put_message

; TODO: split it out in parser.inc
get_hex_number:
        call    get_arg.skip_spaces
        xor     ecx, ecx
        xor     edx, edx

    @@:
        lodsb
        call    is_hex_digit
        jc      .ret
        shl     edx, 4
        or      dl, al
        inc     ecx
        jmp     @b

    .ret:
        dec     esi
        cmp     ecx, 1
        xchg    eax, edx
        ret

;-----------------------------------------------------------------------------
;                       Breakpoints list event

OnBl:
        mov     esi, [curarg]
        cmp     byte [esi], 0
        jz      .listall
        call    get_hex_number
        jc      .ret
        cmp     eax, breakpoints_n
        jae     .err
        push    eax
        add     eax, eax
        lea     edi, [breakpoints + eax + eax*2]
        pop     eax
        test    byte [edi+4], 1
        jz      .err
        call    show_break_info

    .ret:
        ret

    .err:
        mov     esi, aInvalidBreak
        jmp     put_message

    .listall:
        mov     edi, breakpoints
        xor     eax, eax

    @@:
        test    byte [edi+4], 1
        jz      .cont
        push    edi eax
        call    show_break_info
        pop     eax edi

    .cont:
        add     edi, 6
        inc     eax
        cmp     eax, breakpoints_n
        jb      @b
        ret

;-----------------------------------------------------------------------------
                              
show_break_info:
        push    edi
        test    byte [edi+4], 8
        jnz     .dr
        push    dword [edi]
        push    eax
        mov     esi, aBreakNum
        call    put_message_nodraw
        jmp     .cmn

    .dr:
        push    eax
        mov     esi, aMemBreak1
        call    put_message_nodraw
        pop     edi
        push    edi
        mov     esi, aMemBreak2
        test    byte [edi+5], 2
        jz      @f
        mov     esi, aMemBreak3

    @@:
        call    put_message_nodraw
        pop     edi
        push    edi
        mov     esi, aMemBreak6
        test    byte [edi+5], 8
        jnz     @f
        mov     esi, aMemBreak5
        test    byte [edi+5], 4
        jnz     @f
        mov     esi, aMemBreak4

    @@:
        call    put_message_nodraw
        pop     edi
        push    edi
        push    dword [edi]
        mov     esi, aMemBreak7
        call    put_message_nodraw

    .cmn:
        pop     edi
        test    byte [edi+4], 2
        jz      @f
        push    edi
        mov     esi, aDisabled
        call    put_message_nodraw
        pop     edi

    @@:
        test    byte [edi+4], 4
        jz      @f
        mov     esi, aOneShot
        call    put_message_nodraw

    @@:
        mov     esi, newline
        jmp     put_message

;-----------------------------------------------------------------------------
;                        Add breakpoint
; in: EAX = address; BL = flags
; out: CF = 1 => error
;      CF = 0 and EAX = breakpoint number

add_breakpoint:
        xor     ecx, ecx
        mov     edi, breakpoints

    @@:
        test    byte [edi+4], 1
        jz      .found
        add     edi, 6
        inc     ecx
        cmp     ecx, breakpoints_n
        jb      @b
        stc
        ret

    .found:
        stosd
        xchg    eax, ecx
        mov     [edi], bl
        test    bl, 2
        jnz     @f
        or      byte [edi], 2
        push    eax
        call    enable_breakpoint
        pop     eax

    @@:
        clc
        ret

;-----------------------------------------------------------------------------
;                         Remove breakpoint

clear_breakpoint:
        cmp     eax, breakpoints_n
        jae     .ret
        mov     ecx, 4
        inc     eax

    .1:
        cmp     [drx_break-4+ecx*4], eax
        jnz     @f
        and     [drx_break-4+ecx*4], 0

    @@:
        loop    .1
        dec     eax
        push    eax
        add     eax, eax
        lea     edi, [breakpoints + eax + eax*2 + 4]
        test    byte [edi], 1
        pop     eax
        jz      .ret
        push    edi
        call    disable_breakpoint
        pop     edi
        mov     byte [edi], 0

    .ret:
        ret

;-----------------------------------------------------------------------------
;                          Disable breakpoint

disable_breakpoint:
        cmp     eax, breakpoints_n
        jae     .ret
        add     eax, eax
        lea     edi, [breakpoints + eax + eax*2 + 5]
        test    byte [edi-1], 1
        jz      .ret
        test    byte [edi-1], 2
        jnz     .ret
        or      byte [edi-1], 2
        test    byte [edi-1], 8
        jnz     .dr
        push    esi
        ;push    7
        ;pop     ebx
        ;push    69
        ;pop     eax
        ;mov     ecx, [debuggee_pid]
        ;xor     edx, edx
        ;inc     edx
        ;mov     esi, [edi-5]
        mcall    69, 7, [debuggee_pid], 1, [edi-5]
        pop     esi

    .ret:
        ret

    .dr:
        mov     dl, [edi]
        shr     dl, 6
        mov     dh, 80h
        ;push    69
        ;pop     eax
        ;push    9
        ;pop     ebx
        ;mov     ecx, [debuggee_pid]
        mcall    69, 9, [debuggee_pid]
        ret

;-----------------------------------------------------------------------------
;                           Enable breakpoint

enable_breakpoint:
        push    esi
        cmp     eax, breakpoints_n
        jae     .ret
        add     eax, eax
        lea     edi, [breakpoints + eax + eax*2 + 5]
        test    byte [edi-1], 1
        jz      .ret
        test    byte [edi-1], 2
        jz      .ret
        and     byte [edi-1], not 2
        test    byte [edi-1], 8
        jnz     .dr
        ;push    6
        ;pop     ebx
        ;push    69
        ;pop     eax
        ;mov     esi, [edi-5]
        ;mov     ecx, [debuggee_pid]
        ;xor     edx, edx
        ;inc     edx
        mcall    69, 6, [debuggee_pid], 1, [edi-5]
        dec     eax
        jnz     .err
        ;mov     al, 69
        push    0xCC
        mov     edi, esp
        inc     ebx
        mcall    69
        pop     eax

    .ret:
        pop     esi
        ret

    .err:
        or      byte [edi-1], 2
        mov     esi, aBreakErr
        call    put_message
        pop     esi
        ret

    .dr:
        ;push    9
        ;pop     ebx
        ;push    69
        ;pop     eax
        mov     esi, [edi-5]
        ;mov     ecx, [debuggee_pid]
        mov     dl, [edi]
        shr     dl, 6
        mov     dh, [edi]
        and     dh, 0xF
        mcall    69, 9, [debuggee_pid]
        test    eax, eax
        jnz     .err
        pop     esi
        ret

;-----------------------------------------------------------------------------
;                             Find breakpoint

find_breakpoint:
        xor     ecx, ecx
        xchg    eax, ecx
        mov     edi, breakpoints

    @@:
        test    byte [edi+4], 1
        jz      .cont
        test    byte [edi+4], 8
        jnz     .cont
        cmp     [edi], ecx
        jz      .found

    .cont:
        add     edi, 6
        inc     eax
        cmp     eax, breakpoints_n
        jb      @b
        or      eax, -1

    .found:
        ret

;-----------------------------------------------------------------------------
;

find_enabled_breakpoint:
        xor     ecx, ecx
        xchg    eax, ecx
        mov     edi, breakpoints

    @@:
        test    byte [edi+4], 1
        jz      .cont
        test    byte [edi+4], 2 or 8
        jnz     .cont
        cmp     [edi], ecx
        jz      .found

    .cont:
        add     edi, 6
        inc     eax
        cmp     eax, breakpoints_n
        jb      @b
        or      eax, -1

    .found:
        ret

; TODO: add find_disabled_breakpoint

;-----------------------------------------------------------------------------
;                       Unpack executable event

OnUnpack:
    ; program must be loaded - checked when command was parsed
    ; program must be stopped
        mov     esi, aRunningErr
        cmp     [bSuspended], 0
        jz      put_message
   ; all breakpoints must be disabled
        mov     edi, breakpoints

    @@:
        test    byte [edi+4], 1
        jz      .cont
        test    byte [edi+4], 2
        jnz     .cont
        mov     esi, aEnabledBreakErr
        jmp     put_message

    .cont:
        add     edi, 6
        cmp     edi, breakpoints+breakpoints_n*6
        jb      @b
    ; ok, now do it
    ; set breakpoint on 0xC dword access
        push    9
        pop     ebx
        mov     ecx, [debuggee_pid]
        mov     dx, 1111b*256
        push    0xC
        pop     esi

    @@:
        ;push    69
        ;pop     eax
        mcall    69
        test    eax, eax
        jz      .breakok
        inc     edx
        cmp     dl, 4
        jb      @b

    .breakok:
        call    GoOn

    ; now wait for event
    .wait:
        ;push    10
        ;pop     eax
        mcall    10
        dec     eax
        jz      .redraw
        dec     eax
        jz      .key
        dec     eax
        jnz     .debug
    ; button; we have only one button, close
        or      eax, -1
        mcall

    .redraw:
        call    draw_window
        jmp     .wait

    .key:
        mov     al, 2
        mcall
        cmp     ah, 3   ; Ctrl+C
        jnz     .wait

    .userbreak:
        mov     esi, aInterrupted

    .x1:
        push    edx esi
        call    put_message
        pop     esi edx
        or      dh, 80h
        ;push    69
        ;pop     eax
        ;push    9
        ;pop     ebx
        ;mov     ecx, [debuggee_pid]
        mcall    69, 9, [debuggee_pid]
        cmp     esi, aUnpacked
        jnz     OnSuspend
        jmp     AfterSuspend

    .debug:
        cmp     [dbgbuflen], 4*3
        jnz     .notour
        cmp     dword [dbgbuf], 3
        jnz     .notour
        test    byte [dbgbuf+8], 1
        jnz     .our

    .notour:
        mov     esi, aInterrupted
        push    edx
        call    put_message
        pop     edx
        or      dh, 80h
        ;push    69
        ;pop     eax
        ;push    9
        ;pop     ebx
        ;mov     ecx, [debuggee_pid]
        mcall    69, 9, [debuggee_pid]
        jmp     debugmsg

    .our:
        and     [dbgbuflen], 0
        push    edx
        call    get_context
        push    eax
        ;mov     al, 69
        ;mov     bl, 6
        ;mov     ecx, [debuggee_pid]
        ;mov     edi, esp
        ;push    4
        ;pop     edx
        ;push    0xC
        ;pop     esi
        mcall    69, 6, [debuggee_pid], 4, 0xC, esp
        pop     eax
        pop     edx
        cmp     eax, [_eip]
        jz      .done
        call    DoResume
        jmp     .wait

    .done:
        mov     esi, aUnpacked
        jmp     .x1

;-----------------------------------------------------------------------------
;                  Working with program symbols
;
;  TODO: split to symbols.inc

include 'sort.inc'

; compare what? Add context-relative comment and name
compare:
        cmpsd
        jnz     @f
        cmp     esi, edi

    @@:
        ret

; purpose of this function?
compare2:
        cmpsd

    @@:
        cmpsb
        jnz     @f
        cmp     byte [esi-1], 0
        jnz     @b
        cmp     esi, edi

    @@:
        ret

free_symbols:
        mov     ecx, [symbols]
        jecxz   @f
        mcall   68, 13
        and     [symbols], 0
        and     [num_symbols], 0

    @@:
        ret
;-----------------------------------------------------------------------------
;                        Load symbols event

OnLoadSymbols.fileerr:
        test    ebp, ebp
        jz      @f
        mcall   68, 13, edi
        ret

    @@:
        push    eax
        mcall   68, 13, edi
        mov     esi, aCannotLoadFile
        call    put_message_nodraw
        pop     eax
        cmp     eax, 0x20
        jae     .unk
        mov     esi, [load_err_msgs + eax*4]
        test    esi, esi
        jnz     put_message

    .unk:
        mov     esi, unk_err_msg2
        jmp     put_message

OnLoadSymbols:
        xor     ebp, ebp
   ; load input file
        mov     esi, [curarg]
        call    free_symbols

    .silent:
        xor     edi, edi
        cmp     [num_symbols], edi
        jz      @f
                                             
        call    free_symbols
        ;ret                                        
  
    @@:
        mov     ebx, fn70_attr_block
        mov     [ebx+21], esi
        mcall   70
        test    eax, eax
        jnz     .fileerr
        cmp     dword [fileattr+36], edi
        jnz     .memerr
        mov     ecx, dword [fileattr+32]
        mcall   68, 12
        test    eax, eax
        jz      .memerr
        mov     edi, eax
        mov     ebx, fn70_read_block
        mov     [ebx+12], ecx
        mov     [ebx+16], edi
        mov     [ebx+21], esi
        mcall   70
        test    eax, eax
        jnz     .fileerr
    ; calculate memory requirements
        lea     edx, [ecx+edi-1]        ; edx = EOF-1
        mov     esi, edi
        xor     ecx, ecx

    .calcloop:
        cmp     esi, edx
        jae     .calcdone
        cmp     word [esi], '0x'
        jnz     .skipline
        inc     esi
        inc     esi

    @@:
        cmp     esi, edx
        jae     .calcdone
        lodsb
        or      al, 20h
        sub     al, '0'
        cmp     al, 9
        jbe     @b
        sub     al, 'a'-'0'-10
        cmp     al, 15
        jbe     @b
        dec     esi

    @@:
        cmp     esi, edx
        ja      .calcdone
        lodsb
        cmp     al, 20h
        jz      @b
        jb      .calcloop
        cmp     al, 9
        jz      @b
        add     ecx, 12+1
        inc     [num_symbols]

    @@:
        inc     ecx
        cmp     esi, edx
        ja      .calcdone
        lodsb
        cmp     al, 0xD
        jz      .calcloop
        cmp     al, 0xA
        jz      .calcloop
        jmp     @b

    .skipline:
        cmp     esi, edx
        jae     .calcdone
        lodsb
        cmp     al, 0xD
        jz      .calcloop
        cmp     al, 0xA
        jz      .calcloop
        jmp     .skipline

    .calcdone:
        mcall   68, 12
        test    eax, eax
        jnz     .memok
        inc     ebx
        mov     ecx, edi
        mov     al, 68
        mcall

    .memerr:
        mov     esi, aNoMemory
        jmp     put_message

    .memok:
        mov     [symbols], eax
        mov     ebx, eax
        push    edi
        mov     esi, edi
        mov     edi, [num_symbols]
        lea     ebp, [eax+edi*4]
        lea     edi, [eax+edi*8]

    ; parse input data, 
    ; esi->input, edx->EOF, ebx->ptrs, edi->names
    .readloop:
        cmp     esi, edx
        jae     .readdone
        cmp     word [esi], '0x'
        jnz     .readline
        inc     esi
        inc     esi
        xor     eax, eax
        xor     ecx, ecx

    @@:
        shl     ecx, 4
        add     ecx, eax
        cmp     esi, edx
        jae     .readdone
        lodsb
        or      al, 20h
        sub     al, '0'
        cmp     al, 9
        jbe     @b
        sub     al, 'a'-'0'-10
        cmp     al, 15
        jbe     @b
        dec     esi

    @@:
        cmp     esi, edx
        ja      .readdone
        lodsb
        cmp     al, 20h
        jz      @b
        jb      .readloop
        cmp     al, 9
        jz      @b
        mov     dword [ebx], edi
        add     ebx, 4
        mov     dword [ebp], edi
        add     ebp, 4
        mov     dword [edi], ecx
        add     edi, 4
        stosb

    @@:
        xor     eax, eax
        stosb
        cmp     esi, edx
        ja      .readdone
        lodsb
        cmp     al, 0xD
        jz      .readloop
        cmp     al, 0xA
        jz      .readloop
        mov     byte [edi-1], al
        jmp     @b

    .readline:
        cmp     esi, edx
        jae     .readdone
        lodsb
        cmp     al, 0xD
        jz      .readloop
        cmp     al, 0xA
        jz      .readloop
        jmp     .readline

    .readdone:
        pop     ecx
        mcall   68, 13
        mov     ecx, [num_symbols]
        mov     edx, [symbols]
        mov     ebx, compare
        call    sort
        mov     ecx, [num_symbols]
        lea     edx, [edx+ecx*4]
        mov     ebx, compare2
        call    sort
        mov     esi, aSymbolsLoaded
        call    put_message
        jmp     draw_disasm.redraw

;-----------------------------------------------------------------------------
;
; in: EAX = address
; out: ESI, CF

find_symbol:
        cmp     [num_symbols], 0
        jnz     @f

    .ret0:
        xor     esi, esi
        stc
        ret

    @@:
        push    ebx ecx edx
        xor     edx, edx
        mov     esi, [symbols]
        mov     ecx, [num_symbols]
        mov     ebx, [esi]
        cmp     [ebx], eax
        jz      .donez
        jb      @f
        pop     edx ecx ebx
        jmp     .ret0

    @@:
    ; invariant: symbols_addr[edx] < eax < symbols_addr[ecx]
    ; TODO: add meaningful label names
    .0:
        push    edx

    .1:
        add     edx, ecx
        sar     edx, 1
        cmp     edx, [esp]
        jz      .done2
        mov     ebx, [esi+edx*4]
        cmp     [ebx], eax
        jz      .done
        ja      .2
        mov     [esp], edx
        jmp     .1

    .2:
        mov     ecx, edx
        pop     edx
        jmp     .0

    .donecont:
        dec     edx

    .done:
        test    edx, edx
        jz      @f
        mov     ebx, [esi+edx*4-4]
        cmp     [ebx], eax
        jz      .donecont

    @@:
        pop     ecx

    .donez:
        mov     esi, [esi+edx*4]
        add     esi, 4
        pop     edx ecx ebx
        clc
        ret

    .done2:
        lea     esi, [esi+edx*4]
        pop     ecx edx ecx ebx
        stc
        ret

;-----------------------------------------------------------------------------
;
; in: esi->name
; out: if found: CF = 0, EAX = value
;      otherwise CF = 1
find_symbol_name:
        cmp     [num_symbols], 0
        jnz     @f

    .stc_ret:
        stc
        ret

    @@:
        push    ebx ecx edx edi
        push    -1
        pop     edx
        mov     ebx, [symbols]
        mov     ecx, [num_symbols]
        lea     ebx, [ebx+ecx*4]
    
    ; invariant: symbols_name[edx] < name < symbols_name[ecx]
    .0:
        push    edx

    .1:
        add     edx, ecx
        sar     edx, 1
        cmp     edx, [esp]
        jz      .done2
        call    .cmp
        jz      .done
        jb      .2
        mov     [esp], edx
        jmp     .1

    .2:
        mov     ecx, edx
        pop     edx
        jmp     .0

    .done:
        pop     ecx

    .donez:
        mov     eax, [ebx+edx*4]
        mov     eax, [eax]
        pop     edi edx ecx ebx
        clc
        ret

    .done2:
        pop     edx edi edx ecx ebx
        stc
        ret

    .cmp:
        mov     edi, [ebx+edx*4]
        push    esi
        add     edi, 4

    @@:
        cmpsb
        jnz     @f
        cmp     byte [esi-1], 0
        jnz     @b

    @@:
        pop     esi
        ret

;-----------------------------------------------------------------------------
;                        Include disassembler engine

include 'disasm.inc'

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; DATA ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

caption_str db 'Kolibri Debugger',0
caption_len = $ - caption_str

begin_str db    'Kolibri Debugger, version 0.33',10
        db      'Hint: type "help" for help, "quit" for quit'
newline db      10,0
prompt  db      '> ',0

help_groups:
        dd      aControl, 0, 0, help_control_msg
        db      0
        dd      aData, 0, 0, help_data_msg
        db      0
        dd      aBreakpoints, 0, 0, help_breaks_msg
        db      0

;-----------------------------------------------------------------------------
;                   Commands format definitions

; TODO: make it with macros

; flags field:
; &1: command may be called without parameters
; &2: command may be called with parameters
; &4: command may be called without loaded program
; &8: command may be called with loaded program
commands:
        dd      _aH, OnHelp, HelpSyntax, HelpHelp
        db      0Fh
        dd      aHelp, OnHelp, HelpSyntax, HelpHelp
        db      0Fh
        dd      aQuit, OnQuit, QuitSyntax, QuitHelp
        db      0Dh
        dd      aLoad, OnLoad, LoadSyntax, LoadHelp
        db      6
        dd      aReload, OnReload, ReloadSyntax, ReloadHelp
        db      0Dh
        dd      aTerminate, OnTerminate, TerminateSyntax, TerminateHelp
        db      9
        dd      aDetach, OnDetach, DetachSyntax, DetachHelp
        db      9
        dd      aSuspend, OnSuspend, SuspendSyntax, SuspendHelp
        db      9
        dd      aResume, OnResume, ResumeSyntax, ResumeHelp
        db      0Bh
        dd      aStep, OnStepMultiple, StepSyntax, StepHelp
        db      0Bh
        dd      aProceed, OnProceed, ProceedSyntax, ProceedHelp
        db      0Bh
        dd      aCalc, OnCalc, CalcSyntax, CalcHelp
        db      0Eh
        dd      aDump, OnDump, DumpSyntax, DumpHelp
        db      0Bh
        dd      aUnassemble, OnUnassemble, UnassembleSyntax, UnassembleHelp
        db      0Bh
        dd      aBp, OnBp, BpSyntax, BpHelp
        db      0Ah
        dd      aBpm, OnBpmb, BpmSyntax, BpmHelp
        db      0Ah
        dd      aBpmb, OnBpmb, BpmSyntax, BpmHelp
        db      0Ah
        dd      aBpmw, OnBpmw, BpmSyntax, BpmHelp
        db      0Ah
        dd      aBpmd, OnBpmd, BpmSyntax, BpmHelp
        db      0Ah
        dd      aBl, OnBl, BlSyntax, BlHelp
        db      0Bh
        dd      aBc, OnBc, BcSyntax, BcHelp
        db      0Ah
        dd      aBd, OnBd, BdSyntax, BdHelp
        db      0Ah
        dd      aBe, OnBe, BeSyntax, BeHelp
        db      0Ah
        dd      aReg, OnReg, RSyntax, RHelp
        db      0Ah
        dd      aUnpack, OnUnpack, UnpackSyntax, UnpackHelp
        db      9
        dd      aLoadSymbols, OnLoadSymbols, LoadSymbolsSyntax, LoadSymbolsHelp
        db      0Ah
        dd      0

;-----------------------------------------------------------------------------
;                   Help messages for commands groups

aHelp   db      5,'help',0
_aH     db      2,'h',0
HelpHelp db     'Help on specified function',10
HelpSyntax db   'Usage: h or help [group | command]',10,0

help_msg db     'List of known command groups:',10
        db      '"help control"     - display list of control commands',10
        db      '"help data"        - display list of commands concerning data',10
        db      '"help breakpoints" - display list of commands concerning breakpoints',10,0

;               Control commands group

aControl db     8,'control',0
help_control_msg db     'List of control commands:',10
        db      'h = help             - help',10
        db      'quit                 - exit from debugger',10
        db      'load <name> [params] - load program for debugging',10
        db      'reload               - reload debugging program',10
        db      'load-symbols <name>  - load information on symbols for program',10
        db      'terminate            - terminate loaded program',10
        db      'detach               - detach from debugging program',10
        db      'stop                 - suspend execution of debugging program',10
        db      'g [<expression>]     - go on (resume execution of debugging program)',10
        db      's [<num>]            - program step, also <Ctrl+F7>',10
        db      'p [<num>]            - program wide step, also <Ctrl+F8>',10
        db      'unpack               - try to bypass unpacker code (heuristic)',10,0

;               Data commands group

aData   db      5,'data',0
help_data_msg db        'List of data commands:',10
        db      '? <expression>       - calculate value of expression',10
        db      'd [<expression>]     - dump data at given address',10
        db      'u [<expression>]     - unassemble instructions at given address',10
        db      'r <register> <expression> or',10
        db      'r <register>=<expression> - set register value',10,0
    
;               Breakpoints commands group

aBreakpoints db 12,'breakpoints',0
help_breaks_msg db      'List of breakpoints commands:',10
        db      'bp <expression>      - set breakpoint on execution',10
        db      'bpm[b|w|d] <type> <expression> - set breakpoint on memory access',10
        db      'bl [<number>]        - breakpoint(s) info',10
        db      'bc <number>...       - clear breakpoint',10
        db      'bd <number>...       - disable breakpoint',10
        db      'be <number>...       - enable breakpoint',10,0

;-----------------------------------------------------------------------------
;                    Individual command help messages

aQuit   db      5,'quit',0
QuitHelp db     'Quit from debugger',10
QuitSyntax db   'Usage: quit',10,0

aLoad   db      5,'load',0
LoadHelp db     'Load program for debugging',10
LoadSyntax db   'Usage: load <program-name> [parameters]',10,0

aReload db      7,'reload',0
ReloadHelp db   'Reload debugging program (restart debug session)',10
ReloadSyntax db 'Usage: reload',10,0

aTerminate db   10,'terminate',0
TerminateHelp db 'Terminate debugged program',10
TerminateSyntax db 'Usage: terminate',10,0

aDetach db      7,'detach',0
DetachHelp db   'Detach from debugged program',10
DetachSyntax db 'Usage: detach',10,0

aSuspend db     5,'stop',0
SuspendHelp db  'Suspend execution of debugged program',10
SuspendSyntax db 'Usage: stop',10,0

aResume db      2,'g',0
ResumeHelp db   'Go (resume execution of debugged program)',10
ResumeSyntax db 'Usage: g',10
        db      '   or: g <expression> - wait until specified address is reached',10,0

aStep   db      2,'s',0
StepHelp db     'Make step in debugged program',10
StepSyntax db   'Usage: s [<number>]',10,0

aProceed db     2,'p',0
ProceedHelp db  'Make wide step in debugged program (step over CALL, REPxx, LOOP)',10
ProceedSyntax db 'Usage: p [<number>]',10,0

aDump   db      2,'d',0
DumpHelp db     'Dump data of debugged program',10
DumpSyntax db   'Usage: d <expression> - dump data at specified address',10
        db      '   or: d              - continue current dump',10,0

aCalc   db      2,'?',0
CalcHelp db     'Calculate value of expression',10
CalcSyntax db   'Usage: ? <expression>',10,0

aUnassemble db  2,'u',0
UnassembleHelp db 'Unassemble',10
UnassembleSyntax db      'Usage: u <expression> - unassemble instructions at specified address',10
                 db      '   or: u              - continue current unassemble screen',10,0

aReg    db      2,'r',0
RHelp   db      'Set register value',10
RSyntax db      'Usage: r <register> <expression>',10
        db      '   or: r <register>=<expression> - set value of <register> to <expression>',10,0

aBp     db      3,'bp',0
BpHelp  db      'set BreakPoint on execution',10
BpSyntax db     'Usage: bp <expression>',10,0

aBpm    db      4,'bpm',0
aBpmb   db      5,'bpmb',0
aBpmw   db      5,'bpmw',0
aBpmd   db      5,'bpmd',0
BpmHelp db      'set BreakPoint on Memory access',10
        db      'Maximum 4 breakpoints of this type are allowed',10
        db      'Note that for this breaks debugger is activated after access',10
BpmSyntax db    'Usage: bpmb [w] <expression>',10
        db      '       bpmw [w] <expression>',10
        db      '       bpmd [w] <expression>',10
        db      '       bpm is synonym for bpmd',10
        db      '"w" means break only on writes (default is on read/write)',10,0

aBl     db      3,'bl',0
BlHelp  db      'Breakpoint List',10
BlSyntax db     'Usage: bl          - list all breakpoints',10
        db      '       bl <number> - display info on particular breakpoint',10,0

aBc     db      3,'bc',0
BcHelp  db      'Breakpoint Clear',10
BcSyntax db     'Usage: bc <number-list>',10
        db      'Examples: bc 2',10
        db      '          bc 1 3 4 A',10,0

aBd     db      3,'bd',0
BdHelp  db      'Breakpoint Disable',10
BdSyntax db     'Usage: bd <number-list>',10
        db      'Examples: bd 2',10
        db      '          bd 1 3 4 A',10,0

aBe     db      3,'be',0
BeHelp  db      'Breakpoint Enable',10
BeSyntax db     'Usage: be <number-list>',10
        db      'Examples: be 2',10
        db      '          be 1 3 4 A',10,0

aUnpack db      7,'unpack',0
UnpackHelp db   'Try to bypass unpacker code',10
UnpackSyntax db 'Usage: unpack',10,0

aLoadSymbols db 13,'load-symbols',0
LoadSymbolsHelp db 'Load symbolic information for executable',10
LoadSymbolsSyntax db 'Usage: load-symbols <symbols-file-name>',10,0

aUnknownCommand db 'Unknown command',10,0

;-----------------------------------------------------------------------------
;                             Error messages

load_err_msg    db      'Cannot load program. ',0
unk_err_msg     db      'Unknown error code -%4X',10,0
aCannotLoadFile db      'Cannot load file. ',0
unk_err_msg2    db      'Unknown error code %4X.',10,0
load_err_msgs:
        dd      .1, 0, .3, 0, .5, .6, 0, 0, .9, .A, 0, 0, 0, 0, 0, 0
        dd      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, .1E, .1F, .20
.1              db      'HD undefined.',10,0
.3              db      'Unknown FS.',10,0
.5              db      'File not found.',10,0
.6              db      'Unexpected EOF.',10,0
.9              db      'FAT table corrupted.',10,0
.A              db      'Access denied.',10,0
.1E             db      'No memory.',10,0
.1F             db      'Not Menuet/Kolibri executable.',10,0
.20             db      'Too many processes.',10,0
load_succ_msg   db      'Program loaded successfully! PID=%4X. Use "g" to run.',10,0
need_debuggee   db      'No program loaded. Use "load" command.',10,0
aAlreadyLoaded  db      'Program is already loaded. Use "terminate" or "detach" commands',10,0
terminated_msg  db      'Program terminated.',10,0
aException      db      'Debugged program caused an exception %2X. '
aSuspended      db      'Suspended',10,0
aContinued      db      'Continuing',10,0
aRunningErr     db      'Program is running',10,0
read_mem_err    db      'ERROR: cannot read process memory!!!',10,0
aBreakpointLimitExceeded db 'Breakpoint limit exceeded',10,0
aBreakErr       db      'Cannot activate breakpoint, it will be disabled',10,0
aDuplicateBreakpoint db 'Duplicate breakpoint',10,0
aInvalidBreak   db      'Invalid breakpoint number',10,0
OnBeErrMsg      db      'There is already enabled breakpoint on this address',10,0
aBreakNum       db      '%2X: at %8X',0
aMemBreak1      db      '%2X: on ',0
aMemBreak2      db      'read from ',0
aMemBreak3      db      'access of ',0
aMemBreak4      db      'byte',0
aMemBreak5      db      'word',0
aMemBreak6      db      'dword',0
aMemBreak7      db      ' at %8X',0
aOneShot        db      ', one-shot',0
aDisabled       db      ', disabled',0
aBreakStop      db      'Breakpoint #%2X',10,0
aUserBreak      db      'int3 command at %8X',10,0
;dbgmsg_str     db      'Debug message for process %4X.',10,0
aInvAddr        db      'Invalid address',10,0
NoPrgLoaded_str db      'No program loaded'
NoPrgLoaded_len = $ - NoPrgLoaded_str
aRunning        db      'Running'
aPaused         db      'Paused'
aMain            db        '[ CPU ]'
aSSE            db        '[ SSE ]'
aAVX            db        '[ AVX ]'
aMSR            db        '[ MSR ]'
aPoint          db      0x1C
aMinus          db      '-'
aColon          db      ':'
aSpace          db      ' '
aQuests         db      '??'
aDots           db      '...'
aParseError     db      'Parse error',10,0
aDivByZero      db      'Division by 0',10,0
calc_string     db      '%8X',10,0
aNoMemory       db      'No memory',10,0
aSymbolsLoaded  db      'Symbols loaded',10,0
aUnaligned      db      'Unaligned address',10,0
aEnabledBreakErr db     'Enabled breakpoints are not allowed',10,0
aInterrupted    db      'Interrupted',10,0
aUnpacked       db      'Unpacked successful!',10,0
aPacked1        db      'Program is probably packed with ',0
aPacked2        db      '.',10,'Try to unpack automatically? [y/n]: ',0
aY_str          db      'y',10,0
aN_str          db      'n',10,0
mxp_nrv_name    db      'mxp_nrv',0
mxp_name        db      'mxp',0
mxp_lzo_name    db      'mxp_lzo',0
mtappack_name   db      'mtappack',0
flags           db      'CPAZSDO'
flags_bits      db      0,2,4,6,7,10,11

;-----------------------------------------------------------------------------
;                         Registers strings

regs_strs:
        db      'EAX='
        db      'EBX='
        db      'ECX='
        db      'EDX='
        db      'ESI='
        db      'EDI='
        db      'EBP='
        db      'ESP='
        db      'EIP='
        db      'EFLAGS='
fpu_strs:
        db        'ST0='
        db        'ST1='
        db        'ST2='
        db        'ST3='
        db        'ST4='
        db        'ST5='
        db        'ST6='
        db        'ST7='
mmx_strs:
        db        'MM0='
        db        'MM1='
        db        'MM2='
        db        'MM3='
        db        'MM4='
        db        'MM5='
        db        'MM6='
        db        'MM7='
sse_strs:
        db        '-XMM0-'
        db      '-XMM1-'
        db      '-XMM2-'
        db      '-XMM3-'
        db      '-XMM4-'
        db      '-XMM5-'
        db      '-XMM6-'
        db      '-XMM7-'
avx_strs:
        db      '-YMM0-'
        db      '-YMM1-'
        db      '-YMM2-'
        db      '-YMM3-'
        db      '-YMM4-'
        db      '-YMM5-'
        db      '-YMM6-'
        db      '-YMM7-'

debuggee_pid    dd      0
bSuspended      db      0
bAfterGo        db      0
temp_break      dd      0
reg_mode        db        1

include 'disasm_tbl.inc'

reg_table:
        db      2,'al',0
        db      2,'cl',1
        db      2,'dl',2
        db      2,'bl',3
        db      2,'ah',4
        db      2,'ch',5
        db      2,'dh',6
        db      2,'bh',7
        db      2,'ax',8
        db      2,'cx',9
        db      2,'dx',10
        db      2,'bx',11
        db      2,'sp',12
        db      2,'bp',13
        db      2,'si',14
        db      2,'di',15
        db      3,'eax',16
        db      3,'ecx',17
        db      3,'edx',18
        db      3,'ebx',19
        db      3,'esp',20
        db      3,'ebp',21
        db      3,'esi',22
        db      3,'edi',23
        db      3,'eip',24
        db      0

IncludeIGlobals

fn70_read_block:
        dd      0
        dq      0
        dd      ?
        dd      ?
        db      0
        dd      ?

fn70_attr_block:
        dd      5
        dd      0,0,0
        dd      fileattr
        db      0
        dd      ?

fn70_load_block:
        dd      7
        dd      1
load_params dd  0
        dd      0
        dd      0
i_end:
loadname:
        db      0
        rb      255

symbolsfile     rb      260

prgname_ptr dd ?
prgname_len dd ?

IncludeUGlobals

dbgwnd          dd      ?

messages        rb      messages_height*messages_width
messages_pos    dd      ?

cmdline         rb      cmdline_width+1
cmdline_len     dd      ?
cmdline_pos     dd      ?
curarg          dd      ?

cmdline_prev    rb      cmdline_width+1

was_temp_break  db      ?

dbgbufsize      dd      ?
dbgbuflen       dd      ?
dbgbuf          rb      256

fileattr        rb      40

needzerostart:

context:

_eip    dd      ?
_eflags dd      ?
_eax    dd      ?
_ecx    dd      ?
_edx    dd      ?
_ebx    dd      ?
_esp    dd      ?
_ebp    dd      ?
_esi    dd      ?
_edi    dd      ?
oldcontext rb $-context

mmx_context:
_mm0    dq        ?
_mm1    dq        ?
_mm2    dq        ?
_mm3    dq        ?
_mm4    dq        ?
_mm5    dq        ?
_mm6    dq        ?
_mm7    dq        ?
oldmmxcontext rb $-mmx_context

fpu_context:
_st0    dq      ?
_st1    dq      ?
_st2    dq      ?
_st3    dq      ?
_st4    dq      ?
_st5    dq      ?
_st6    dq      ?
_st7    dq      ?
oldfpucontext rb $-fpu_context

sse_context:
_xmm0    dq        2 dup ?
_xmm1   dq        2 dup ?
_xmm2   dq      2 dup ?
_xmm3   dq      2 dup ?
_xmm4   dq      2 dup ?
_xmm5   dq      2 dup ?
_xmm6   dq      2 dup ?
_xmm7   dq      2 dup ?
oldssecontext rb $-sse_context

avx_context:
_ymm0   dq      4 dup ?
_ymm1   dq      4 dup ?
_ymm2   dq      4 dup ?
_ymm3   dq      4 dup ?
_ymm4   dq      4 dup ?
_ymm5   dq      4 dup ?
_ymm6   dq      4 dup ?
_ymm7   dq      4 dup ?
oldavxcontext rb $-avx_context

step_num dd 0
proc_num dd 0
dumpread dd     ?
dumppos dd      ?
dumpdata rb     dump_height*10h

; breakpoint structure:
; dword +0: address
; byte +4: flags
; bit 0: 1 <=> breakpoint valid
; bit 1: 1 <=> breakpoint disabled
; bit 2: 1 <=> one-shot breakpoint
; bit 3: 1 <=> DRx breakpoint
; byte +5: overwritten byte
;          for DRx breaks: flags + (index shl 6)
breakpoints_n = 256
breakpoints     rb      breakpoints_n*6
drx_break       rd      4

disasm_buf_size         dd      ?

symbols         dd      ?
num_symbols     dd      ?

bReload                 db      ?

needzeroend:

disasm_buffer           rb      256
disasm_start_pos        dd      ?
disasm_cur_pos          dd      ?
disasm_cur_str          dd      ?
disasm_string           rb      256

i_param         rb      256

; stack
        align   400h
        rb      400h
used_mem:

; vim: ft=fasm tabstop=4

