use32
        db      'MENUET01'
        dd      1
        dd      start
        dd      i_end
memsize dd      mem
        dd      stacktop
        dd      0, 0

include 'lang.inc'
include 'font.inc'
include 'sort.inc'
include 'kglobals.inc'
include 'memalloc.inc'
include 'dialogs.inc'
include 'viewer.inc'
include 'tools.inc'

start:
        mov     eax, mem
        call    mf_init
        call    init_console
        call    draw_window
        push    66
        pop     eax
        push    1
        pop     ebx
        mov     ecx, ebx
        int     40h     ; set keyboard mode to scancodes
        mov     esi, def_left_dir
        mov     edi, panel1_dir
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        mov     esi, def_right_dir
        mov     edi, panel2_dir
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        mov     eax, 200
        mov     [panel1_nfa], eax
        mov     [panel2_nfa], eax
        mov     eax, 200*4 + 32 + 200*304
        push    eax
        call    mf_alloc
        mov     [panel1_files], eax
        pop     eax
        call    mf_alloc
        mov     [panel2_files], eax
        test    eax, eax
        jz      exit
        cmp     [panel1_files], eax
        jz      exit
        mov     [panel1_sortmode], 0    ; sort by name
        mov     [panel2_sortmode], 0
        mov     [num_screens], 1
        mov     eax, 8
        call    mf_alloc
        mov     [screens], eax
        test    eax, eax
        jz      exit
        mov     ecx, panels_vtable
        mov     [eax], ecx
        mov     [active_screen_vtable], ecx
        call    draw_keybar
        call    draw_cmdbar
        mov     [prev_dir], 0
        mov     ebp, panel1_data
        call    read_folder
        call    draw_panel
        mov     [bSilentFolderMode], 1
        mov     ebp, panel2_data
        call    read_folder
        call    draw_panel
event:
        push    10
        pop     eax
        int     40h
        dec     eax
        jz      redraw
        dec     eax
        jz      key
; button - we have only one button, close
exit:
        or      eax, -1
        int     40h
redraw:
; query kbd state from OS
        mov     al, 66
        push    3
        pop     ebx
        int     0x40
        and     eax, 0x3F
        cmp     al, [ctrlstate]
        mov     [ctrlstate], al
        jz      @f
        call    draw_keybar
@@:
        mov     al, 9
        mov     ebx, procinfo
        or      ecx, -1
        int     40h
; test if rolled up
; height of rolled up window is [skinh]+3
        mov     eax, [ebx+46]
        sub     eax, [skinh]
        cmp     eax, 5
        ja      @f
        mov     al, 12
        push    1
        pop     ebx
        int     0x40
        xor     eax, eax
; ebx, ecx, edi are ignored by function 0 after first redraw
        mov     edx, 0x53000000
        int     0x40
        mov     al, 12
        inc     ebx
        int     0x40
        jmp     event
@@:
        xor     ecx, ecx
        mov     eax, [ebx+42]
        mov     [wnd_width], eax
        sub     eax, 5*2-1
        jae     @f
        xor     eax, eax
@@:
        cdq
        mov     esi, font_width
        div     esi
        cmp     eax, 54
        jae     @f
        mov     al, 54
        mov     ch, 1
@@:
        cmp     eax, 255
        jbe     @f
        mov     eax, 255
        mov     ch, 1
@@:
        cmp     eax, [cur_width]
        mov     [cur_width], eax
        setnz   cl
        or      cl, ch
        test    edx, edx
        mov     [fill_width], edx
        setnz   ch
        mov     eax, [ebx+46]
        mov     [wnd_height], eax
        sub     eax, [skinh]
        sub     eax, 5-1
        jns     @f
        xor     eax, eax
@@:
        cdq
        mov     esi, font_height
        div     esi
        cmp     eax, 8
        jae     @f
        mov     al, 8
        mov     cl, 1
@@:
        cmp     eax, 255
        jbe     @f
        mov     eax, 255
        mov     cl, 1
@@:
        mov     [fill_height], edx
        cmp     eax, [cur_height]
        mov     [cur_height], eax
        jnz     .resize
        test    cl, cl
        jnz     .resize
        test    edx, edx
        setnz   cl
        or      cl, ch
        jz      @f
        test    byte [ebx+70], 1
        jnz     @f
.resize:
        push    67
        pop     eax
        or      ebx, -1
        or      ecx, -1
        mov     edx, [cur_width]
        imul    edx, font_width
        add     edx, 5*2-1
        mov     esi, [cur_height]
        imul    esi, font_height
        add     esi, [skinh]
        add     esi, 5-1
        int     40h
.resize_draw:
        call    init_console
;        call    draw_window
        call    draw_keybar
        mov     ebp, [active_screen_data]
        mov     eax, [active_screen_vtable]
        call    dword [eax+screen_vtable.OnRedraw]
        jmp     event
@@:
        call    draw_window
        jmp     event
alt_f9:
        cmp     [saved_width], -1
        jz      @f
        mov     eax, [saved_width]
        mov     [cur_width], eax
        or      [saved_width], -1
        mov     eax, [saved_height]
        mov     [cur_height], eax
        or      [saved_height], -1
        jmp     redraw.resize
@@:
        push    48
        pop     eax
        push    5
        pop     ebx
        int     0x40
        push    eax
        sub     eax, [esp+2]
        inc     eax
        movzx   eax, ax
        sub     eax, 10
        xor     edx, edx
        mov     ecx, font_width
        div     ecx
        xchg    [cur_width], eax
        mov     [saved_width], eax
        mov     eax, ebx
        shr     ebx, 16
        sub     eax, ebx
        sub     eax, 5-1
        sub     eax, [skinh]
        xor     edx, edx
        mov     ecx, font_height
        div     ecx
        xchg    [cur_height], eax
        mov     [saved_height], eax
        mov     ecx, ebx
        pop     ebx
        shr     ebx, 16
        mov     edx, [cur_width]
        imul    edx, font_width
        add     edx, 5*2-1
        mov     esi, [cur_height]
        imul    esi, font_height
        add     esi, [skinh]
        add     esi, 4
        push    67
        pop     eax
        int     0x40
        jmp     redraw.resize_draw
key:
        mov     al, 2
        int     40h
        test    al, al
        jnz     event
        xchg    al, ah
        cmp     al, 0xE0
        jnz     @f
        mov     [bWasE0], 1
        jmp     event
@@:
        xchg    ah, [bWasE0]
        mov     ebp, [active_screen_data]
        mov     edx, [active_screen_vtable]
        cmp     al, 0x1D
        jz      .ctrl_down
        cmp     al, 0x9D
        jz      .ctrl_up
        cmp     al, 0x2A
        jz      .lshift_down
        cmp     al, 0xAA
        jz      .lshift_up
        cmp     al, 0x36
        jz      .rshift_down
        cmp     al, 0xB6
        jz      .rshift_up
        cmp     al, 0x38
        jz      .alt_down
        cmp     al, 0xB8
        jz      .alt_up
        call    [edx+screen_vtable.OnKey]
        jmp     event
.ctrl_down:
        test    ah, ah
        jnz     .rctrl_down
        or      [ctrlstate], 4
        jmp     .keybar
.rctrl_down:
        or      [ctrlstate], 8
        jmp     .keybar
.ctrl_up:
        test    ah, ah
        jnz     .rctrl_up
        and     [ctrlstate], not 4
        jmp     .keybar
.rctrl_up:
        and     [ctrlstate], not 8
.keybar:
        call    draw_keybar
        call    draw_image
@@:     jmp     event
.lshift_down:
; ignore E0 2A sequence
; e.g. arrow keys with NumLock on generate sequence E0 2A E0 xx / E0 xx+80 E0 AA
; where xx is scancode, so we can safely ignore E0 2A
        test    ah, ah
        jnz     @b
        or      [ctrlstate], 1
        jmp     .keybar
.lshift_up:
; ignore E0 AA sequence
        test    ah, ah
        jnz     @b
        and     [ctrlstate], not 1
        jmp     .keybar
.rshift_down:
        or      [ctrlstate], 2
        jmp     .keybar
.rshift_up:
        and     [ctrlstate], not 2
        jmp     .keybar
.alt_down:
        test    ah, ah
        jnz     .ralt_down
        or      [ctrlstate], 0x10
        jmp     .keybar
.ralt_down:
        or      [ctrlstate], 0x20
        jmp     .keybar
.alt_up:
        test    ah, ah
        jnz     .ralt_up
        and     [ctrlstate], not 0x10
        jmp     .keybar
.ralt_up:
        and     [ctrlstate], not 0x20
        jmp     .keybar

process_ctrl_keys:
        cmp     byte [esi], 0
        jz      .done
        push    ecx
        cmp     al, [esi]
        jz      .check
.cont:
        pop     ecx
        add     esi, 8
        jmp     process_ctrl_keys
.done:
        stc
        ret
.check:
        mov     cl, 0
        call    .check_ctrlkey
        jc      .cont
        mov     cl, 2
        call    .check_ctrlkey
        jc      .cont
        mov     cl, 4
        call    .check_ctrlkey
        jc      .cont
        pop     ecx
        call    dword [esi+4]
        clc
        ret
.check_ctrlkey:
        push    eax edx
        movzx   edx, [ctrlstate]
        shr     edx, cl
        add     cl, cl
        movzx   eax, word [esi+2]
        shr     eax, cl
        and     eax, 15
        cmp     al, ctrlkey_tests_num
        jae     .fail
        xchg    eax, edx
        and     al, 3
        call    [ctrlkey_tests + edx*4]
        cmp     al, 1
        pop     edx eax
        ret
.fail:
        stc
        pop     edx eax
        ret

ctrlkey_test0:
        test    al, al
        setz    al
        ret
ctrlkey_test1:
        test    al, al
        setnp   al
        ret
ctrlkey_test2:
        cmp     al, 3
        setz    al
        ret
ctrlkey_test3:
        cmp     al, 1
        setz    al
        ret
ctrlkey_test4:
        cmp     al, 2
        setz    al
        ret

new_screen:
        call    xmalloc
        test    eax, eax
        jnz     @f
        ret
@@:
        mov     ebp, eax
        mov     ebx, [num_screens]
        inc     ebx
        shl     ebx, 3
        mov     eax, [screens]
        call    xrealloc
        test    eax, eax
        jnz     @f
        mov     eax, ebp
        call    mf_free
        xor     eax, eax
        ret
@@:
        mov     [screens], eax
        inc     [num_screens]
        mov     [eax+ebx-8], edx
        mov     [eax+ebx-4], ebp
        mov     eax, [num_screens]
        dec     eax
        mov     [active_screen], eax
        mov     [active_screen_vtable], edx
        mov     [active_screen_data], ebp
        jmp     draw_keybar

next_screen:
        mov     eax, [active_screen]
        inc     eax
        cmp     eax, [num_screens]
        jnz     @f
        xor     eax, eax
@@:     mov     [active_screen], eax
        jmp     change_screen

delete_active_screen:
        mov     edi, [screens]
        mov     eax, [active_screen]
        shl     eax, 3
        add     edi, eax
        push    dword [edi+4]
        lea     esi, [edi+8]
        mov     ecx, [num_screens]
        sub     ecx, [active_screen]
        dec     ecx
        add     ecx, ecx
        rep     movsd
        dec     [num_screens]
        mov     ebx, [num_screens]
        shl     ebx, 3
        mov     eax, [screens]
        call    mf_realloc      ; must succeed, because we decrease size
        pop     eax
        call    mf_free
        and     [active_screen], 0

change_screen:
        pusha
        mov     eax, [active_screen]
        mov     esi, [screens]
        mov     ebp, [esi+eax*8+4]
        mov     eax, [esi+eax*8]
        mov     [active_screen_vtable], eax
        mov     [active_screen_data], ebp
        call    draw_keybar
        call    [eax+screen_vtable.OnRedraw]
        popa
        ret

F12:
        mov     eax, [cur_width]
        add     eax, 8
        mov     esi, eax
        mul     [num_screens]
        call    xmalloc
        test    eax, eax
        jnz     @f
        ret
@@:
        mov     ebx, eax
        mov     edi, eax
        xor     ecx, ecx
.next:
        xor     eax, eax
        stosd
        inc     ecx
        cmp     ecx, [num_screens]
        jz      @f
        lea     eax, [edi+esi-4]
        mov     dword [edi-4], eax
@@:
        xor     eax, eax
        stosd
        dec     ecx
        jz      @f
        lea     eax, [edi-8]
        sub     eax, esi
        mov     dword [edi-4], eax
@@:
        mov     al, '&'
        stosb
        cmp     ecx, 36
        jae     .noletter
        lea     eax, [ecx+'0']
        cmp     al, '9'
        jbe     @f
        add     al, 7
@@:
        stosb
        mov     al, '.'
        stosb
        jmp     .letter_done
.noletter:
        mov     al, ' '
        stosb
        stosb
.letter_done:
        mov     al, ' '
        stosb
        pushad
        mov     eax, [screens]
        mov     ebp, [eax+ecx*8+4]
        mov     eax, [eax+ecx*8]
        mov     ecx, [cur_width]
        sub     ecx, 12
        call    [eax + screen_vtable.getname]
        popad
        sub     edi, 4
        add     edi, [cur_width]
        inc     ecx
        cmp     ecx, [num_screens]
        jb      .next
        mov     eax, [active_screen]
        mul     esi
        add     eax, ebx
        push    1
        push    aScreens
        push    eax
        call    menu
        cmp     eax, -1
        jz      @f
        sub     eax, ebx
        div     esi
        mov     [active_screen], eax
@@:
        mov     eax, ebx
        call    mf_free
        jmp     change_screen

panels_OnKey:
        mov     ebp, [active_panel]
        mov     ecx, [ebp + panel1_index - panel1_data]
        mov     edx, [ebp + panel1_start - panel1_data]
        mov     ebx, [ebp + panel1_colst - panel1_data]
        add     ebx, edx
        mov     esi, panels_ctrlkeys
        jmp     process_ctrl_keys
.ret:
        ret
.up:
        jecxz   .ret
        dec     ecx
        mov     [ebp + panel1_index - panel1_data], ecx
        cmp     ecx, edx
        jae     .done_redraw
        mov     [ebp + panel1_start - panel1_data], ecx
.done_redraw:
;        call    draw_panel
;        ret
        jmp     draw_panel
.down:
        inc     ecx
        cmp     ecx, [ebp + panel1_numfiles - panel1_data]
        jae     .ret
        mov     [ebp + panel1_index - panel1_data], ecx
        cmp     ecx, ebx
        jb      .done_redraw
        sub     ecx, [ebp + panel1_colst - panel1_data]
        inc     ecx
        mov     [ebp + panel1_start - panel1_data], ecx
        jmp     .done_redraw
.left:
        jecxz   .ret
        sub     ecx, [ebp + panel1_colsz - panel1_data]
        jae     @f
        xor     ecx, ecx
@@:
        mov     [ebp + panel1_index - panel1_data], ecx
        cmp     ecx, edx
        jae     .done_redraw
        sub     edx, [ebp + panel1_colsz - panel1_data]
        jae     @f
        xor     edx, edx
@@:
        mov     [ebp + panel1_start - panel1_data], edx
        jmp     .done_redraw
.right:
        add     ecx, [ebp + panel1_colsz - panel1_data]
        cmp     ecx, [ebp + panel1_numfiles - panel1_data]
        jb      @f
        mov     ecx, [ebp + panel1_numfiles - panel1_data]
        dec     ecx
@@:
        mov     [ebp + panel1_index - panel1_data], ecx
        cmp     ecx, ebx
        jb      .done_redraw
        add     ebx, [ebp + panel1_colsz - panel1_data]
        cmp     ebx, [ebp + panel1_numfiles - panel1_data]
        jbe     @f
        mov     ebx, [ebp + panel1_numfiles - panel1_data]
@@:
        sub     ebx, [ebp + panel1_colst - panel1_data]
        jae     @f
        xor     ebx, ebx
@@:
        mov     [ebp + panel1_start - panel1_data], ebx
        jmp     .done_redraw
.tab:
        xor     [active_panel], panel1_data xor panel2_data
        call    draw_cmdbar
        call    draw_panel
        mov     ebp, [active_panel]
        jmp     .done_redraw
.home:
        and     [ebp + panel1_start - panel1_data], 0
        and     [ebp + panel1_index - panel1_data], 0
        jmp     .done_redraw
.end:
        mov     eax, [ebp + panel1_numfiles - panel1_data]
        dec     eax
        mov     [ebp + panel1_index - panel1_data], eax
        inc     eax
        sub     eax, [ebp + panel1_colst - panel1_data]
        jae     @f
        xor     eax, eax
@@:
        mov     [ebp + panel1_start - panel1_data], eax
        jmp     .done_redraw
.enter:
        call    get_curfile_folder_entry
        test    byte [ecx], 10h
        jnz     .enter_folder
        call    find_extension
        jc      .run_app
        jnz     .run_app
.run_association:
        mov     eax, [edi+4]
.run_association2:
        mov     [execparams], execdata
        mov     [execptr], eax
        jmp     .dorun
.run_app:
        mov     [execptr], execdata
        and     [execparams], 0
.dorun:
        lea     esi, [ebp + panel1_dir - panel1_data]
        mov     edi, execdata
@@:
        lodsb
        test    al, al
        jz      @f
        stosb
        cmp     edi, execdataend-1
        jae     .bigfilename
        jmp     @b
@@:
        lea     esi, [ecx+40]
        mov     al, '/'
        stosb
@@:
        lodsb
        stosb
        cmp     edi, execdataend
        ja      .bigfilename
        test    al, al
        jnz     @b
; for fasm call - special handling, because
; 1) fasm command line convention is different : fasm infile,outfile[,path] rather than tinypad infile
; 2) fasm will probably create new file in directory, so we want to reload panel data
        xor     edx, edx
        cmp     [execparams], edx
        jz      .nofasm
        cmp     [execptr], fasm
        jnz     .nofasm
        cmp     edi, execdata+(execdataend-execdata)/2
        ja      .bigfilename
        mov     esi, execdata
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        mov     byte [esi-1], ','
        inc     edx
; output file: no extension if input file has extension, '.bin' otherwise
        push    edi
@@:
        dec     edi
        cmp     byte [edi], '.'
        jz      .ptfound
        cmp     byte [edi], '/'
        jnz     @b
        pop     edi
        cmp     edi, execdataend-4
        ja      .bigfilename
        mov     dword [edi-1], '.bin'
        mov     byte [edi+3], 0
        jmp     .nofasm
.ptfound:
        mov     byte [edi], 0
        pop     edi
.nofasm:
        mov     ebx, execinfo
; if command line is more than 256 symbols, the kernel will truncate it
; we does not want this!
; N.B. We know that command line is either NULL or execdata, which is always ASCIIZ string,
;      but can be up to 1023 symbols
        mov     esi, [ebx+8]
        test    esi, esi
        jz      .cmdlinelenok
@@:
        lodsb
        test    al, al
        jnz     @b
        sub     esi, [ebx+8]
        dec     esi
        cmp     esi, 256
        ja      .bigcmdline
.cmdlinelenok:
        push    70
        pop     eax
        int     40h
        neg     eax
        js      @f
        push    aContinue
        mov     esi, esp
        call    get_error_msg
        push    eax
        push    aRunError
        mov     eax, esp
        push    esi
        push    1
        push    eax
        push    2
        push    -1
        push    -1
        push    aError
        call    SayErr
        add     esp, 3*4
        ret
@@:
        test    edx, edx
        jz      @f
        push    5
        pop     eax
        push    20
        pop     ebx
        int     0x40
        jmp     .ctrl_r
@@:
        ret
.bigfilename3:
        pop     esi
.bigfilename2:
        pop     esi
.bigfilename:
        mov     eax, aFileNameTooBig
@@:
        push    aContinue
        mov     esi, esp
        push    eax
        mov     eax, esp
        push    esi
        push    1
        push    eax
        push    1
        push    -1
        push    -1
        push    aError
        call    SayErr
        add     esp, 2*4
        ret
.bigcmdline:
        mov     eax, aCmdLineTooBig
        jmp     @b
.bigfoldername2:
        mov     byte [ecx], 0
.bigfoldername:
        mov     eax, aFolderNameTooBig
        jmp     @b
.copytoself:
        mov     eax, aCannotCopyToSelf
        jmp     @b
.enter_folder:
        lea     esi, [ecx+40]
        cmp     word [esi], '..'
        jnz     @f
        cmp     byte [esi+2], 0
        jz      .dotdot
@@:
        push    esi
        lea     esi, [ebp + panel1_dir - panel1_data]
        mov     edi, prev_dir
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        lea     edi, [esi-1]
        lea     edx, [ebp + panel1_dir - panel1_data + 1024]
        cmp     esi, edx
        pop     esi
        jae     .bigfoldername
        mov     ecx, edi
        mov     al, '/'
        cmp     [edi-1], al
        jz      @f
        stosb
@@:
        lodsb
        cmp     edi, edx
        jae     .bigfoldername2
        stosb
        test    al, al
        jnz     @b
.reread:
        call    read_folder
.done_cmdbar:
        call    draw_cmdbar
        jmp     .done_redraw
.dotdot:
        lea     edi, [ebp + panel1_dir - panel1_data]
        mov     al, 0
        or      ecx, -1
        repnz   scasb
        dec     edi
        mov     al, '/'
        std
        repnz   scasb
        cld
        inc     edi
        mov     byte [edi], 0
        inc     edi
        push    edi
        call    read_folder
        pop     edi
        mov     edx, [ebp + panel1_files - panel1_data]
        mov     ecx, [ebp + panel1_numfiles - panel1_data]
.scanloop:
        mov     esi, [edx]
        add     esi, 40
        push    esi edi
@@:
        lodsb
        call    match_symbol
        jnz     @f
        inc     edi
        test    al, al
        jnz     @b
@@:
        pop     edi esi
        jz      .scanfound
        add     edx, 4
        loop    .scanloop
        jmp     .scandone
.scanfound:
        sub     edx, [ebp + panel1_files - panel1_data]
        shr     edx, 2
        mov     [ebp + panel1_index - panel1_data], edx
        sub     edx, [ebp + panel1_colst - panel1_data]
        jb      .scandone
        inc     edx
        mov     [ebp + panel1_start - panel1_data], edx
.scandone:
        jmp     .done_cmdbar
.ctrl_f39:
        sub     al, 0x3D
        add     al, al
        mov     ah, [ebp + panel1_sortmode - panel1_data]
        and     ah, 0xFE
        cmp     al, ah
        jnz     @f
        and     [ebp + panel1_sortmode - panel1_data], 1
        or      al, [ebp + panel1_sortmode - panel1_data]
        xor     al, 1
@@:
        mov     [ebp + panel1_sortmode - panel1_data], al
        mov     eax, [ebp + panel1_index - panel1_data]
        mov     ecx, [ebp + panel1_files - panel1_data]
        push    dword [ecx+eax*4]
        push    ecx
        call    sort_files
        pop     edi
        pop     eax
        or      ecx, -1
        repnz   scasd
        not     ecx
        dec     ecx
        mov     [ebp + panel1_index - panel1_data], ecx
        sub     ecx, [ebp + panel1_start - panel1_data]
        jb      .less_start
        sub     ecx, [ebp + panel1_colst - panel1_data]
        jae     .gr_end
@@:     jmp     .done_redraw
.less_start:
        add     [ebp + panel1_start - panel1_data], ecx
        jmp     @b
.gr_end:
        inc     ecx
        add     [ebp + panel1_start - panel1_data], ecx
        jmp     @b
.alt_f12:
        mov     ebp, panel1_data
        cmp     al, 0x3B
        jz      @f
        mov     ebp, panel2_data
@@:
; get drives list
        mov     ebx, dirinfo
        mov     [ebx+dirinfo.size-dirinfo], 1
        mov     [ebx+dirinfo.dirdata-dirinfo], driveinfo
        mov     [ebx+dirinfo.name-dirinfo], tmpname
        mov     byte [tmpname], '/'
        xor     ecx, ecx
.drive_loop_e:
        mov     byte [tmpname+1], 0
        push    70
        pop     eax
        int     40h
        mov     ebx, dirinfo
        test    eax, eax
        jnz     .drive_loop_e_done
        mov     esi, driveinfo+32+40
        mov     edi, tmpname+1
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        push    [ebx+dirinfo.first-dirinfo]
        and     [ebx+dirinfo.first-dirinfo], 0
.drive_loop_i:
        push    70
        pop     eax
        int     40h
        mov     ebx, dirinfo
        test    eax, eax
        jnz     .drive_loop_i_done
        mov     eax, 32+8
        call    xmalloc
        test    eax, eax
        jz      .drive_loop_i_done
        jecxz   @f
        mov     [ecx], eax
@@:
        and     dword [eax], 0
        mov     [eax+4], ecx
        mov     ecx, eax
        lea     edi, [eax+8]
        mov     esi, tmpname
@@:
        lodsb
        test    al, al
        jz      @f
        stosb
        jmp     @b
@@:
        mov     esi, driveinfo+32+40
        mov     al, '/'
        stosb
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        inc     [ebx+dirinfo.first-dirinfo]
        jmp     .drive_loop_i
.drive_loop_i_done:
        pop     [ebx+dirinfo.first-dirinfo]
        inc     [ebx+dirinfo.first-dirinfo]
        jmp     .drive_loop_e
.drive_loop_e_done:
        and     [ebx+dirinfo.first-dirinfo], 0
        lea     edi, [ebp + panel1_dir - panel1_data]
.find_cur_drive_loop:
        push    edi
        lea     esi, [ecx+8]
@@:
        lodsb
        test    al, al
        jz      .cur_drive_found
        call    match_symbol
        jnz     @f
        inc     edi
        jmp     @b
@@:
        cmp     dword [ecx+4], 0
        jz      @f
        pop     edi
        mov     ecx, [ecx+4]
        jmp     .find_cur_drive_loop
@@:
.cur_drive_found:
        pop     edi
        push    1
        push    aDrive
        push    ecx
        mov     eax, [ebp + panel1_height - panel1_data]
        sub     eax, 2
        jae     @f
        add     eax, 2
@@:
        push    eax
        push    [ebp + panel1_width - panel1_data]
        push    [ebp + panel1_top - panel1_data]
        push    [ebp + panel1_left - panel1_data]
        call    menu_centered_in
        cmp     eax, -1
        jz      .ret2
        lea     esi, [eax+8]
        lea     edi, [ebp + panel1_dir - panel1_data]
        push    ecx esi edi
        mov     esi, edi
        mov     edi, prev_dir
        mov     ecx, 1024/4
        rep     movsd
        pop     edi esi ecx
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
@@:
        cmp     dword [ecx+4], 0
        jz      @f
        mov     ecx, [ecx+4]
        jmp     @b
@@:
        mov     eax, ecx
        mov     ecx, [ecx]
        call    mf_free
        test    ecx, ecx
        jnz     @b
        call    read_folder
        jmp     .done_redraw
.shift_f5:
        call    get_curfile_folder_entry
        lea     esi, [ecx+40]
        mov     edi, CopyDestEditBuf
        mov     eax, CopyDestEditBuf.length
        stosd
        scasd
        xor     eax, eax
        stosd
        mov     edx, edi
@@:
        lodsb
        test    al, al
        jz      .f5_common
        stosb
        jmp     @b
.f5:
        mov     edi, CopyDestEditBuf
        mov     eax, CopyDestEditBuf.length
        stosd
        scasd
        xor     eax, eax
        stosd
        mov     edx, edi
        mov     esi, ebp
        xor     esi, panel1_data xor panel2_data
        add     esi, panel1_dir - panel1_data
@@:
        lodsb
        test    al, al
        jz      @f
        cmp     edi, CopyDestEditBuf+12+511
        jae     .bigfoldername
        stosb
        jmp     @b
@@:
        mov     al, '/'
        stosb
.f5_common:
        mov     byte [edi], 0
        sub     edi, edx
        mov     [edx-8], edi
        mov     edi, CopySourceTextBuf
        mov     esi, aCopy1
@@:
        lodsb
        test    al, al
        jz      @f
        stosb
        jmp     @b
@@:
        call    get_curfile_folder_entry
        lea     esi, [ecx+40]
        lea     eax, [esi+1]
@@:
        inc     esi
        cmp     byte [esi-1], 0
        jnz     @b
        sub     esi, eax
        xchg    eax, esi
        dec     esi
        mov     edx, [cur_width]
        sub     edx, 50
        sub     eax, edx
        jbe     @f
        add     esi, eax
        mov     al, '.'
        stosb
        stosb
        stosb
@@:
        lodsb
        test    al, al
        jz      @f
        stosb
        jmp     @b
@@:
        mov     esi, aCopy2
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        mov     ebx, copy_dlgdata
        mov     eax, [cur_width]
        sub     eax, 12
        mov     [ebx + dlgtemplate.width], eax
        dec     eax
        dec     eax
        mov     [ebx - copy_dlgdata + copy_dlgdata.width2], eax
        mov     [ebx - copy_dlgdata + copy_dlgdata.width3], eax
        shr     eax, 1
        dec     eax
        dec     eax
        mov     [ebx - copy_dlgdata + copy_dlgdata.copy_x2], eax
        sub     eax, aCopyLength-1
        mov     [ebx - copy_dlgdata + copy_dlgdata.copy_x1], eax
        add     eax, aCopyLength+3
        mov     [ebx - copy_dlgdata + copy_dlgdata.cnl_x1], eax
        add     eax, aCancelBLength - 1
        mov     [ebx - copy_dlgdata + copy_dlgdata.cnl_x2], eax
        mov     byte [ebx - copy_dlgdata + copy_dlgdata.flags0], 0xC
        and     byte [ebx - copy_dlgdata + copy_dlgdata.flags1], not 4
        and     byte [ebx - copy_dlgdata + copy_dlgdata.flags2], not 4
        push    ebx
        call    DialogBox
        cmp     eax, copy_dlgdata.copy_btn
        jnz     .ret2
; Копируем
        mov     esi, CopyDestEditBuf+12
        mov     edi, esi
        xor     eax, eax
        or      ecx, -1
        repnz   scasb
        dec     edi
        dec     edi
        cmp     edi, esi
        jb      .ret2
        cmp     byte [edi], '/'
        jnz     @f
; Наличие/отсутствие заканчивающего слэша важно только для копирования папок
        cmp     edi, esi
        jz      @f
        mov     byte [edi], 0
        dec     edi
@@:
; Если путь не начинается со слэша, считаем его относительно текущей папки
        cmp     byte [esi], '/'
        jz      .copy_absolute_path
        push    esi
        push    edi
        lea     edi, [ebp + panel1_dir - panel1_data]
        or      ecx, -1
        xor     eax, eax
        repnz   scasb
        not     ecx
        pop     edi
        push    edi
        add     edi, ecx
        cmp     edi, CopyDestEditBuf+12+513
        pop     edi
        jb      @f
        pop     esi
        jmp     .bigfilename
@@:
        lea     edx, [edi+2]
        sub     edx, esi
        lea     edi, [edi+ecx+1]
        xchg    ecx, edx
        std
        lea     esi, [esi+ecx-1]
        rep     movsb
        cld
        pop     edi
        lea     esi, [ebp + panel1_dir - panel1_data]
        push    edi
        mov     ecx, edx
        rep     movsb
        mov     byte [edi-1], '/'
        pop     esi
.copy_absolute_path:
; Получаем атрибуты назначения
        mov     cl, 0x10
        xor     eax, eax
        mov     edi, esi
.countslashloop:
        cmp     byte [edi], '/'
        jnz     @f
        inc     eax
@@:
        inc     edi
        cmp     byte [edi], 0
        jnz     .countslashloop
        cmp     eax, 2
        jbe     @f
        mov     ebx, attrinfo
        mov     [attrinfo.attr], 0
        mov     [ebx + attrinfo.name - attrinfo], esi
        push    70
        pop     eax
        int     0x40
        mov     cl, byte [attrinfo.attr]
@@:
        test    cl, 0x10
        jz      .copyfile
; Нам подсунули каталог назначения, дописываем имя файла
        cmp     edi, CopyDestEditBuf+12+513
        jae     .bigfilename
        mov     al, '/'
        stosb
        push    esi
        call    get_curfile_folder_entry
        lea     esi, [ecx+40]
@@:
        lodsb
        cmp     edi, CopyDestEditBuf+12+513
        jae     .bigfilename2
        stosb
        test    al, al
        jnz     @b
        pop     esi
.copyfile:
; Имя исходного файла
        push    esi
        lea     esi, [ebp+panel1_dir-panel1_data]
        mov     edi, saved_file_name
        push    edi
@@:
        lodsb
        test    al, al
        jz      @f
        stosb
        jmp     @b
@@:
        mov     al, '/'
        stosb
        call    get_curfile_folder_entry
        lea     esi, [ecx+40]
@@:
        lodsb
        cmp     edi, saved_file_name+1024
        jae     .bigfilename3
        stosb
        test    al, al
        jnz     @b
        pop     esi
        pop     edi
; Нельзя скопировать файл поверх самого себя!
        push    esi edi
        call    strcmpi
        pop     edi esi
        jz      .copytoself
; Собственно, копируем
; esi->source name, edi->destination name
        mov     [writeinfo.code], 2
        mov     [writeinfo.name], edi
        and     dword [writeinfo.first], 0
        and     dword [writeinfo.first+4], 0
        mov     [writeinfo.data], copy_buffer
        mov     ebx, readinfo
        and     dword [ebx+readinfo.first-readinfo], 0
        and     dword [ebx+readinfo.first+4-readinfo], 0
        mov     [ebx+readinfo.size-readinfo], copy_buffer_size
        mov     [ebx+readinfo.data-readinfo], copy_buffer
        mov     [ebx+readinfo.name-readinfo], esi
.copyloop:
        mov     ebx, readinfo
        push    70
        pop     eax
        int     0x40
        test    eax, eax
        jz      .copyreadok
        cmp     eax, 6
        jz      .copyreadok
        push    esi
        push    aCannotReadFile
        call    get_error_msg
        push    eax
        mov     eax, esp
        push    RetryOrCancelBtn
        push    2
        push    eax
        push    3
        push    -1
        push    -1
        push    aError
        call    SayErr
        add     esp, 3*4
        test    eax, eax
        jz      .copyloop
        jmp     .copyfailed
.copyreadok:
        test    ebx, ebx
        jz      .copydone
        add     dword [readinfo.first], ebx
        adc     dword [readinfo.first+4], 0
        mov     [writeinfo.size], ebx
.copywrite:
        mov     ebx, writeinfo
        push    70
        pop     eax
        int     0x40
        test    eax, eax
        jz      .copywriteok
        push    edi
        push    aCannotWriteFile
        call    get_error_msg
        push    eax
        mov     eax, esp
        push    RetryOrCancelBtn
        push    2
        push    eax
        push    3
        push    -1
        push    -1
        push    aError
        call    SayErr
        add     esp, 3*4
        test    eax, eax
        jz      .copywrite
        jmp     .copyfailed
.copywriteok:
        mov     ecx, [writeinfo.size]
        add     dword [writeinfo.first], ecx
        adc     dword [writeinfo.first+4], 0
        mov     [writeinfo.code], 3
        cmp     ecx, copy_buffer_size
        jz      .copyloop
.copydone:
        push    ebp
        call    .ctrl_r
        pop     ebp
        xor     ebp, panel1_data xor panel2_data
        jmp     .ctrl_r
.copyfailed:
        cmp     [bConfirmDeleteIncomplete], 0
        jz      @f
        cmp     [writeinfo.code], 2
        jz      .copydone
        push    aIncompleteFile
        mov     eax, esp
        push    DeleteOrKeepBtn
        push    2
        push    eax
        push    1
        push    -1
        push    -1
        push    aCopyCaption
        call    SayErr
        add     esp, 4
        test    eax, eax
        jnz     .copydone
@@:
        mov     ebx, delinfo
        push    dword [ebx+21]
        mov     dword [ebx+21], edi
        push    70
        pop     eax
        int     0x40
; ignore errors
        pop     dword [delinfo+21]
        jmp     .copydone

.f3:
        call    view_file
.ret2:
        ret
.f8:
        call    get_curfile_folder_entry
        cmp     [bConfirmDelete], 0
        jz      .f8_allowed
        mov     ebx, f8_confirm_dlgdata
        mov     esi, aConfirmDeleteText
        mov     edi, aConfirmDeleteTextBuf
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        dec     edi
        mov     esi, aDeleteFolder
        test    byte [ecx], 10h
        jnz     @f
        mov     esi, aDeleteFile
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        lea     esi, [ecx+40]
        mov     [ebx - f8_confirm_dlgdata + f8_confirm_dlgdata.name], esi
        or      eax, -1
@@:
        inc     eax
        cmp     byte [eax+esi], 0
        jnz     @b
        sub     edi, aConfirmDeleteTextBuf+1
        cmp     eax, edi
        jae     @f
        mov     eax, edi
@@:
        inc     eax
        inc     eax
        mov     edx, [cur_width]
        sub     edx, 8
        cmp     eax, edx
        jbe     @f
        mov     eax, edx
@@:
        mov     [ebx + dlgtemplate.width], eax
        dec     eax
        dec     eax
        mov     [ebx - f8_confirm_dlgdata + f8_confirm_dlgdata.width2], eax
        mov     [ebx - f8_confirm_dlgdata + f8_confirm_dlgdata.width3], eax
        shr     eax, 1
        mov     [ebx - f8_confirm_dlgdata + f8_confirm_dlgdata.del_x2], eax
        sub     eax, aDeleteLength-1
        mov     [ebx - f8_confirm_dlgdata + f8_confirm_dlgdata.del_x1], eax
        add     eax, aDeleteLength
        mov     [ebx - f8_confirm_dlgdata + f8_confirm_dlgdata.cnl_x1], eax
        add     eax, aCancelLength - 1
        mov     [ebx - f8_confirm_dlgdata + f8_confirm_dlgdata.cnl_x2], eax
        or      byte [ebx - f8_confirm_dlgdata + f8_confirm_dlgdata.flags1], 4
        and     byte [ebx - f8_confirm_dlgdata + f8_confirm_dlgdata.flags2], not 4
        push    ebx
        call    DialogBox
        cmp     eax, f8_confirm_dlgdata.del_btn
        jnz     .ret2
.f8_allowed:
        lea     esi, [ebp + panel1_dir - panel1_data]
        mov     edi, execdata
@@:
        lodsb
        test    al, al
        jz      @f
        stosb
        jmp     @b
@@:
        lea     esi, [ecx+40]
        mov     al, '/'
        stosb
@@:
        lodsb
        cmp     edi, execdataend
        jae     .bigfilename
        stosb
        test    al, al
        jnz     @b
.retrydel:
        push    70
        pop     eax
        mov     ebx, delinfo
        int     0x40
        test    eax, eax
        jz      .ctrl_r
        push    execdata
        push    aCannotDeleteFolder
        call    get_curfile_folder_entry
        test    byte [ecx], 10h
        jnz     @f
        mov     dword [esp], aCannotDeleteFile
@@:
        call    get_error_msg
        push    eax
        mov     eax, esp
        push    RetryOrCancelBtn
        push    2
        push    eax
        push    3
        push    -1
        push    -1
        push    aError
        call    SayErr
        add     esp, 3*4
        test    eax, eax
        jz      .retrydel
.ctrl_r:
; Rescan panel
;       call    read_folder
;       jmp     .done_redraw
        mov     eax, [ebp + panel1_index - panel1_data]
        push    eax
        call    get_curfile_name
        mov     esi, ecx
        mov     edi, saved_file_name
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        push    [ebp + panel1_start - panel1_data]
        call    read_folder
        pop     [ebp + panel1_start - panel1_data]
        pop     [ebp + panel1_index - panel1_data]
        or      eax, -1
@@:
        inc     eax
        cmp     eax, [ebp + panel1_numfiles - panel1_data]
        jae     .ctrl_r.notfound
        mov     ecx, [ebp + panel1_files - panel1_data]
        mov     esi, [ecx+eax*4]
        add     esi, 40
        mov     edi, saved_file_name
        call    strcmpi
        jnz     @b
.ctrl_r.found:
        mov     [ebp + panel1_index - panel1_data], eax
.ctrl_r.notfound:
        mov     eax, [ebp + panel1_numfiles - panel1_data]
        dec     eax
        cmp     [ebp + panel1_index - panel1_data], eax
        jbe     @f
        mov     [ebp + panel1_index - panel1_data], eax
@@:
        mov     eax, [ebp + panel1_index - panel1_data]
        cmp     [ebp + panel1_start - panel1_data], eax
        jbe     @f
        mov     [ebp + panel1_start - panel1_data], eax
@@:
        inc     eax
        sub     eax, [ebp + panel1_colst - panel1_data]
        jae     @f
        xor     eax, eax
@@:
        cmp     [ebp + panel1_start - panel1_data], eax
        jae     @f
        mov     [ebp + panel1_start - panel1_data], eax
@@:
        mov     eax, [ebp + panel1_numfiles - panel1_data]
        sub     eax, [ebp + panel1_colst - panel1_data]
        jbe     @f
        cmp     [ebp + panel1_start - panel1_data], eax
        jbe     @f
        mov     [ebp + panel1_start - panel1_data], eax
@@:
        jmp     .done_redraw
.menu:
; display context menu
; ignore folders
        call    get_curfile_folder_entry
        test    byte [ecx], 10h
        jz      @f
.menuret:
        ret
@@:
        call    find_extension
        jc      .menuret
        jnz     .menuret
; known extension
        mov     ebx, [edi+8]
        test    ebx, ebx
        jz      .menuret
        mov     ecx, esi
@@:
        inc     ecx
        cmp     byte [ecx-1], 0
        jnz     @b
        sub     ecx, esi        ; ecx = длина имени файла+1 = длина заголовка+1
        cmp     ecx, 15
        jb      @f
        mov     cl, 15
@@:
        xor     edx, edx
.menucreateloop:
        mov     eax, [ebx]
        test    eax, eax
        jz      .menucreated
@@:
        inc     eax
        cmp     byte [eax-1], 0
        jnz     @b
        sub     eax, [ebx]
        cmp     eax, ecx
        ja      @f
        mov     eax, ecx
@@:
        add     eax, 12
        call    xmalloc
        test    eax, eax
        jz      .menucreated
        add     eax, 4
        test    edx, edx
        jz      @f
        mov     [edx], eax
@@:
        mov     [eax+4], edx
        mov     edx, eax
        push    esi
        mov     esi, [ebx+4]
        mov     [eax-4], esi
        mov     esi, [ebx]
        lea     edi, [eax+8]
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        sub     esi, [ebx]
        sub     esi, ecx
        jae     .menunoadd
        neg     esi
@@:
        mov     byte [edi-1], ' '
        stosb
        dec     esi
        jnz     @b
.menunoadd:
        pop     esi
        add     ebx, 8
        jmp     .menucreateloop
.menucreated:
        test    edx, edx
        jz      .menuret
        and     dword [edx], 0
@@:
        cmp     dword [edx+4], 0
        jz      @f
        mov     edx, [edx+4]
        jmp     @b
@@:
        push    1
        push    esi
        push    edx
        call    menu
        cmp     eax, -1
        jz      .menuret
        push    dword [eax-4]
@@:
        test    edx, edx
        jz      @f
        lea     eax, [edx-4]
        mov     edx, [edx]
        call    mf_free
        jmp     @b
@@:
        pop     eax
        call    get_curfile_folder_entry
        jmp     .run_association2
.f7:
        mov     dword [CopyDestEditBuf], CopyDestEditBuf.length
        and     dword [CopyDestEditBuf+4], 0
        and     dword [CopyDestEditBuf+8], 0
        mov     byte [CopyDestEditBuf+12], 0
        mov     ebx, mkdir_dlgdata
        mov     eax, [cur_width]
        sub     eax, 12
        mov     [ebx + dlgtemplate.width], eax
        dec     eax
        dec     eax
        mov     [ebx - mkdir_dlgdata + mkdir_dlgdata.width2], eax
        shr     eax, 1
        dec     eax
        dec     eax
        mov     [ebx - mkdir_dlgdata + mkdir_dlgdata.cont_x2], eax
        sub     eax, a_ContinueLength-1
        mov     [ebx - mkdir_dlgdata + mkdir_dlgdata.cont_x1], eax
        add     eax, a_ContinueLength+3
        mov     [ebx - mkdir_dlgdata + mkdir_dlgdata.cnl_x1], eax
        add     eax, aCancelBLength - 1
        mov     [ebx - mkdir_dlgdata + mkdir_dlgdata.cnl_x2], eax
        mov     byte [ebx - mkdir_dlgdata + mkdir_dlgdata.flags0], 0xC
        and     byte [ebx - mkdir_dlgdata + mkdir_dlgdata.flags1], not 4
        and     byte [ebx - mkdir_dlgdata + mkdir_dlgdata.flags2], not 4
        push    ebx
        call    DialogBox
        cmp     eax, mkdir_dlgdata.cont_btn
        jnz     .ret2
        mov     esi, CopyDestEditBuf+12
        cmp     byte [esi], 0
        jz      .ret2
        cmp     byte [esi], '/'
        jz      .mkdir_absolute_path
        push    esi
        lea     edi, [ebp + panel1_dir - panel1_data]
        or      ecx, -1
        xor     eax, eax
        repnz   scasb
        not     ecx
        mov     edi, esi
@@:
        cmp     byte [edi+1], 0
        jz      @f
        inc     edi
        jmp     @b
@@:
        push    edi
        add     edi, ecx
        cmp     edi, CopyDestEditBuf+12+513
        pop     edi
        jb      @f
        pop     esi
        jmp     .bigfilename
@@:
        lea     edx, [edi+2]
        sub     edx, esi
        lea     edi, [edi+ecx+1]
        xchg    ecx, edx
        std
        lea     esi, [esi+ecx-1]
        rep     movsb
        cld
        pop     edi
        lea     esi, [ebp + panel1_dir - panel1_data]
        push    edi
        mov     ecx, edx
        rep     movsb
        mov     byte [edi-1], '/'
        pop     esi
.mkdir_absolute_path:
.mkdir_retry:
        push    70
        pop     eax
        mov     ebx, mkdirinfo
        int     0x40
        test    eax, eax
        jz      @f
        push    CopyDestEditBuf+12
        push    aCannotMakeFolder
        call    get_error_msg
        push    eax
        mov     eax, esp
        push    RetryOrCancelBtn
        push    2
        push    eax
        push    3
        push    -1
        push    -1
        push    aError
        call    SayErr
        add     esp, 3*4
        test    eax, eax
        jz      .mkdir_retry
@@:
        jmp     .copydone

panels_OnRedraw:
        call    draw_cmdbar
        mov     ebp, panel1_data
        call    draw_panel
        mov     ebp, panel2_data
        call    draw_panel
        ret

init_console:
        mov     eax, [console_data_ptr]
        call    mf_free
        mov     eax, [cur_width]
        mul     [cur_height]
        mov     ecx, eax
        add     eax, eax
        add     eax, eax
        call    mf_alloc
        test    eax, eax
        jz      exit
        mov     [console_data_ptr], eax
        mov     edi, eax
        mov     ax, 0720h
        rep     stosw
        mov     [panel1_left], ecx
        mov     [panel1_top], ecx
        mov     [panel2_top], ecx
        mov     eax, [cur_width]
        inc     eax
        shr     eax, 1
        mov     [panel1_width], eax
        mov     [panel2_left], eax
        sub     eax, [cur_width]
        neg     eax
        mov     [panel2_width], eax
        mov     eax, [cur_height]
        dec     eax
        dec     eax
        mov     [panel1_height], eax
        mov     [panel2_height], eax
        ret

get_curfile_folder_entry:
        mov     ecx, [ebp + panel1_index - panel1_data]
        shl     ecx, 2
        add     ecx, [ebp + panel1_files - panel1_data]
        mov     ecx, [ecx]
        ret
get_curfile_name:
        call    get_curfile_folder_entry
        add     ecx, 40
        ret

panels_getname:
if lang eq ru
        mov     eax, 'Пане'
        stosd
        mov     eax, 'ли  '
        stosd
        mov     eax, '    '
        stosd
        stosb
else
        mov     eax, 'Pane'
        stosd
        mov     eax, 'ls  '
        stosd
        mov     eax, '    '
        stosd
        stosb
end if
        sub     ecx, 13
        mov     ebp, [active_panel]
        lea     esi, [ebp + panel1_dir - panel1_data]
        push    3
        pop     edx
@@:
        lodsb
        stosb
        dec     ecx
        test    al, al
        jz      @f
        cmp     al, '/'
        jnz     @b
        dec     edx
        jnz     @b
@@:
        test    al, al
        jnz     @f
        dec     esi
        dec     edi
@@:
        push    esi
@@:
        lodsb
        test    al, al
        jnz     @b
        dec     esi
        mov     ebx, esi
        sub     ebx, [esp]
        dec     esi
        push    esi
        mov     esi, [ebp + panel1_index - panel1_data]
        shl     esi, 2
        add     esi, [ebp + panel1_files - panel1_data]
        mov     esi, [esi]
        add     esi, 40
        push    esi
@@:
        lodsb
        test    al, al
        jnz     @b
        add     ebx, esi
        sub     ebx, [esp]
        dec     esi
        cmp     ebx, ecx
        jbe     @f
        mov     al, '.'
        stosb
        stosb
        stosb
        sub     ecx, 3
        mov     ebx, ecx
@@:
        add     edi, ebx
        inc     ecx
        std
@@:
        movsb
        dec     ecx
        jz      .nodir
        cmp     esi, [esp]
        jae     @b
        mov     al, '/'
        stosb
        dec     ecx
        jz      .nodir
        mov     esi, [esp+4]
@@:
        cmp     esi, [esp+8]
        jb      .nodir
        movsb
        loop    @b
.nodir:
        cld
        pop     eax
        pop     eax
        pop     eax
        ret

draw_window:
        push    12
        pop     eax
        push    1
        pop     ebx
        int     40h
        mov     al, 48
        mov     bl, 4
        int     40h
        mov     [skinh], eax
        mov     ebx, [cur_width]
        imul    ebx, font_width
        add     ebx, 100*65536 + 5*2-1
        mov     ecx, [cur_height]
        imul    ecx, font_height
        lea     ecx, [eax+ecx+5-1+100*65536]
        xor     eax, eax
        mov     edx, 0x53000000
        mov     edi, header
        int     40h
        mov     al, 13
        xor     edx, edx
        cmp     [fill_width], 0
        jz      @f
        mov     ebx, [wnd_width]
        sub     ebx, [fill_width]
        sub     ebx, 5-1
        shl     ebx, 16
        mov     bx, word [fill_width]
        mov     ecx, [skinh-2]
        mov     cx, word [wnd_height]
        sub     cx, word [skinh]
        sub     cx, 5-1
        int     0x40
@@:
        cmp     [fill_height], 0
        jz      @f
        mov     al, 13
        xor     edx, edx
        mov     ebx, 50000h
        mov     bx, word [wnd_width]
        sub     ebx, 9
        mov     ecx, [wnd_height]
        sub     ecx, [fill_height]
        sub     ecx, 5-1
        shl     ecx, 16
        mov     cx, word [fill_height]
        int     0x40
@@:
;        xor     ecx, ecx
;        call    draw_image
        and     [min_x], 0
        and     [min_y], 0
        mov     eax, [cur_width]
        dec     eax
        mov     [max_x], eax
        mov     eax, [cur_height]
        dec     eax
        mov     [max_y], eax
        call    draw_image.force
        mov     al, 12
        push    2
        pop     ebx
        int     40h
        ret

draw_image.nomem:
        mov     al, 13
        xor     edx, edx
        mov     ebx, [cur_width]
        imul    ebx, font_width
        add     ebx, 5*65536
        mov     ecx, [skinh-2]
        mov     cx, word [cur_height]
        imul    cx, font_height
        int     40h
        mov     al, 4
        mov     ebx, 32*65536+32
        mov     ecx, 0xFFFFFF
        mov     edx, nomem_draw
        push    nomem_draw.size
        pop     esi
        int     40h
        ret

draw_image:
; determine draw rectangle
        and     [max_x], 0
        or      [min_x], -1
        or      [min_y], -1
        mov     esi, [console_data_ptr]
        xor     eax, eax
        xor     edx, edx
        mov     ecx, [cur_width]
        imul    ecx, [cur_height]
.m1:
        mov     bx, [esi]
        cmp     bx, [esi+ecx*2]
        jz      .m2
        cmp     eax, [min_x]
        ja      @f
        mov     [min_x], eax
@@:
        cmp     eax, [max_x]
        jb      @f
        mov     [max_x], eax
@@:
        cmp     edx, [min_y]
        jae     @f
        mov     [min_y], edx
@@:
        mov     [max_y], edx
.m2:
        add     esi, 2
        inc     eax
        cmp     eax, [cur_width]
        jb      .m1
        xor     eax, eax
        inc     edx
        cmp     edx, [cur_height]
        jb      .m1
        mov     eax, [cursor_x]
        cmp     eax, -1
        jz      .m3
        cmp     eax, [min_x]
        ja      @f
        mov     [min_x], eax
@@:
        cmp     eax, [max_x]
        jb      @f
        mov     [max_x], eax
@@:
        mov     edx, [cursor_y]
        cmp     edx, [min_y]
        ja      @f
        mov     [min_y], edx
@@:
        cmp     edx, [max_y]
        jb      @f
        mov     [max_y], edx
@@:
.m3:
        xchg    eax, [old_cursor_x]
        xchg    edx, [old_cursor_y]
        cmp     eax, -1
        jz      .m4
        cmp     eax, [min_x]
        ja      @f
        mov     [min_x], eax
@@:
        cmp     eax, [max_x]
        jb      @f
        mov     [max_x], eax
@@:
        cmp     edx, [min_y]
        ja      @f
        mov     [min_y], edx
@@:
        cmp     edx, [max_y]
        jb      @f
        mov     [max_y], edx
@@:
.m4:
        cmp     [min_y], -1
        jz      .nodraw
.force:
; allocate memory for image
        mov     ecx, [max_x]
        sub     ecx, [min_x]
        inc     ecx
        mov     [used_width], ecx
        mov     edx, [max_y]
        sub     edx, [min_y]
        inc     edx
        mov     [used_height], edx
        imul    ecx, edx
        imul    ecx, font_width*font_height
        add     ecx, [heapend]
        push    64
        pop     eax
        push    1
        pop     ebx
        int     0x40
        test    eax, eax
        jnz     draw_image.nomem
        mov     edi, [heapend]
        mov     esi, [console_data_ptr]
        mov     eax, [min_y]
        imul    eax, [cur_width]
        add     eax, [min_x]
        lea     esi, [esi+eax*2]
        mov     ecx, [used_height]
.lh:
        push    ecx esi
        mov     ecx, [used_width]
.lw:
        push    ecx
        mov     ebx, [esi]
        mov     eax, [cur_width]
        imul    eax, [cur_height]
        mov     [eax*2+esi], bx
        movzx   eax, bl
        push    edi
        movzx   ebx, bh
        mov     ebp, ebx
        shr     ebp, 4
        and     ebx, 0xF
        sub     ebx, ebp
        add     esi, 2
if font_width > 8
        lea     edx, [eax+eax+font]
else
        lea     edx, [eax+font]
end if
        mov     ecx, font_height
.sh:
        push    ecx edx edi
        xor     ecx, ecx
        mov     edx, [edx]
.sw:
        shr     edx, 1
        sbb     eax, eax
        and     eax, ebx
        add     eax, ebp
        mov     [edi], al
        add     ecx, 1
        add     edi, 1
        cmp     ecx, font_width
        jb      .sw
        pop     edi edx ecx
        mov     eax, [used_width]
        imul    eax, font_width
        add     edi, eax
if font_width > 8
        add     edx, 256*2
else
        add     edx, 256
end if
        loop    .sh
        pop     edi
.skip_symbol:
        pop     ecx
        add     edi, font_width
        dec     ecx
        jnz     .lw
        mov     eax, [used_width]
        imul    eax, (font_height-1)*font_width
        add     edi, eax
        pop     esi ecx
        add     esi, [cur_width]
        add     esi, [cur_width]
        dec     ecx
        jnz     .lh
; cursor
        mov     eax, [cursor_y]
        inc     eax
        jz      .nocursor
        sub     eax, [min_y]
        mul     [used_width]
        imul    eax, font_height*font_width
        mov     edx, [cursor_x]
        sub     edx, [min_x]
        inc     edx
        imul    edx, font_width
        add     eax, edx
        add     eax, [heapend]
        mov     edx, [used_width]
        imul    edx, font_width
        neg     edx
        mov     ecx, (font_height*15+50)/100
.cursor_loop:
        push    ecx
        mov     ecx, font_width
        add     eax, edx
        push    eax
@@:
;        add     byte [eax-1], 0x10
        xor     byte [eax-1], 7
        sub     eax, 1
        loop    @b
        pop     eax
        pop     ecx
        loop    .cursor_loop
.nocursor:
        mov     ecx, [used_width]
        imul    ecx, font_width*65536
        mov     cx, word [used_height]
        imul    cx, font_height
        mov     edx, [min_x]
        imul    edx, font_width
        add     edx, 5
        shl     edx, 16
        mov     dx, word [min_y]
        imul    dx, font_height
        add     edx, [skinh]
        push    65
        pop     eax
        mov     ebx, [heapend]
        push    8
        pop     esi
        mov     edi, console_colors
        xor     ebp, ebp
        int     0x40
        push    64
        pop     eax
        push    1
        pop     ebx
        mov     ecx, [heapend]
        int     0x40
.nodraw:
        ret

get_console_ptr:
; in: eax=x, edx=y
; out: edi->console data
        push    edx
        imul    edx, [cur_width]
        add     edx, eax
        mov     edi, [console_data_ptr]
        lea     edi, [edi + edx*2]
        pop     edx
        ret

draw_keybar:
        pushad
        xor     eax, eax
        test    [ctrlstate], 3
        jz      @f
        inc     eax
@@:
        test    [ctrlstate], 0xC
        jz      @f
        or      al, 2
@@:
        test    [ctrlstate], 0x30
        jz      @f
        or      al, 4
@@:
        imul    eax, 6*12
        mov     esi, [active_screen_vtable]
        mov     esi, [esi+screen_vtable.keybar]
        add     esi, eax
        xor     ecx, ecx
        inc     ecx
        xor     eax, eax
        mov     edx, [cur_height]
        dec     edx
        call    get_console_ptr
        push    6
        pop     ebx
        mov     eax, [cur_width]
        sub     eax, 11+9+3*2+6
        cmp     eax, 7*11
        jl      @f
        cdq
        mov     bl, 11
        div     ebx
        mov     ebx, eax
@@:
        xor     edx, edx
.l:
        add     edx, 7
        cmp     cl, 10
        jb      @f
        inc     edx
@@:
        cmp     edx, [cur_width]
        ja      .ret
        cmp     cl, 10
        jae     .twodig
        lea     eax, [ecx+'0']
        stosb
        mov     al, [keybar_number_color]
        stosb
        jmp     .cmn
.twodig:
        mov     al, cl
        cbw
        div     [_10]
        add     al, '0'
        stosb
        mov     al, [keybar_number_color]
        stosb
        xchg    al, ah
        add     al, '0'
        stosw
.cmn:
        mov     ah, [keybar_name_color]
        push    ecx
        mov     cl, 6
@@:
        lodsb
        stosw
        loop    @b
        mov     al, ' '
        lea     ecx, [ebx-6]
        cmp     byte [esp], 12
        jz      .ret_pop
        add     edx, ecx
        rep     stosw
        pop     ecx
        inc     edx
        cmp     edx, [cur_width]
        ja      .ret
        mov     ah, [keybar_bgr_color]
        stosw
        inc     ecx
        jmp     .l
.ret_pop:
        pop     ecx
.ret:
        cmp     byte [edi-2], ' '
        jnz     @f
        dec     edi
        dec     edi
@@:
        push    edi
        xor     eax, eax
        mov     edx, [cur_height]
        call    get_console_ptr
        mov     ecx, edi
        pop     edi
        sub     ecx, edi
        shr     ecx, 1
        mov     al, ' '
        mov     ah, [keybar_name_color]
        rep     stosw
.done:
        popad
        ret

draw_cmdbar:
        mov     esi, [active_panel]
        add     esi, panel1_dir - panel1_data
        xor     eax, eax
        mov     edx, [cur_height]
        dec     edx
        dec     edx
        call    get_console_ptr
        mov     ah, [cmdbar_prefix_color]
        mov     ecx, [cur_width]
        dec     ecx
@@:
        lodsb
        test    al, al
        jz      @f
        stosw
        loop    @b
@@:
        mov     al, '>'
        stosw
        mov     al, ' '
        mov     ah, [cmdbar_normal_color]
        rep     stosw
        ret

draw_border:
        push    edi
        mov     al, 0xC9
        stosw
        mov     al, 0xCD
        lea     ecx, [ebx-2]
        rep     stosw
        mov     al, 0xBB
        stosw
        pop     edi
        add     edi, [cur_width]
        add     edi, [cur_width]
        lea     ecx, [edx-2]
.l:
        push    edi
        mov     al, 0xBA
        stosw
        mov     al, 0x20
        push    ecx
        lea     ecx, [ebx-2]
        rep     stosw
        pop     ecx
        mov     al, 0xBA
        stosw
        pop     edi
        add     edi, [cur_width]
        add     edi, [cur_width]
        loop    .l
        mov     al, 0xC8
        stosw
        mov     al, 0xCD
        lea     ecx, [ebx-2]
        rep     stosw
        mov     al, 0xBC
        stosw
        ret

draw_panel:
        mov     eax, [ebp + panel1_left - panel1_data]
        mov     edx, [ebp + panel1_top - panel1_data]
        call    get_console_ptr
; draw border
        mov     ah, [panel_border_color]
        mov     ebx, [ebp + panel1_width - panel1_data]
        mov     edx, [ebp + panel1_height - panel1_data]
        call    draw_border
        push    eax
        mov     eax, [ebp + panel1_left - panel1_data]
        mov     edx, [ebp + panel1_top - panel1_data]
        add     edx, [ebp + panel1_height - panel1_data]
        sub     edx, 3
        call    get_console_ptr
        pop     eax
        mov     al, 0xC7
        stosw
        mov     al, 0xC4
        lea     ecx, [ebx-2]
        rep     stosw
        mov     al, 0xB6
        stosw
        mov     eax, [ebp + panel1_width - panel1_data]
        sub     eax, 3
        shr     eax, 1
        mov     [column_width], eax
        mov     eax, [ebp + panel1_left - panel1_data]
        inc     eax
        mov     [column_left], eax
        add     eax, [column_width]
        mov     edx, [ebp + panel1_top - panel1_data]
        inc     edx
        mov     [column_top], edx
        dec     edx
        call    get_console_ptr
        mov     ah, [panel_border_color]
        mov     al, 0xD1
        mov     [edi], ax
        add     edi, [cur_width]
        add     edi, [cur_width]
        mov     ecx, [ebp + panel1_height - panel1_data]
        sub     ecx, 4
        mov     [column_height], ecx
        mov     al, 0xB3
@@:
        mov     [edi], ax
        add     edi, [cur_width]
        add     edi, [cur_width]
        loop    @b
        mov     al, 0xC1
        stosw
        mov     eax, [column_height]
        dec     eax
        mov     [ebp + panel1_colsz - panel1_data], eax
        add     eax, eax
        mov     [ebp + panel1_colst - panel1_data], eax
        mov     eax, [ebp + panel1_start - panel1_data]
        mov     [column_index], eax
        call    draw_column
        mov     eax, [ebp + panel1_width - panel1_data]
        sub     eax, 3
        mov     ecx, [column_width]
        sub     eax, ecx
        mov     [column_width], eax
        inc     ecx
        add     [column_left], ecx
        call    draw_column
; Заголовок панели (текущая папка)
        lea     esi, [ebp + panel1_dir - panel1_data]
        mov     edi, cur_header
        mov     ecx, [ebp + panel1_width - panel1_data]
        sub     ecx, 7
        movsb
@@:
        lodsb
        stosb
        dec     ecx
        test    al, al
        jz      .header_created
        cmp     al, '/'
        jnz     @b
        mov     edx, esi
@@:
        lodsb
        test    al, al
        jnz     @b
        sub     esi, edx
        dec     esi
        cmp     esi, ecx
        jbe     @f
        mov     word [edi], '..'
        mov     byte [edi+2], '.'
        add     edi, 3
        sub     ecx, 3
        add     edx, esi
        sub     edx, ecx
@@:
        mov     esi, edx
@@:
        lodsb
        stosb
        dec     ecx
        test    al, al
        jnz     @b
.header_created:
        mov     edx, [ebp + panel1_top - panel1_data]
        mov     eax, [ebp + panel1_left - panel1_data]
        shr     ecx, 1
        lea     eax, [eax+ecx+3]
        call    get_console_ptr
        mov     ah, [panel_active_header_color]
        cmp     ebp, [active_panel]
        jz      @f
        mov     ah, [panel_header_color]
@@:
        mov     al, ' '
        stosw
        mov     esi, cur_header
@@:
        lodsb
        test    al, al
        jz      @f
        stosw
        jmp     @b
@@:
        mov     al, ' '
        stosw
        mov     edx, [ebp + panel1_top - panel1_data]
        inc     edx
        mov     eax, [ebp + panel1_left - panel1_data]
        inc     eax
        call    get_console_ptr
        movzx   eax, [ebp + panel1_sortmode - panel1_data]
        mov     al, [compare_names+eax]
        stosb
        cmp     [ebp + panel1_numfiles - panel1_data], 0
        jz      .skip_curinfo
; Информация о выбранном файле
        mov     ebx, [ebp + panel1_index - panel1_data]
        mov     eax, [ebp + panel1_files - panel1_data]
        mov     ebx, [eax+ebx*4]
        mov     eax, [ebp + panel1_left - panel1_data]
        add     eax, [ebp + panel1_width - panel1_data]
        dec     eax
        mov     edx, [ebp + panel1_top - panel1_data]
        add     edx, [ebp + panel1_height - panel1_data]
        dec     edx
        dec     edx
        call    get_console_ptr
        mov     ecx, [ebp + panel1_width - panel1_data]
        dec     ecx
        dec     ecx
; Время модификации
        sub     edi, 5*2
        sub     ecx, 6
        mov     al, [ebx+24+2]
        aam
        or      ax, 0x3030
        mov     [edi], ah
        mov     [edi+2], al
        mov     [edi+4], byte ':'
        mov     al, [ebx+24+1]
        aam
        or      ax, 0x3030
        mov     [edi+6], ah
        mov     [edi+8], al
        mov     al, [panel_normal_color]
        mov     [edi+1], al
        mov     [edi+3], al
        mov     [edi+5], al
        mov     [edi+7], al
        mov     [edi+9], al
        dec     edi
        mov     [edi], al
        dec     edi
        mov     byte [edi], ' '
; Дата модификации
        sub     edi, 8*2
        sub     ecx, 9
        mov     al, [ebx+28+0]
        aam
        or      ax, 0x3030
        mov     [edi], ah
        mov     [edi+2], al
        mov     [edi+4], byte '.'
        mov     al, [ebx+28+1]
        aam
        or      ax, 0x3030
        mov     [edi+6], ah
        mov     [edi+8], al
        mov     [edi+10], byte '.'
        mov     ax, [ebx+28+2]
        div     [_100]
        mov     al, ah
        aam
        or      ax, 0x3030
        mov     [edi+12], ah
        mov     [edi+14], al
        mov     al, [panel_normal_color]
        mov     [edi+1], al
        mov     [edi+3], al
        mov     [edi+5], al
        mov     [edi+7], al
        mov     [edi+9], al
        mov     [edi+11], al
        mov     [edi+13], al
        mov     [edi+15], al
        dec     edi
        mov     [edi], al
        dec     edi
        mov     [edi], byte ' '
; Размер
        std
        mov     ah, [panel_normal_color]
        dec     edi
        dec     edi
        dec     ecx
        test    byte [ebx], 0x10
        jz      .size_file
        push    ecx
        mov     esi, aFolder+aFolder.size-1
        mov     ecx, aFolder.size
        cmp     word [ebx+40], '..'
        jnz     @f
        cmp     byte [ebx+42], 0
        jnz     @f
        mov     esi, aUp+aUp.size-1
        mov     cl, aUp.size
@@:
        sub     [esp], ecx
@@:
        lodsb
        stosw
        loop    @b
        pop     ecx
        jmp     .size_done
.size_file:
        mov     edx, [ebx+36]
        test    edx, edx
        jz      .size_less_4g
        cmp     edx, 10*256
        jae     .size_tera
; в гигабайтах
        mov     al, 'G'
        shl     edx, 2
        jmp     .size_letter
.size_tera:
; в терабайтах
        shr     edx, 8
        mov     al, 'T'
        jmp     .size_letter
.size_less_4g:
        mov     edx, [ebx+32]
        cmp     edx, 10*(1 shl 20)
        jae     .size_mega
        cmp     edx, 1 shl 20
        jb      .size_num
        shr     edx, 10
        mov     al, 'K'
        jmp     .size_letter
.size_mega:
        mov     al, 'M'
        shr     edx, 20
.size_letter:
        stosw
        dec     ecx
        mov     al, ' '
        stosw
        dec     ecx
.size_num:
        xchg    eax, edx
        xor     edx, edx
        div     [_10d]
        xchg    eax, edx
        add     al, '0'
        mov     ah, [panel_normal_color]
        stosw
        dec     ecx
        test    edx, edx
        jnz     .size_num
.size_done:
        cld
; Имя
        sub     edi, ecx
        sub     edi, ecx
        lea     esi, [ebx+40]
@@:
        lodsb
        test    al, al
        jnz     @b
        sub     esi, ebx
        sub     esi, 41
        mov     ah, [panel_normal_color]
        cmp     esi, ecx
        lea     edx, [ebx+40]
        jbe     @f
        mov     al, '{'
        mov     [edi-2], ax
        add     edx, esi
        sub     edx, ecx
@@:
        mov     esi, edx
@@:
        lodsb
        test    al, al
        jz      @f
        stosw
        loop    @b
@@:
        mov     al, ' '
        rep     stosw
.skip_curinfo:
        call    draw_image
        ret

draw_column:
        mov     eax, [column_left]
        mov     edx, [column_top]
        call    get_console_ptr
; заголовок столбца
        push    edi
        mov     ah, [column_header_color]
        mov     al, ' '
        mov     ecx, [column_width]
if lang eq ru
        sub     ecx, 3
else
        sub     ecx, 4
end if
        shr     ecx, 1
        rep     stosw
if lang eq ru
        mov     al, 'И'
        stosw
        mov     al, 'м'
        stosw
        mov     al, 'я'
        stosw
else
        mov     al, 'N'
        stosw
        mov     al, 'a'
        stosw
        mov     al, 'm'
        stosw
        mov     al, 'e'
        stosw
end if
        mov     al, ' '
        mov     ecx, [column_width]
if lang eq ru
        sub     ecx, 2
else
        sub     ecx, 3
end if
        shr     ecx, 1
        rep     stosw
        pop     edi
        add     edi, [cur_width]
        add     edi, [cur_width]
; файлы
        mov     edx, [ebp + panel1_numfiles - panel1_data]
        mov     ecx, [column_height]
        dec     ecx
.l:
        cmp     [column_index], edx
        jae     .ret
        push    ecx
        mov     esi, [column_index]
        mov     ecx, [ebp + panel1_files - panel1_data]
        mov     esi, [ecx+esi*4]
        add     esi, 40
        mov     ah, [esi - 40 + 5]
        cmp     ebp, [active_panel]
        jnz     @f
        mov     ecx, [column_index]
        cmp     ecx, [ebp + panel1_index - panel1_data]
        jnz     @f
        mov     ah, [esi - 40 + 6]
@@:
        mov     ecx, [column_width]
        push    edi
@@:
        lodsb
        test    al, al
        jz      @f
        stosw
        loop    @b
        cmp     byte [esi], 0
        jz      @f
        mov     byte [edi], '}'
@@:
        mov     al, ' '
        rep     stosw
        pop     edi
        add     edi, [cur_width]
        add     edi, [cur_width]
        inc     [column_index]
        pop     ecx
        dec     ecx
        jnz     .l
.ret:
        cmp     ebp, panel1_data
        jnz     .ret2
; Число экранов
        mov     eax, [num_screens]
        dec     eax
        jz      .ret2
        push    eax
        xor     eax, eax
        xor     edx, edx
        call    get_console_ptr
        mov     ah, [panel_nscreens_color]
        mov     al, '['
        stosw
        pop     eax
        push    -'0'
@@:
        xor     edx, edx
        div     [_10d]
        push    edx
        test    eax, eax
        jnz     @b
@@:
        pop     eax
        add     eax, '0'
        jz      @f
        mov     ah, [panel_nscreens_color]
        stosw
        jmp     @b
@@:
        mov     al, ']'
        mov     ah, [panel_nscreens_color]
        stosw
.ret2:
        ret

;insert_last_dot:
;        push    eax esi
;        mov     ah, 0
;.loop:
;        lodsb
;        test    al, al
;        jz      .done
;        cmp     al, '.'
;        jnz     .loop
;        mov     ah, 1
;        jmp     .loop
;.done:
;        test    ah, ah
;        jnz     @f
;        mov     byte [esi-1], '.'
;        mov     byte [esi], 0
;@@:
;        pop     esi eax
;        ret

;delete_last_dot:
;        push    esi
;@@:
;        cmp     byte [esi], 0
;        jz      @f
;        inc     esi
;        jmp     @b
;@@:
;        cmp     byte [esi-1], '.'
;        jnz     @f
;        mov     byte [esi-1], 0
;@@:     pop     esi
;        ret

read_folder:
        mov     eax, [ebp + panel1_nfa - panel1_data]
        mov     [dirinfo.size], eax
        shl     eax, 2
        add     eax, [ebp + panel1_files - panel1_data]
        mov     [dirinfo.dirdata], eax
        lea     eax, [ebp + panel1_dir - panel1_data]
        mov     [dirinfo.name], eax
.retry:
        push    70
        pop     eax
        mov     ebx, dirinfo
        int     40h
        test    eax, eax
        jz      .ok
        cmp     eax, 6
        jz      .ok
; Failed to read folder, notify user
        cmp     [bSilentFolderMode], 0
        jnz     .dont_notify
        push    aContinue
        push    aRetry
        mov     edx, esp
        call    get_error_msg
        push    [dirinfo.name]
        push    aCannotReadFolder
        push    eax
        mov     eax, esp
        push    edx
        push    2
        push    eax
        push    3
        push    -1
        push    -1
        push    aError
        call    SayErr
        add     esp, 5*4
        test    eax, eax
        jz      .retry
.dont_notify:
        mov     esi, prev_dir
        cmp     byte [esi], 0
        jz      @f
        lea     edi, [ebp + panel1_dir - panel1_data]
        mov     ecx, 1024/4
        rep     movsd
        mov     byte [prev_dir], 0
        ret
@@:
        mov     [bSilentFolderMode], 1  ; enter silent mode
        mov     esi, [dirinfo.name]
        xor     edx, edx
.up1:
        lodsb
        test    al, al
        jz      .up1done
        cmp     al, '/'
        jnz     .up1
        inc     edx
        lea     edi, [esi-1]
        jmp     .up1
.up1done:
        cmp     edx, 2
        jbe     .noup
        stosb
        jmp     read_folder
.noup:
        mov     esi, [dirinfo.name]
        mov     edi, esi
        lodsd
        or      eax, 0x00202000
        cmp     eax, '/rd/'
        jnz     @f
        lodsw
        cmp     ax, '1'
        jz      .nosetrd
@@:
        mov     eax, '/rd/'
        stosd
        mov     ax, '1'
        stosw
        jmp     read_folder
.nosetrd:
; Даже рамдиск не прочитался. Значит, не судьба...
        and     dword [ebp + panel1_numfiles - panel1_data], 0
        and     dword [ebp + panel1_index - panel1_data], 0
        and     dword [ebp + panel1_start - panel1_data], 0
        mov     [bSilentFolderMode], 0  ; leave silent mode
        ret
.ok:
        mov     eax, [dirinfo.dirdata]
        cmp     [eax+8], ebx
        jz      .readdone
        push    eax
        mov     eax, [ebp + panel1_files - panel1_data]
        call    mf_free
        pop     eax
        mov     eax, [eax+8]
        add     eax, 0xF
        and     eax, not 0xF
        push    eax
        imul    eax, 4+304
        add     eax, 32
        call    xmalloc
        test    eax, eax
        jnz     .succ1
        pop     eax
        jmp     .readdone
.succ1:
        mov     [ebp + panel1_files - panel1_data], eax
        pop     [ebp + panel1_nfa - panel1_data]
        mov     [prev_dir], 0
        jmp     read_folder
.readdone:
        and     [ebp + panel1_start - panel1_data], 0
        and     [ebp + panel1_index - panel1_data], 0
        and     [ebp + panel1_start - panel1_data], 0
        mov     edi, [ebp + panel1_files - panel1_data]
        mov     eax, [ebp + panel1_nfa - panel1_data]
        lea     eax, [edi + eax*4 + 32]
        mov     ecx, [eax-32+4]
        test    ecx, ecx
        jz      .loopdone
        xor     edx, edx
; Игнорируем специальные входы, соответствующие папке '.' и метке тома
.ptrinit:
        cmp     word [eax+40], '.'
        jz      .loopcont
        test    byte [eax], 8
        jnz     .loopcont
        test    edx, edx
        jnz     .nodotdot
        cmp     word [eax+40], '..'
        jnz     .nodotdot
        cmp     byte [eax+42], 0
        jnz     .nodotdot
        mov     edx, eax
        push    edi
@@:
        cmp     edi, [ebp + panel1_files - panel1_data]
        jbe     @f
        push    dword [edi-4]
        pop     dword [edi]
        sub     edi, 4
        jmp     @b
@@:
        stosd
        pop     edi
        scasd
        jmp     .dotdot
.nodotdot:
        stosd
.dotdot:
; подсветка
;        call    insert_last_dot
        pushad
        mov     ebp, eax
        lea     esi, [ebp+40]
        mov     edi, lower_file_name
        mov     edx, edi
@@:
        lodsb
        call    tolower
        stosb
        test    al, al
        jnz     @b
        mov     esi, edx
        lea     edx, [edi-1]
        xor     ecx, ecx
.highlight_test_loop:
        mov     ebx, [highlight_groups+ecx*4]
        mov     al, [ebx + highlight.IncludeAttributes]
        mov     ah, [ebp]
        and     ah, al
        cmp     ah, al
        jnz     .highlight_test_failed
        lea     edi, [ebx + highlight.Mask]
        call    match_mask_rev_lowercase
        jc      .highlight_test_failed
        mov     ah, [ebx + highlight.NormalColor]
        test    ah, ah
        jnz     @f
        mov     ah, [panel_normal_color]
@@:
        mov     [ebp+5], ah
        mov     ah, [ebx + highlight.CursorColor]
        test    ah, ah
        jnz     @f
        mov     ah, [panel_cursor_color]
@@:
        mov     [ebp+6], ah
        jmp     .doname
.highlight_test_failed:
        inc     ecx
        cmp     ecx, [highlight_num_groups]
        jb      .highlight_test_loop
.nohighlight:
        mov     ah, [panel_normal_color]
        mov     [ebp+5], ah
        mov     ah, [panel_cursor_color]
        mov     [ebp+6], ah
.doname:
;        call    delete_last_dot
        popad
.loopcont:
        add     eax, 304
        dec     ecx
        jnz     .ptrinit
.loopdone:
        sub     edi, [ebp + panel1_files - panel1_data]
        shr     edi, 2
        mov     [ebp + panel1_numfiles - panel1_data], edi
.done:
; Сортировка
sort_files:
        movzx   eax, [ebp + panel1_sortmode - panel1_data]
        mov     ebx, [compare_fns + eax*4]
        mov     edx, [ebp + panel1_files - panel1_data]
        mov     ecx, [ebp + panel1_numfiles - panel1_data]
        jecxz   .skip
        mov     eax, [edx]
        cmp     word [eax], '..'
        jnz     .nodotdot
        cmp     byte [eax+2], 0
        jnz     .nodotdot
        dec     ecx
        add     edx, 4
.nodotdot:
        call    sort
.skip:
        mov     [bSilentFolderMode], 0  ; leave silent mode
        ret

compare_name:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        add     esi, 40
        add     edi, 40
        jmp     strcmpi
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

compare_name_rev:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        add     esi, 40
        add     edi, 40
        xchg    esi, edi
        jmp     strcmpi
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

strcmpi:
        push    eax
@@:
        lodsb
        call    match_symbol
        jnz     .ret
        inc     edi
        test    al, al
        jnz     @b
.ret:
        pop     eax
        ret

compare_ext:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        add     esi, 40
        add     edi, 40
        push    esi edi
        call    seek_ext
        xchg    esi, edi
        call    seek_ext
        xchg    esi, edi
        call    strcmpi
        jnz     .ret
        pop     edi esi
        jmp     strcmpi
.ret:
        pop     edi esi
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

compare_ext_rev:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        add     esi, 40
        add     edi, 40
        push    esi edi
        call    seek_ext
        xchg    esi, edi
        call    seek_ext
        call    strcmpi
        jnz     .ret
        pop     edi esi
        xchg    esi, edi
        jmp     strcmpi
.ret:
        pop     edi esi
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

seek_ext:
        push    eax
        xor     eax, eax
.l:
        inc     esi
        cmp     byte [esi-1], '.'
        jnz     @f
        mov     eax, esi
@@:
        cmp     byte [esi-1], 0
        jnz     .l
        test    eax, eax
        jnz     @f
        lea     eax, [esi-1]
@@:
        mov     esi, eax
        pop     eax
        ret

compare_modified:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        push    edi
        mov     edi, [edi+28]
        cmp     edi, [esi+28]
        pop     edi
        jnz     @f
        push    edi
        mov     edi, [edi+24]
        cmp     edi, [esi+24]
        pop     edi
        jnz     @f
        add     esi, 40
        add     edi, 40
        jmp     strcmpi
@@:
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

compare_modified_rev:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        push    esi
        mov     esi, [esi+28]
        cmp     esi, [edi+28]
        pop     esi
        jnz     @f
        push    esi
        mov     esi, [esi+24]
        cmp     esi, [edi+24]
        pop     esi
        jnz     @f
        add     esi, 40
        add     edi, 40
        xchg    esi, edi
        jmp     strcmpi
@@:
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

compare_size:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        push    edi
        mov     edi, [edi+36]
        cmp     edi, [esi+36]
        pop     edi
        jnz     @f
        push    edi
        mov     edi, [edi+32]
        cmp     edi, [esi+32]
        pop     edi
        jnz     @f
        add     esi, 40
        add     edi, 40
        jmp     strcmpi
@@:
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

compare_size_rev:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        push    esi
        mov     esi, [esi+36]
        cmp     esi, [edi+36]
        pop     esi
        jnz     @f
        push    esi
        mov     esi, [esi+32]
        cmp     esi, [edi+32]
        pop     esi
        jnz     @f
        add     esi, 40
        add     edi, 40
        xchg    esi, edi
        jmp     strcmpi
@@:
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

compare_unordered:
        cmp     esi, edi
        ret
compare_unordered_rev:
        cmp     edi, esi
        ret

compare_created:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        push    edi
        mov     edi, [edi+12]
        cmp     edi, [esi+12]
        pop     edi
        jnz     @f
        push    edi
        mov     edi, [edi+8]
        cmp     edi, [esi+8]
        pop     edi
        jnz     @f
        add     esi, 40
        add     edi, 40
        jmp     strcmpi
@@:
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

compare_created_rev:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        push    esi
        mov     esi, [esi+12]
        cmp     esi, [edi+12]
        pop     esi
        jnz     @f
        push    esi
        mov     esi, [esi+8]
        cmp     esi, [edi+8]
        pop     esi
        jnz     @f
        add     esi, 40
        add     edi, 40
        xchg    esi, edi
        jmp     strcmpi
@@:
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

compare_accessed:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        push    edi
        mov     edi, [edi+20]
        cmp     edi, [esi+20]
        pop     edi
        jnz     @f
        push    edi
        mov     edi, [edi+16]
        cmp     edi, [esi+16]
        pop     edi
        jnz     @f
        add     esi, 40
        add     edi, 40
        jmp     strcmpi
@@:
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

compare_accessed_rev:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        push    esi
        mov     esi, [esi+20]
        cmp     esi, [edi+20]
        pop     esi
        jnz     @f
        push    esi
        mov     esi, [esi+16]
        cmp     esi, [edi+16]
        pop     esi
        jnz     @f
        add     esi, 40
        add     edi, 40
        xchg    esi, edi
        jmp     strcmpi
@@:
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

if 0
match_mask:
; in: esi->name, edi->mask
; out: CF clear <=> match
        pusha
        xchg    esi, edi
.main_cycle:
        push    esi
@@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, ','
        jz      @f
        cmp     al, '|'
        jnz     @b
@@:
        mov     [esi-1], byte 0
        xchg    esi, [esp]
        call    match_single_mask
        pop     esi
        mov     [esi-1], al
        jnc     .found
        cmp     al, ','
        jz      .main_cycle
.done_fail:
        stc
        popa
        ret
.found:
        test    al, al
        jz      .done_succ
        cmp     al, '|'
        jz      .test_exclude
        lodsb
        jmp     .found
.done_succ:
        clc
        popa
        ret
.test_exclude:
        push    esi
@@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, ','
        jnz     @b
@@:
        mov     byte [esi-1], 0
        xchg    esi, [esp]
        call    match_single_mask
        pop     esi
        mov     [esi-1], al
        jnc     .done_fail
        test    al, al
        jz      .done_succ
        jmp     .test_exclude

match_single_mask:
; in: esi->mask, edi->name
; out: CF clear <=> match
        pusha
.mask_symbol:
        lodsb
        test    al, al
        jz      .mask_done
        cmp     al, '*'
        jz      .asterisk
        cmp     al, '?'
        jz      .quest
        cmp     al, '['
        jz      .list
        call    match_symbol
        jnz     .done_fail
        inc     edi
        jmp     .mask_symbol
.done_fail:
        stc
        popa
        ret
.mask_done:
        scasb
        jnz     .done_fail
.done_succ:
        clc
        popa
        ret
.quest:
        mov     al, 0
        scasb
        jz      .done_fail
        jmp     .mask_symbol
.list:
        lodsb
        cmp     al, ']'
        jz      .done_fail
        cmp     byte [esi+1], '-'
        jz      .range
        call    match_symbol
        jnz     .list
.listok:
        inc     edi
@@:
        lodsb
        cmp     al, ']'
        jnz     @b
        jmp     .mask_symbol
.range:
        call    match_symbol
        ja      @f
        mov     al, [esi+2]
        call    match_symbol
        jae     .listok
@@:
        inc     esi
        jmp     .list
.asterisk:
        cmp     byte [esi], 0
        jz      .done_succ
        cmp     byte [esi], '?'
        jnz     @f
        mov     al, 0
        scasb
        jz      .done_fail
        inc     esi
        jmp     .asterisk
@@:
        cmp     byte [esi], '['
        jz      .asterisk_common
; the mask is ...*<normal-symbol>...
.asterisk_normal:
        mov     al, [esi]
@@:
        cmp     byte [edi], 0
        jz      .done_fail
        call    match_symbol
        jz      @f
        inc     edi
        jmp     @b
@@:
        inc     edi
        inc     esi
        call    match_single_mask
        jnc     .done_succ
        dec     esi
        jmp     .asterisk_normal
.asterisk_common:
        push    edi
@@:
        call    match_single_mask
        jnc     @f
        mov     al, 0
        scasb
        jnz     @b
        pop     edi
        jmp     .done_fail
@@:
        pop     edi
        jmp     .done_succ

match_mask_rev:
; in: esi->name, edx->end of name, edi->mask
; out: CF clear <=> match
        pusha
        xchg    esi, edx
        xchg    esi, edi
.main_cycle:
        mov     ecx, esi
@@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, ','
        jz      @f
        cmp     al, '|'
        jnz     @b
@@:
        dec     esi
        mov     [esi], byte 0
        call    match_single_mask_rev2
        mov     [esi], al
        inc     esi
        jnc     .found
        cmp     al, ','
        jz      .main_cycle
.done_fail:
        stc
        popa
        ret
.found:
        test    al, al
        jz      .done_succ
        cmp     al, '|'
        jz      .test_exclude
        lodsb
        jmp     .found
.done_succ:
        clc
        popa
        ret
.test_exclude:
        mov     ecx, esi
@@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, ','
        jnz     @b
@@:
        dec     esi
        mov     byte [esi], 0
        call    match_single_mask_rev2
        mov     [esi], al
        inc     esi
        jnc     .done_fail
        test    al, al
        jz      .done_succ
        jmp     .test_exclude

match_single_mask_rev2:
        pusha
        jmp     match_single_mask_rev.mask_symbol
match_single_mask_rev:
; in: esi->mask, edi->end of name, edx->start of name
; out: CF clear <=> match
        pusha
        mov     ecx, esi
@@:
        lodsb
        test    al, al
        jnz     @b
        dec     esi
; esi->end of mask, ecx->start of mask
.mask_symbol:
        dec     esi
        cmp     esi, ecx
        jb      .mask_done
        mov     al, [esi]
        cmp     al, '*'
        jz      .asterisk
        cmp     al, '?'
        jz      .quest
        cmp     al, ']'
        jz      .list
        dec     edi
        cmp     edi, edx
        jb      .done_fail
        call    match_symbol
        jz      .mask_symbol
.done_fail:
        stc
        popa
        ret
.mask_done:
        cmp     edi, edx
        jnz     .done_fail
.done_succ:
        clc
        popa
        ret
.quest:
        dec     edi
        cmp     edi, edx
        jb      .done_fail
        jmp     .mask_symbol
.list:
        dec     edi
        cmp     edi, edx
        jb      .done_fail
.list_check:
        dec     esi
        cmp     esi, ecx
        jbe     .done_fail
        mov     al, [esi]
        cmp     al, '['
        jz      .done_fail
        cmp     byte [esi-1], '-'
        jz      .range
        call    match_symbol
        jnz     .list_check
.listok:
@@:
        dec     esi
        cmp     esi, ecx
        jb      .done_fail
        cmp     byte [esi], '['
        jnz     @b
        jmp     .mask_symbol
.range:
        call    match_symbol
        jb      @f
        mov     al, [esi-2]
        call    match_symbol
        jbe     .listok
@@:
        dec     esi
        dec     esi
        jmp     .list_check
.asterisk:
        cmp     esi, ecx
        jz      .done_succ
        cmp     byte [esi-1], '?'
        jnz     @f
        cmp     edi, edx
        jz      .done_fail
        dec     esi
        jmp     .asterisk
@@:
        cmp     byte [esi-1], ']'
        jz      .asterisk_common
; the mask is ...<normal-symbol>*...
.asterisk_normal:
        mov     al, [esi-1]
@@:
        cmp     edi, edx
        jz      .done_fail
        call    match_symbol
        jz      @f
        dec     edi
        jmp     @b
@@:
        dec     edi
        dec     esi
        call    match_single_mask_rev2
        jnc     .done_succ
        inc     esi
        jmp     .asterisk_normal
.asterisk_common:
        push    edi
@@:
        call    match_single_mask_rev2
        jnc     @f
        dec     edi
        cmp     edi, edx
        jae     @b
        pop     edi
        jmp     .done_fail
@@:
        pop     edi
        jmp     .done_succ
end if

tolower:
        cmp     al, 'A'
        jb      @f
        cmp     al, 'Z'
        ja      @f
        add     al, ' '
@@:     ret

match_symbol:
; in: al,[edi]=symbols
; out: flags as 'cmp al,[edi]'
        push    eax
        call    tolower
        mov     ah, [edi]
        xchg    al, ah
        call    tolower
        cmp     ah, al
        pop     eax
        ret

match_mask_rev_lowercase:
; in: esi->name, edx->end of name, edi->mask
; out: CF clear <=> match
        pusha
        xchg    esi, edx
        xchg    esi, edi
.main_cycle:
        mov     ecx, esi
@@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, ','
        jz      @f
        cmp     al, '|'
        jnz     @b
@@:
        dec     esi
        mov     [esi], byte 0
        call    match_single_mask_rev_lowercase
        mov     [esi], al
        inc     esi
        jnc     .found
        cmp     al, ','
        jz      .main_cycle
.done_fail:
        stc
        popa
        ret
.found:
        test    al, al
        jz      .done_succ
        cmp     al, '|'
        jz      .test_exclude
        lodsb
        jmp     .found
.done_succ:
        clc
        popa
        ret
.test_exclude:
        mov     ecx, esi
@@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, ','
        jnz     @b
@@:
        dec     esi
        mov     byte [esi], 0
        call    match_single_mask_rev_lowercase
        mov     [esi], al
        inc     esi
        jnc     .done_fail
        test    al, al
        jz      .done_succ
        jmp     .test_exclude

match_single_mask_rev_lowercase:
; in: esi->end of mask, ecx->start of mask, edi->end of name, edx->start of name
; out: CF clear <=> match
        push    esi edi eax
.mask_symbol:
        dec     esi
        cmp     esi, ecx
        jb      .mask_done
        mov     al, [esi]
        cmp     al, '*'
        jz      .asterisk
        cmp     al, '?'
        jz      .quest
        cmp     al, ']'
        jz      .list
        dec     edi
        cmp     edi, edx
        jb      .done_fail
        cmp     al, [edi]
        jz      .mask_symbol
.done_fail:
        stc
        pop     eax edi esi
        ret
.mask_done:
        cmp     edi, edx
        jnz     .done_fail
.done_succ:
        clc
        pop     eax edi esi
        ret
.quest:
        dec     edi
        cmp     edi, edx
        jb      .done_fail
        jmp     .mask_symbol
.list:
        dec     edi
        cmp     edi, edx
        jb      .done_fail
.list_check:
        dec     esi
        cmp     esi, ecx
        jbe     .done_fail
        mov     al, [esi]
        cmp     al, '['
        jz      .done_fail
        cmp     byte [esi-1], '-'
        jz      .range
        cmp     al, [edi]
        jnz     .list_check
.listok:
@@:
        dec     esi
        cmp     esi, ecx
        jb      .done_fail
        cmp     byte [esi], '['
        jnz     @b
        jmp     .mask_symbol
.range:
        cmp     al, [edi]
        jb      @f
        mov     al, [esi-2]
        cmp     al, [edi]
        jbe     .listok
@@:
        dec     esi
        dec     esi
        jmp     .list_check
.asterisk:
        cmp     esi, ecx
        jz      .done_succ
        cmp     byte [esi-1], '?'
        jnz     @f
        cmp     edi, edx
        jz      .done_fail
        dec     esi
        jmp     .asterisk
@@:
        cmp     byte [esi-1], ']'
        jz      .asterisk_common
; the mask is ...<normal-symbol>*...
.asterisk_normal:
        mov     al, [esi-1]
@@:
        cmp     edi, edx
        jz      .done_fail
        cmp     al, [edi]
        jz      @f
        dec     edi
        jmp     @b
@@:
        dec     edi
        dec     esi
        call    match_single_mask_rev_lowercase
        jnc     .done_succ
        inc     esi
        jmp     .asterisk_normal
.asterisk_common:
        push    edi
@@:
        call    match_single_mask_rev_lowercase
        jnc     @f
        dec     edi
        cmp     edi, edx
        jae     @b
        pop     edi
        jmp     .done_fail
@@:
        pop     edi
        jmp     .done_succ

find_extension:
        lea     esi, [ecx+40]
        push    esi
@@:
        lodsb
        test    al, al
        jnz     @b
@@:
        dec     esi
        cmp     byte [esi], '.'
        jz      .found_ext
        cmp     esi, [esp]
        ja      @b
; empty extension
        pop     esi
        stc
        ret
.found_ext:
        inc     esi
        mov     edi, associations
@@:
        push    esi edi
        mov     edi, [edi]
        call    strcmpi
        pop     edi esi
        jz      @f
        add     edi, 12
        cmp     edi, associations_end
        jb      @b
; unknown extension
        inc     edi
@@:
        pop     esi
        ret

header  db      'Kolibri Far 0.21',0

nomem_draw      db      'No memory for redraw.',0
.size = $ - nomem_draw

def_left_dir    db      '/rd/1',0
def_right_dir   db      '/hd0/1',0

bSilentFolderMode db    1

if lang eq ru
aFolder         db      'Папка'
.size = $-aFolder
aUp             db      'Вверх'
.size = $-aUp
aDrive          db      'Диск',0
aScreens        db      'Экраны',0
else
aFolder         db      'Folder'
.size = $-aFolder
aUp             db      'Up'
.size = $-aUp
aDrive          db      'Drive',0
aScreens        db      'Screens',0
end if

_10d dd 10
_100d dd 100
_10 db 10
_100 db 100

fpu_cw  dw      000011100111111b

keybar_panels:
if lang eq ru
; без клавиш-модификаторов
        db      'Помощь'
        db      'ПользМ'
        db      'Просм '
        db      'Редакт'
        db      'Копир '
        db      'Перен '
        db      'Папка '
        db      'Удален'
        db      'КонфМн'
        db      'Выход '
        db      'Модули'
        db      'Экраны'
; Shift
        db      'Добавл'
        db      'Распак'
        db      'АрхКом'
        db      'Редак.'
        db      'Копир '
        db      'Переим'
        db      '      '
        db      'Удален'
        db      'Сохран'
        db      'Послдн'
        db      'Группы'
        db      'Выбран'
; Ctrl
        db      'Левая '
        db      'Правая'
        db      'Имя   '
        db      'Расшир'
        db      'Модиф '
        db      'Размер'
        db      'Несорт'
        db      'Создан'
        db      'Доступ'
        db      'Описан'
        db      'Владел'
        db      'Сорт  '
; Ctrl+Shift
        db      '      '
        db      '      '
        db      'Просм '
        db      'Редакт'
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
; Alt
        db      'Левая '
        db      'Правая'
        db      'Смотр.'
        db      'Редак.'
        db      'Печать'
        db      'Связь '
        db      'Искать'
        db      'Истор '
        db      'Видео '
        db      'Дерево'
        db      'ИстПр '
        db      'ИстПап'
; Alt+Shift
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      'КонфПл'
        db      '      '
        db      '      '
        db      '      '
; Alt+Ctrl
times 12 db     '      '
; Alt+Ctrl+Shift
times 12 db     '      '
else
; No modificators
        db      'Help  '
        db      'UserMn'
        db      'View  '
        db      'Edit  '
        db      'Copy  '
        db      'RenMov'
        db      'MkFold'
        db      'Delete'
        db      'ConfMn'
        db      'Quit  '
        db      'Plugin'
        db      'Screen'
; Shift
        db      'Add   '
        db      'Extrct'
        db      'ArcCmd'
        db      'Edit..'
        db      'Copy  '
        db      'Rename'
        db      '      '
        db      'Delete'
        db      'Save  '
        db      'Last  '
        db      'Group '
        db      'SelUp '
; Ctrl
        db      'Left  '
        db      'Right '
        db      'Name  '
        db      'Extens'
        db      'Modifn'
        db      'Size  '
        db      'Unsort'
        db      'Creatn'
        db      'Access'
        db      'Descr '
        db      'Owner '
        db      'Sort  '
; Ctrl+Shift
        db      '      '
        db      '      '
        db      'View  '
        db      'Edit  '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
; Alt
        db      'Left  '
        db      'Right '
        db      'View..'
        db      'Edit..'
        db      'Print '
        db      'MkLink'
        db      'Find  '
        db      'Histry'
        db      'Video '
        db      'Tree  '
        db      'ViewHs'
        db      'FoldHs'
; Alt+Shift
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      'ConfPl'
        db      '      '
        db      '      '
        db      '      '
; Alt+Ctrl
times 12 db     '      '
; Alt+Ctrl+Shift
times 12 db     '      '
end if

keybar_viewer:
if lang eq ru
; без клавиш-модификаторов
        db      'Помощь'
        db      'Развер'
        db      'Выход '
        db      'Код   '
        db      '      '
        db      'Редакт'
        db      'Поиск '
keybar_cp:
        db      'cp1251'
        db      '      '
        db      'Выход '
        db      'Модули'
        db      'Экраны'
; Shift
        db      '      '
        db      'Слова '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      'Дальше'
        db      'Таблиц'
        db      '      '
        db      '      '
        db      '      '
        db      '      '
; Ctrl
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      'Позиц '
        db      '      '
        db      '      '
; Ctrl+Shift
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
; Alt
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      'Печать'
        db      '      '
        db      'Назад '
        db      'Перейт'
        db      'Видео '
        db      '      '
        db      'ИстПр '
        db      '      '
; Alt+Shift
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      'Конфиг'
        db      '      '
        db      '      '
        db      '      '
; Alt+Ctrl
times 12 db     '      '
; Alt+Ctrl+Shift
times 12 db     '      '
else
; No modificators
        db      'Help  '
        db      'Unwrap'
        db      'Quit  '
        db      'Hex   '
        db      '      '
        db      'Edit  '
        db      'Search'
keybar_cp:
        db      'cp1251'
        db      '      '
        db      'Quit  '
        db      'Plugin'
        db      'Screen'
; Shift
        db      '      '
        db      'WWrap '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      'Next  '
        db      'Table '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
; Ctrl
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      'GoFile'
        db      '      '
        db      '      '
; Ctrl+Shift
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
; Alt
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      'Print '
        db      '      '
        db      'Prev  '
        db      'Goto  '
        db      'Video '
        db      '      '
        db      'ViewHs'
        db      '      '
; Alt+Shift
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      'Config'
        db      '      '
        db      '      '
        db      '      '
; Alt+Ctrl
times 12 db     '      '
; Alt+Ctrl+Shift
times 12 db     '      '
end if

        align   4
cur_width       dd      80
cur_height      dd      25
saved_width     dd      -1
saved_height    dd      -1
fill_width      dd      0
fill_height     dd      0
max_width = 256
max_height = 256
console_data_ptr dd     0

cursor_x        dd      -1
cursor_y        dd      -1
old_cursor_x    dd      -1
old_cursor_y    dd      -1

active_panel    dd      panel1_data

console_colors  dd      0x000000, 0x000080, 0x008000, 0x008080
                dd      0x800000, 0x800080, 0x808000, 0xC0C0C0
                dd      0x808080, 0x0000FF, 0x00FF00, 0x00FFFF
                dd      0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF

compare_fns     dd      compare_name
                dd      compare_name_rev
                dd      compare_ext
                dd      compare_ext_rev
                dd      compare_modified
                dd      compare_modified_rev
                dd      compare_size
                dd      compare_size_rev
                dd      compare_unordered
                dd      compare_unordered_rev
                dd      compare_created
                dd      compare_created_rev
                dd      compare_accessed
                dd      compare_accessed_rev

ctrlkey_tests   dd      ctrlkey_test0
                dd      ctrlkey_test1
                dd      ctrlkey_test2
                dd      ctrlkey_test3
                dd      ctrlkey_test4
ctrlkey_tests_num = 5

virtual at 0
screen_vtable:
        .OnRedraw       dd      ?
        .OnKey          dd      ?
        .keybar         dd      ?
        .getname        dd      ?
end virtual

panels_vtable:
        dd      panels_OnRedraw
        dd      panels_OnKey
        dd      keybar_panels
        dd      panels_getname

viewer_vtable:
        dd      viewer_OnRedraw
        dd      viewer_OnKey
        dd      keybar_viewer
        dd      viewer_getname

; additions to this table require changes in tools.inc::get_error_msg
errors1:
        dd      error0msg
        dd      error1msg
        dd      error2msg
        dd      error3msg
        dd      error4msg
        dd      error5msg
        dd      error6msg
        dd      error7msg
        dd      error8msg
        dd      error9msg
        dd      error10msg
        dd      error11msg
errors2:
        dd      error30msg
        dd      error31msg
        dd      error32msg

encodings:
.cp866 = 0
.cp1251 = 1

.names:
        db      'cp866   '
        db      'cp1251  '

.tables:
; cp866 - trivial map
        times 128 db %+127
; cp1251
        db      0x3F,0x3F,0x27,0x3F,0x22,0x3A,0xC5,0xD8,0x3F,0x25,0x3F,0x3C,0x3F,0x3F,0x3F,0x3F
        db      0x3F,0x27,0x27,0x22,0x22,0x07,0x2D,0x2D,0x3F,0x54,0x3F,0x3E,0x3F,0x3F,0x3F,0x3F
        db      0xFF,0xF6,0xF7,0x3F,0xFD,0x3F,0xB3,0x15,0xF0,0x63,0xF2,0x3C,0xBF,0x2D,0x52,0xF4
        db      0xF8,0x2B,0x49,0x69,0x3F,0xE7,0x14,0xFA,0xF1,0xFC,0xF3,0x3E,0x3F,0x3F,0x3F,0xF5
        times 0x30 db %-1+0x80
        times 0x10 db %-1+0xE0

active_screen   dd      0
tabsize         dd      8

ascii2scan:
        times 32 db 0
        db      0x39,0x02,0x03,0x04,0x05,0x06,0x08,0x28,0x0A,0x0B,0x09,0x0D,0x33,0x0C,0x34,0x35
        db      0x0B,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x27,0x27,0x33,0x0D,0x34,0x35
        db      0x03,0x1E,0x30,0x2E,0x20,0x12,0x21,0x22,0x23,0x17,0x24,0x25,0x26,0x32,0x31,0x18
        db      0x19,0x10,0x13,0x1F,0x14,0x16,0x2F,0x11,0x2D,0x15,0x2C,0x1A,0x2B,0x1B,0x07,0x0C
        db      0x29,0x1E,0x30,0x2E,0x20,0x12,0x21,0x22,0x23,0x17,0x24,0x25,0x26,0x32,0x31,0x18
        db      0x19,0x10,0x13,0x1F,0x14,0x16,0x2F,0x11,0x2D,0x15,0x2C,0x1A,0x2B,0x1B,0x29,0x00
        db      0x21,0x33,0x20,0x16,0x26,0x14,0x27,0x19,0x30,0x10,0x13,0x25,0x2F,0x15,0x24,0x22
        db      0x23,0x2E,0x31,0x12,0x1E,0x1A,0x11,0x2D,0x17,0x18,0x1B,0x1F,0x32,0x28,0x34,0x2C
        db      0x21,0x33,0x20,0x16,0x26,0x14,0x27,0x19,0x30,0x10,0x13,0x25,0x2F,0x15,0x24,0x22
        times 48 db 0
        db      0x23,0x2E,0x31,0x12,0x1E,0x1A,0x11,0x2D,0x17,0x18,0x1B,0x1F,0x32,0x28,0x34,0x2C
        db      0x29,0x29,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00

; Клавишные сочетания
; db scancode, reserved
; dw ctrlstate
; dd handler
; ctrlstate: младшие 4 бита - для Shift, следующие - для Ctrl, следующие - для Alt
; 0 = ни одна клавиша не нажата
; 1 = ровно одна нажата
; 2 = обе нажаты
; 3 = левая нажата, правая нет
; 4 = правая нажата, левая нет
panels_ctrlkeys:
        dw      0x48, 0
        dd      panels_OnKey.up
        dw      0x50, 0
        dd      panels_OnKey.down
        dw      0x4B, 0
        dd      panels_OnKey.left
        dw      0x4D, 0
        dd      panels_OnKey.right
        dw      0xF, 0
        dd      panels_OnKey.tab
        dw      0x47, 0
        dd      panels_OnKey.home
        dw      0x4F, 0
        dd      panels_OnKey.end
        dw      0x1C, 0
        dd      panels_OnKey.enter
        dw      0x3D, 0
        dd      panels_OnKey.f3
        dw      0x3F, 0
        dd      panels_OnKey.f5
        dw      0x3F, 1
        dd      panels_OnKey.shift_f5
        dw      0x41, 0
        dd      panels_OnKey.f7
        dw      0x42, 0
        dd      panels_OnKey.f8
        dw      0x43, 0x100
        dd      alt_f9
        dw      0x44, 0
        dd      exit
        dw      0x5D, 0
        dd      panels_OnKey.menu
repeat 9-3+1
        dw      0x3D+%-1, 0x10
        dd      panels_OnKey.ctrl_f39
end repeat
        dw      0x3B, 0x100
        dd      panels_OnKey.alt_f12
        dw      0x3C, 0x100
        dd      panels_OnKey.alt_f12
        dw      0x58, 0
        dd      F12
        dw      0x13, 0x10
        dd      panels_OnKey.ctrl_r
        db      0

viewer_ctrlkeys:
        dw      1, 0
        dd      viewer_OnKey.exit
        dw      0x51, 0
        dd      viewer_OnKey.pgdn
        dw      0x49, 0
        dd      viewer_OnKey.pgup
        dw      0x50, 0
        dd      viewer_OnKey.down
        dw      0x48, 0
        dd      viewer_OnKey.up
        dw      0x4B, 0
        dd      viewer_OnKey.left
        dw      0x4B, 0x10
        dd      viewer_OnKey.ctrl_left
        dw      0x4B, 0x11
        dd      viewer_OnKey.ctrl_shift_left
        dw      0x4C, 0
        dd      viewer_OnKey.exit
        dw      0x4D, 0
        dd      viewer_OnKey.right
        dw      0x4D, 0x10
        dd      viewer_OnKey.ctrl_right
        dw      0x4D, 0x11
        dd      viewer_OnKey.ctrl_shift_right
        dw      0x3C, 0
        dd      viewer_OnKey.f2
        dw      0x3D, 0
        dd      viewer_OnKey.exit
        dw      0x3E, 0
        dd      viewer_OnKey.f4
        dw      0x42, 0
        dd      viewer_OnKey.f8
        dw      0x44, 0
        dd      viewer_OnKey.exit
        dw      0x47, 0
        dd      viewer_OnKey.home
        dw      0x4F, 0
        dd      viewer_OnKey.end
        dw      0x58, 0
        dd      F12
        dw      0x43, 0x100
        dd      alt_f9
        db      0

dirinfo:
                dd      1
.first          dd      0
                dd      0
.size           dd      300
.dirdata        dd      0
                db      0
.name           dd      0

readinfo:
                dd      0
.first          dq      0
.size           dd      0
.data           dd      0
                db      0
.name           dd      0

writeinfo:
.code           dd      2
.first          dq      0
.size           dd      0
.data           dd      0
                db      0
.name           dd      0

attrinfo:
                dd      5
                dd      0
                dd      0
                dd      0
                dd      .attr
                db      0
.name           dd      0

delinfo:
                dd      8
                dd      0
                dd      0
                dd      0
                dd      0
                db      0
                dd      execdata

mkdirinfo:
                dd      9
                dd      0
                dd      0
                dd      0
                dd      0
                db      0
                dd      CopyDestEditBuf+12

if lang eq ru
compare_names   db      'иИрРмМаАнНсСдД'
else
compare_names   db      'nNxXmMsSuUcCaA'
end if

; Здесь начинаются конфигурационные данные - в текущей реализации они зашиты в бинарник

; Панель
panel_normal_color      db      1Bh
panel_border_color      db      1Bh
panel_cursor_color      db      30h
panel_header_color      db      1Bh
panel_active_header_color db    30h
column_header_color     db      1Eh
panel_nscreens_color    db      0Bh
; Диалоги
dialog_colors:
dialog_main_color       db      70h
dialog_border_color     db      70h
dialog_header_color     db      70h
dialog_edit_color       db      30h
dialog_unmodified_edit_color db 38h
dialog_normal_btn_color db      70h
dialog_selected_btn_color db    30h
; Предупреждения и ошибки
warning_colors:
; !!! должны быть те же поля и в том же порядке, что и для обычных диалогов !!!
warning_main_color      db      4Fh
warning_border_color    db      4Fh
warning_header_color    db      4Fh
warning_edit_color      db      30h
warning_unmodified_edit_color db 38h
warning_normal_btn_color db     4Fh
warning_selected_btn_color db   70h
; Меню
menu_normal_color       db      3Fh
menu_selected_color     db      0Fh
menu_highlight_color    db      3Eh
menu_selected_highlight_color db 0Eh
menu_border_color       db      3Fh
menu_header_color       db      3Fh
menu_scrollbar_color    db      3Fh
; Линейка клавиш
keybar_number_color     db      7
keybar_name_color       db      30h
keybar_bgr_color        db      7
; Командная строка
cmdbar_normal_color     db      7
cmdbar_prefix_color     db      7
; Просмотрщик
view_normal_color       db      1Bh
view_status_color       db      30h
view_arrows_color       db      1Eh

; Подсветка файлов
highlight_num_groups    dd      10
highlight_groups        dd      highlight_group0
                dd      highlight_group1
                dd      highlight_group2
                dd      highlight_group3
                dd      highlight_group4
                dd      highlight_group5
                dd      highlight_group6
                dd      highlight_group7
                dd      highlight_group8
                dd      highlight_group9

; Формат описания группы подсветки:
virtual at 0
highlight:
        .NormalColor            db ?
        .CursorColor            db ?
        .IncludeAttributes      db ?
        .Mask:                  ; ASCIIZ-string
end virtual

; all highlight masks must be in lowercase!
highlight_group0:
        db      13h
        db      38h
        db      2
        db      '*',0
highlight_group1:
        db      13h
        db      38h
        db      4
        db      '*',0
highlight_group2:
        db      1Fh
        db      3Fh
        db      10h
        db      '*|..',0
highlight_group3:
        db      0
        db      0
        db      10h
        db      '..',0
highlight_group4:
        db      1Ah
        db      3Ah
        db      0
        db      '*.exe,*.com,*.bat,*.cmd',0
highlight_group5:
        db      1Ah
        db      3Ah
        db      0
        db      '*|*.*',0
highlight_group6:
        db      1Dh
        db      3Dh
        db      0
        db      '*.rar,*.zip,*.[zj],*.[bg7]z,*.[bg]zip,*.tar,*.t[ag]z,*.ar[cj],*.r[0-9][0-9],'
        db      '*.a[0-9][0-9],*.bz2,*.cab,*.msi,*.jar,*.lha,*.lzh,*.ha,*.ac[bei],*.pa[ck],'
        db      '*.rk,*.cpio,*.rpm,*.zoo,*.hqx,*.sit,*.ice,*.uc2,*.ain,*.imp,*.777,*.ufa,*.boa,'
        db      '*.bs[2a],*.sea,*.hpk,*.ddi,*.x2,*.rkv,*.[lw]sz,*.h[ay]p,*.lim,*.sqz,*.chz',0
highlight_group7:
        db      16h
        db      36h
        db      0
        db      '*.bak,*.tmp',0
highlight_group8:
        db      17h
        db      37h
        db      0
        db      '*.asm,*.inc',0
highlight_group9:
        db      1Fh
        db      3Fh
        db      10h
        db      '*',0

associations:
        dd      aAsm, tinypad, AsmMenu
        dd      aInc, tinypad, 0
        dd      aTxt, tinypad, 0
        dd      aJpg, jpegview, 0
        dd      aJpeg, jpegview, 0
        dd      aGif, gifview, GifMenu
        dd      aWav, ac97, 0
        dd      aMp3, ac97, 0
        dd      aMid, midamp, 0
        dd      aBmp, mv, BmpMenu
        dd      aPng, archer, 0
        dd      aRtf, rtfread, 0
        dd      a3ds, view3ds, 0
        dd      aLif, life2, 0
associations_end:

aAsm db 'asm',0
aInc db 'inc',0
aTxt db 'txt',0
tinypad db '/rd/1/TinyPad',0

aJpg db 'jpg',0
aJpeg db 'jpeg',0
jpegview db '/rd/1/JpegView',0

aGif db 'gif',0
gifview db '/rd/1/GIFVIEW',0

aWav db 'wav',0
aMp3 db 'mp3',0
ac97 db '/rd/1/AC97SND',0

aMid db 'mid',0
midamp db '/rd/1/MIDAMP',0

aBmp db 'bmp',0
mv db '/rd/1/MV',0

aPng db 'png',0
archer db '/rd/1/@rcher',0

aRtf db 'rtf',0
rtfread db '/rd/1/RtfRead',0

a3ds db '3ds',0
view3ds db '/rd/1/3d/view3ds',0

aLif db 'lif',0
life2 db '/rd/1/demos/life2',0

AsmMenu:
        dd      aEdit, tinypad
        dd      aCompile, fasm
        dd      0
BmpMenu:
        dd      aView, mv
        dd      aEdit, animage
        dd      0
GifMenu:
        dd      aView, gifview
        dd      aEdit, animage
        dd      0

if lang eq en
aView   db      '&View',0
aCompile db     '&Compile',0
aEdit   db      '&Edit',0
else
aView   db      '&Просмотр',0
aCompile db     '&Компилировать',0
aEdit   db      '&Редактор',0
end if

fasm    db      '/rd/1/develop/fasm',0
animage db      '/rd/1/animage',0

bConfirmDelete  db      1
bConfirmDeleteIncomplete db 0

; Здесь заканчиваются конфигурационные данные

bWasE0          db      0
ctrlstate       db      0

align   4
; Сообщение о обломе при выделении памяти
nomem_dlgdata:
        dd      2
        dd      -1
        dd      -1
        dd      12
        dd      2
        dd      1
        dd      1
        dd      aError
        rb      4
        dd      0
        dd      0
        dd      2
; строка "No memory"
        dd      1
        dd      1,0,10,0
        dd      aNoMemory
        dd      1
; кнопка "Ok"
        dd      2
        dd      4,1,7,1
        dd      aOk
        dd      0xD

f8_confirm_dlgdata:
        dd      1
.x      dd      -1
.y      dd      -1
.width  dd      ?
.height dd      3
        dd      4
        dd      2
        dd      aDeleteCaption
.main_color db ?
.border_color db ?
.header_color db ?
        db      0
        dd      0
        dd      0
        dd      4
; строка "Вы хотите удалить ..."
        dd      1
        dd      1,0
.width2 dd      ?
        dd      0
        dd      aConfirmDeleteTextBuf
        dd      1
; строка с именем файла/папки
        dd      1
        dd      1,1
.width3 dd      ?
        dd      1
.name   dd      ?
        dd      1
; кнопка "удалить"
.del_btn:
        dd      2
.del_x1 dd      ?
        dd      2
.del_x2 dd      ?
        dd      2
        dd      aDelete
.flags1 dd      0xC
; кнопка "отменить"
        dd      2
.cnl_x1 dd      ?
        dd      2
.cnl_x2 dd      ?
        dd      2
        dd      aCancel
.flags2 dd      8

; диалог копирования
copy_dlgdata:
        dd      1
.x      dd      -1
.y      dd      -1
.width  dd      ?
.height dd      4
        dd      4
        dd      2
        dd      aCopyCaption
.main_color db ?
.border_color db ?
.header_color db ?
        db      0
        dd      0
        dd      0
        dd      4
; строка 'Копировать "%s" в:'
        dd      1
        dd      1,0
.width2 dd      ?
        dd      0
        dd      CopySourceTextBuf
        dd      0
; поле редактирования с именем файла/папки назначения
        dd      3
        dd      1,1
.width3 dd      ?
        dd      1
        dd      CopyDestEditBuf
.flags0 dd      0
; кнопка "копировать"
.copy_btn:
        dd      2
.copy_x1 dd     ?
        dd      3
.copy_x2 dd     ?
        dd      3
        dd      aCopy
.flags1 dd      18h
; кнопка "отменить"
        dd      2
.cnl_x1 dd      ?
        dd      3
.cnl_x2 dd      ?
        dd      3
        dd      aCancelB
.flags2 dd      8

mkdir_dlgdata:
        dd      1
        dd      -1, -1
.width  dd      ?
.height dd      4
        dd      4, 2
        dd      aMkDirCaption
        dd      ?
        dd      0
        dd      0
        dd      4
; Строка "Создать папку"
        dd      1
        dd      1,0,aMkDirLen,0
        dd      aMkDir
        dd      0
; поле редактирования с именем создаваемой папки
        dd      3
        dd      1,1
.width2 dd      ?
        dd      1
        dd      CopyDestEditBuf
.flags0 dd      0xC
; кнопка "Продолжить"
.cont_btn:
        dd      2
.cont_x1 dd     ?
        dd      3
.cont_x2 dd     ?
        dd      3
        dd      a_Continue
.flags1 dd      18h
; кнопка "отменить"
        dd      2
.cnl_x1 dd      ?
        dd      3
.cnl_x2 dd      ?
        dd      3
        dd      aCancelB
.flags2 dd      8

RetryOrCancelBtn:
        dd      aRetry
        dd      a_Cancel
DeleteOrKeepBtn:
        dd      a_Delete
        dd      aKeep
RetryOrIgnoreBtn:
        dd      aRetry
        dd      aIgnore

if lang eq ru
aDeleteCaption          db      'Удаление',0
aConfirmDeleteText      db      'Вы хотите удалить ',0
aDeleteFolder           db      'папку',0
aConfirmDeleteTextMax = $ - aConfirmDeleteText - 2
aDeleteFile             db      'файл',0
aDelete                 db      ' Удалить ',0
aDeleteLength = $ - aDelete - 1
aCancel                 db      ' Отменить ',0
aCancelLength = $ - aCancel - 1
aCancelB                db      '[ Отменить ]',0
aCancelBLength = $ - aCancelB - 1
aCopyCaption            db      'Копирование',0
aCopy                   db      '[ Копировать ]',0
aCopyLength = $ - aCopy - 1
a_Continue              db      '[ Продолжить ]',0
a_ContinueLength = $ - a_Continue - 1
aCopy1                  db      'Копировать "',0
aCopy2                  db      '" в:',0
aError                  db      'Ошибка',0
aContinue               db      'Продолжить',0
aRetry                  db      'Повторить',0
a_Cancel                db      'Отменить',0
a_Delete                db      'Удалить',0
error0msg               db      'Странно... Нет ошибки',0
error1msg               db      'Странно... Не определена база и/или раздел жёсткого диска',0
error2msg               db      'Функция не поддерживается для данной файловой системы',0
error3msg               db      'Неизвестная файловая система',0
error4msg               db      'Странно... Ошибка 4',0
error5msg               db      'Файл не найден',0
error6msg               db      'Файл закончился',0
error7msg               db      'Странно... Указатель вне памяти приложения',0
error8msg               db      'Диск заполнен',0
error9msg               db      'Файловая структура разрушена',0
error10msg              db      'Доступ запрещён',0
error11msg              db      'Ошибка устройства',0
error30msg              db      'Недостаточно памяти',0
error31msg              db      'Файл не является исполняемым',0
error32msg              db      'Слишком много процессов',0
aUnknownError           db      'Неизвестный код ошибки: ',0
aCannotReadFolder       db      'Не могу прочитать папку',0
aRunError               db      'Ошибка при запуске программы:',0
aFileNameTooBig         db      'Полное имя файла слишком длинное',0
aFolderNameTooBig       db      'Полное имя папки слишком длинное',0
aCmdLineTooBig          db      'Командная строка превышает границу OS в 256 символов',0
aCannotCopyToSelf       db      'Файл не может быть скопирован в самого себя',0
aCannotReadFile         db      'Не могу прочитать файл',0
aIncompleteFile         db      'Был получен неполный файл. Удалить его?',0
aKeep                   db      'Оставить',0
aCannotWriteFile        db      'Не могу записать в файл',0
aCannotDeleteFile       db      'Не могу удалить файл',0
aCannotDeleteFolder     db      'Не могу удалить папку',0
aIgnore                 db      'Игнорировать',0
aMkDirCaption           db      'Создание папки',0
aMkDir                  db      'Создать папку',0
aMkDirLen = $ - aMkDir - 1
aCannotMakeFolder       db      'Не могу создать папку',0
else
aDeleteCaption          db      'Delete',0
aConfirmDeleteText      db      'Do you wish to delete ',0
aDeleteFolder           db      'the folder',0
aConfirmDeleteTextMax = $ - aConfirmDeleteText - 2
aDeleteFile             db      'the file',0
aDelete                 db      ' Delete ',0
aDeleteLength = $ - aDelete - 1
aCancel                 db      ' Cancel ',0
aCancelLength = $ - aCancel - 1
aCancelB                db      '[ Cancel ]',0
aCancelBLength = $ - aCancelB - 1
aCopyCaption            db      'Copy',0
aCopy                   db      '[ Copy ]',0
aCopyLength = $ - aCopy - 1
a_Continue              db      '[ Continue ]',0
a_ContinueLength = $ - a_Continue - 1
aCopy1                  db      'Copy "',0
aCopy2                  db      '" to:',0
aError                  db      'Error',0
aContinue               db      'Continue',0
aRetry                  db      'Retry',0
a_Cancel                db      'Cancel',0
a_Delete                db      'Delete',0
error0msg               db      'Strange... No error',0
error1msg               db      'Strange... Hard disk base and/or partition not defined',0
error2msg               db      'The file system does not support this function',0
error3msg               db      'Unknown file system',0
error4msg               db      'Strange... Error 4',0
error5msg               db      'File not found',0
error6msg               db      'End of file',0
error7msg               db      'Strange... Pointer lies outside of application memory',0
error8msg               db      'Disk is full',0
error9msg               db      'File structure is destroyed',0
error10msg              db      'Access denied',0
error11msg              db      'Device error',0
error30msg              db      'Not enough memory',0
error31msg              db      'File is not executable',0
error32msg              db      'Too many processes',0
aUnknownError           db      'Unknown error code: ',0
aCannotReadFolder       db      'Cannot read folder',0
aRunError               db      'Cannot execute program:',0
aFileNameTooBig         db      'Full file name is too long',0
aFolderNameTooBig       db      'Full folder name is too long',0
aCmdLineTooBig          db      'Command line is too long (OS limit is 256 symbols)',0
aCannotCopyToSelf       db      'File cannot be copied onto itself',0
aCannotReadFile         db      'Cannot read file',0
aIncompleteFile         db      'Incomplete file was retrieved. Delete it?',0
aKeep                   db      'Keep',0
aCannotWriteFile        db      'Cannot write file',0
aCannotDeleteFile       db      'Cannot delete file',0
aCannotDeleteFolder     db      'Cannot delete folder',0
aIgnore                 db      'Ignore',0
aMkDirCaption           db      'Make folder',0
aMkDir                  db      'Create the folder',0
aMkDirLen = $ - aMkDir - 1
aCannotMakeFolder       db      'Cannot create folder',0
end if
aOk                     db      'OK',0
aNoMemory               db      'No memory!',0

execinfo:
        dd      7
        dd      0
execparams dd   0
        dd      0
        dd      0
        db      0
execptr dd      ?

IncludeIGlobals

i_end:

IncludeUGlobals

execdata rb     1024
execdataend:
        align   4
attrinfo.attr   rb      40

panel1_data:
panel1_left     dd      ?
panel1_top      dd      ?
panel1_width    dd      ?
panel1_height   dd      ?
panel1_index    dd      ?
panel1_start    dd      ?
panel1_colsz    dd      ?
panel1_colst    dd      ?
panel1_sortmode db      ?
                rb      3
panel1_nfa      dd      ?
panel1_numfiles dd      ?
panel1_files    dd      ?
panel1_dir      rb      1024

panel2_data:
panel2_left     dd      ?
panel2_top      dd      ?
panel2_width    dd      ?
panel2_height   dd      ?
panel2_index    dd      ?
panel2_start    dd      ?
panel2_colsz    dd      ?
panel2_colst    dd      ?
panel2_sortmode db      ?
                rb      3
panel2_nfa      dd      ?
panel2_numfiles dd      ?
panel2_files    dd      ?
panel2_dir      rb      1024

;console_data    rb      max_width*max_height*2

nomem_dlgsavearea       rb      (12+4)*(3+3)*2

cur_header      rb      max_width
tmp             dd      ?

skinh           dd      ?
std_colors      rd      10

min_y           dd      ?
max_y           dd      ?
min_x           dd      ?
max_x           dd      ?
used_width      dd      ?
used_height     dd      ?

wnd_width       dd      ?
wnd_height      dd      ?

column_left     dd      ?
column_top      dd      ?
column_width    dd      ?
column_height   dd      ?
column_index    dd      ?

scrpos          dq      ?
viewer_right_side dq    ?

saved_file_name:
procinfo        rb      1024
lower_file_name = procinfo + 512

error_msg       rb      128

prev_dir        rb      1024

driveinfo       rb      32+304
tmpname         rb      32

screens         dd      ?
num_screens     dd      ?
active_screen_vtable dd ?
active_screen_data dd   ?

aConfirmDeleteTextBuf   rb      aConfirmDeleteTextMax + 1
CopySourceTextBuf       rb      512
CopyDestEditBuf         rb      12+512+1
.length = $ - CopyDestEditBuf - 13

align 4
layout          rb      128

copy_buffer_size = 32768
copy_buffer     rb      copy_buffer_size

; stack
        align   4
        rb      512
stacktop:

mem:
