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

start:
        mov     eax, mem
        call    mf_init
        call    draw_window
        push    66
        pop     eax
        push    1
        pop     ebx
        mov     ecx, ebx
        int     40h     ; set keyboard mode to scancodes
        call    init_console
        mov     esi, def_left_dir
        mov     edi, panel1_dir
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
@@:
        mov     esi, def_right_dir
        mov     edi, panel2_dir
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        mov     eax, 304
        mov     [panel1_nfa], eax
        mov     [panel2_nfa], eax
        mov     [panel1_files], buf1
        mov     [panel2_files], buf2
        mov     [panel1_sortmode], 0    ; sort by name
        mov     [panel2_sortmode], 0
        mov     [num_screens], 1
        mov     eax, 8
        call    mf_alloc
        mov     [screens], eax
        mov     ecx, panels_vtable
        mov     [eax], ecx
        mov     [active_screen_vtable], ecx
        call    draw_keybar
        call    draw_cmdbar
        mov     ebp, panel1_data
        call    read_folder
        call    draw_panel
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
        mov     edx, 0x13000000
        int     0x40
        mov     al, 12
        inc     ebx
        int     0x40
        jmp     event
@@:
        xor     ecx, ecx
        mov     eax, [ebx+42]
        sub     eax, 5*2
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
        setnz   ch
        or      cl, ch
        mov     eax, [ebx+46]
        sub     eax, [skinh]
        sub     eax, 5
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
        cmp     eax, [cur_height]
        mov     [cur_height], eax
        setnz   ch
        or      cl, ch
        test    edx, edx
        setnz   ch
        test    cx, cx
        jz      @f
        mov     eax, [MemForImage]
        call    mf_free
        and     [MemForImage], 0
        call    init_console
        push    67
        pop     eax
        or      ebx, -1
        or      ecx, -1
        mov     edx, [cur_width]
        imul    edx, font_width
        add     edx, 5*2
        mov     esi, [cur_height]
        imul    esi, font_height
        add     esi, [skinh]
        add     esi, 5
        int     40h
        call    draw_window
        call    draw_keybar
        mov     ebp, [active_screen_data]
        mov     eax, [active_screen_vtable]
        call    dword [eax+screen_vtable.OnRedraw]
        jmp     event
@@:
        call    draw_window
        jmp     event
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

; TODO: add "no memory" error handling
new_screen:
        call    mf_alloc
        test    eax, eax
        jnz     @f
        ret
@@:
        mov     ebp, eax
        mov     ebx, [num_screens]
        inc     ebx
        shl     ebx, 3
        mov     eax, [screens]
        call    mf_realloc
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
        call    mf_realloc
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
        call    mf_alloc
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
; find extension
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
        jmp     .run_app
.found_ext:
        inc     esi
        mov     edi, associations
@@:
        push    esi edi
        mov     edi, [edi]
        call    strcmpi
        pop     edi esi
        jz      .run_association
        add     edi, 8
        cmp     edi, associations_end
        jb      @b
        jmp     .run_app
.run_association:
        mov     [execparams], execdata
        mov     eax, [edi+4]
        mov     [execptr], eax
        jmp     .dorun
.run_app:
        mov     [execptr], execdata
        and     [execparams], 0
.dorun:
        pop     esi
        lea     esi, [ebp + panel1_dir - panel1_data]
        mov     edi, execdata
; TODO: add overflow check
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
        stosb
        test    al, al
        jnz     @b
        push    70
        pop     eax
        mov     ebx, execinfo
        int     40h
        ret
.enter_folder:
        lea     esi, [ecx+40]
        cmp     word [esi], '..'
        jnz     @f
        cmp     byte [esi+2], 0
        jz      .dotdot
@@:
        lea     edi, [ebp + panel1_dir - panel1_data]
        mov     al, 0
        or      ecx, -1
        repnz   scasb
        dec     edi
        mov     al, '/'
        cmp     [edi-1], al
        jz      @f
        stosb
@@:
; TODO: add buffer overflow check
@@:
        lodsb
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
        mov     eax, [ebp + panel1_nfa - panel1_data]
        lea     esi, [esi+eax*4+32+40]
        add     esi, [ebp + panel1_files - panel1_data]
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
        call    mf_alloc
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
        mov     al, [dialog_border_color]
        mov     [ebx + dlgtemplate.border_color], al
        mov     al, [dialog_header_color]
        mov     [ebx + dlgtemplate.header_color], al
        mov     al, [dialog_main_color]
        mov     [ebx + dlgtemplate.main_color], al
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
        stosb
        test    al, al
        jnz     @b
        push    70
        pop     eax
        mov     ebx, delinfo
        int     0x40
.ctrl_r:
; Rescan panel
;       call    read_folder
;       jmp     .done_redraw
        mov     eax, [ebp + panel1_index - panel1_data]
        push    eax
        call    get_curfile_name
        mov     esi, eax
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
        mov     esi, [ebp + panel1_nfa - panel1_data]
        mov     ecx, [ebp + panel1_files - panel1_data]
        lea     esi, [ecx+esi*4+32+40]
        add     esi, [ecx+eax*4]
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

panels_OnRedraw:
        call    draw_cmdbar
        mov     ebp, panel1_data
        call    draw_panel
        mov     ebp, panel2_data
        call    draw_panel
        ret

init_console:
        mov     ax, 0720h
        mov     ecx, [cur_width]
        imul    ecx, [cur_height]
        mov     edi, console_data
        rep     stosw
        and     [panel1_left], 0
        and     [panel1_top], 0
        and     [panel2_top], 0
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
        push    eax
        mov     ecx, [ebp + panel1_index - panel1_data]
        mov     eax, [ebp + panel1_files - panel1_data]
        mov     ecx, [eax+ecx*4]
        mov     eax, [ebp + panel1_nfa - panel1_data]
        lea     ecx, [ecx+eax*4+32]
        add     ecx, [ebp + panel1_files - panel1_data]
        pop     eax
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
        mov     edx, [ebp + panel1_files - panel1_data]
        mov     esi, [ebp + panel1_index - panel1_data]
        mov     esi, [edx+esi*4]
        add     esi, edx
        mov     edx, [ebp + panel1_nfa - panel1_data]
        lea     esi, [esi+edx*4+32+40]
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
        add     ebx, 100*65536 + 5*2
        mov     ecx, [cur_height]
        imul    ecx, font_height
        lea     ecx, [eax+ecx+5+100*65536]
        xor     eax, eax
        mov     edx, 0x13000000
        mov     edi, header
        int     40h
        mov     al, 48
        push    3
        pop     ebx
        mov     ecx, std_colors
        push    40
        pop     edx
        int     40h
;        mov     bl, 7
;        int     40h
;        xor     ax, ax
;        shr     ebx, 16
;        or      ebx, eax
;        mov     ecx, [std_colors+16]
;        mov     edx, header
;        push    header.length
;        pop     esi
;        push    4
;        pop     eax
;        int     40h
        xor     ecx, ecx
        call    draw_image
        push    12
        pop     eax
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
        cmp     [MemForImage], 0
        jnz     .allocated
; allocate memory for image
        mov     eax, [cur_width]
        imul    eax, [cur_height]
        imul    eax, font_width*font_height*3
        call    mf_alloc
        test    eax, eax
        jz      draw_image.nomem
        mov     [MemForImage], eax
.allocated:
        mov     edi, [MemForImage]
        mov     esi, console_data
        mov     ecx, [cur_height]
.lh:
        push    ecx
        mov     ecx, [cur_width]
.lw:
        push    ecx edi
        xor     eax, eax
        mov     al, [esi+1]
        and     al, 0xF
        mov     ebx, [console_colors + eax*4]   ; цвет текста
        mov     al, [esi+1]
        shr     al, 4
        mov     ebp, [console_colors + eax*4]   ; цвет фона
        lodsb
        inc     esi
if font_width > 8
        lea     edx, [eax+eax+font]
else
        lea     edx, [eax+font]
end if
        mov     ecx, font_height
.sh:
        push    ecx edi
        xor     ecx, ecx
.sw:
        mov     eax, ebx
        bt      [edx], ecx
        jc      @f
        mov     eax, ebp
@@:
        stosw
        shr     eax, 16
        stosb
        inc     ecx
        cmp     ecx, font_width
        jb      .sw
        pop     edi ecx
        mov     eax, [cur_width]
        imul    eax, font_width*3
        add     edi, eax
if font_width > 8
        add     edx, 256*2
else
        add     edx, 256
end if
        loop    .sh
        pop     edi ecx
        add     edi, font_width*3
        loop    .lw
        mov     eax, [cur_width]
        imul    eax, (font_height-1)*font_width*3
        add     edi, eax
        pop     ecx
        loop    .lh
        push    7
        pop     eax
        mov     ebx, [MemForImage]
        mov     ecx, [cur_width]
        imul    ecx, font_width*10000h
        mov     cx, word [cur_height]
        imul    cx, font_height
        mov     edx, [skinh]
        add     edx, 5*10000h
        int     40h
        ret

get_console_ptr:
; in: eax=x, edx=y
; out: edi->console data
        push    edx
        imul    edx, [cur_width]
        add     edx, eax
        lea     edi, [console_data + edx*2]
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
        mov     eax, [cur_width]
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
        mov     eax, [ebp + panel1_nfa - panel1_data]
        lea     ebx, [ebx+eax*4+32]
        add     ebx, [ebp + panel1_files - panel1_data]
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
        mov     ecx, [ebp + panel1_nfa - panel1_data]
        lea     esi, [esi + ecx*4 + 32 + 40]
        add     esi, [ebp + panel1_files - panel1_data]
; подсветка
;        call    insert_last_dot
        xor     ecx, ecx
.highlight_test_loop:
        mov     ebx, [highlight_groups+ecx*4]
        mov     al, [ebx + highlight.IncludeAttributes]
        mov     ah, [esi - 40]
        and     ah, al
        cmp     ah, al
        jnz     .highlight_test_failed
        push    edi
        lea     edi, [ebx + highlight.Mask]
        call    match_mask
        pop     edi
        jc      .highlight_test_failed
        mov     ah, [ebx + highlight.NormalColor]
        cmp     ebp, [active_panel]
        jnz     @f
        mov     ecx, [column_index]
        cmp     ecx, [ebp + panel1_index - panel1_data]
        jnz     @f
        mov     ah, [ebx + highlight.CursorColor]
@@:
        test    ah, ah
        jz      .nohighlight
        jmp     .doname
.highlight_test_failed:
        inc     ecx
        cmp     ecx, [highlight_num_groups]
        jb      .highlight_test_loop
.nohighlight:
        mov     ah, [panel_normal_color]
        cmp     ebp, [active_panel]
        jnz     @f
        mov     ecx, [column_index]
        cmp     ecx, [ebp + panel1_index - panel1_data]
        jnz     @f
        mov     ah, [panel_cursor_color]
@@:
.doname:
;        call    delete_last_dot
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
        push    70
        pop     eax
        mov     ebx, dirinfo
        int     40h
        test    eax, eax
        jz      .ok
        cmp     eax, 6
        jz      .ok
; TODO: add error handling
        mov     [ebp + panel1_numfiles - panel1_data], 2
        mov     eax, [ebp + panel1_nfa - panel1_data]
        shl     eax, 2
        add     eax, [ebp + panel1_files - panel1_data]
        add     eax, 32+40
        mov     word [eax], '..'
        mov     byte [eax+2], 0
        add     eax, 304
        mov     dword [eax], 'Read'
        mov     dword [eax+4], ' err'
        mov     dword [eax+8], 'or'
        mov     eax, [ebp + panel1_files - panel1_data]
        mov     dword [eax], 0
        mov     dword [eax+4], 304
        and     dword [ebp + panel1_index - panel1_data], 0
        and     dword [ebp + panel1_start - panel1_data], 0
        ret
.ok:
        mov     eax, [dirinfo.dirdata]
        cmp     [eax+8], ebx
        jz      .readdone
        push    eax
        mov     eax, [ebp + panel1_files - panel1_data]
        cmp     eax, buf1
        jz      @f
        cmp     eax, buf2
        jz      @f
        call    mf_free
@@:
        pop     eax
        mov     eax, [eax+8]
        add     eax, 0xF
        and     eax, not 0xF
        push    eax
        imul    eax, 4+304
        add     eax, 32
        call    mf_alloc
        test    eax, eax
        jnz     .succ1
        pop     eax
; TODO: add error handling
        jmp     .readdone
.succ1:
        mov     [ebp + panel1_files - panel1_data], eax
        pop     [ebp + panel1_nfa - panel1_data]
        jmp     read_folder
.readdone:
        and     [ebp + panel1_start - panel1_data], 0
        and     [ebp + panel1_index - panel1_data], 0
        and     [ebp + panel1_start - panel1_data], 0
        mov     edi, [ebp + panel1_files - panel1_data]
        mov     eax, [ebp + panel1_nfa - panel1_data]
        lea     esi, [edi + eax*4 + 32]
        xor     eax, eax
        mov     ecx, [esi-32+4]
        jecxz   .loopdone
; Игнорируем специальные входы, соответствующие папке '.' и метке тома
.ptrinit:
        cmp     word [esi+eax+40], '.'
        jz      .loopcont
        test    byte [esi+eax], 8
        jnz     .loopcont
        stosd
.loopcont:
        add     eax, 304
        loop    .ptrinit
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
        call    sort
        ret

compare_name:
        push    eax
        mov     eax, [ebp + panel1_nfa - panel1_data]
        add     esi, [ebp + panel1_files - panel1_data]
        add     edi, [ebp + panel1_files - panel1_data]
        lea     esi, [esi+eax*4+0x20]
        lea     edi, [edi+eax*4+0x20]
        pop     eax
        cmp     word [esi+40], '..'
        jnz     @f
        cmp     byte [esi+42], 0
        jz      .less
@@:     cmp     word [edi+40], '..'
        jnz     @f
        cmp     byte [edi+42], 0
        jz      .greater
@@:
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
        push    eax
        mov     eax, [ebp + panel1_nfa - panel1_data]
        add     esi, [ebp + panel1_files - panel1_data]
        add     edi, [ebp + panel1_files - panel1_data]
        lea     esi, [esi+eax*4+0x20]
        lea     edi, [edi+eax*4+0x20]
        pop     eax
        cmp     word [esi+40], '..'
        jnz     @f
        cmp     byte [esi+42], 0
        jz      .less
@@:     cmp     word [edi+40], '..'
        jnz     @f
        cmp     byte [edi+42], 0
        jz      .greater
@@:
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
        push    eax
        mov     eax, [ebp + panel1_nfa - panel1_data]
        add     esi, [ebp + panel1_files - panel1_data]
        add     edi, [ebp + panel1_files - panel1_data]
        lea     esi, [esi+eax*4+0x20]
        lea     edi, [edi+eax*4+0x20]
        pop     eax
        cmp     word [esi+40], '..'
        jnz     @f
        cmp     byte [esi+42], 0
        jz      .less
@@:     cmp     word [edi+40], '..'
        jnz     @f
        cmp     byte [edi+42], 0
        jz      .greater
@@:
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
        push    eax
        mov     eax, [ebp + panel1_nfa - panel1_data]
        add     esi, [ebp + panel1_files - panel1_data]
        add     edi, [ebp + panel1_files - panel1_data]
        lea     esi, [esi+eax*4+0x20]
        lea     edi, [edi+eax*4+0x20]
        pop     eax
        cmp     word [esi+40], '..'
        jnz     @f
        cmp     byte [esi+42], 0
        jz      .less
@@:     cmp     word [edi+40], '..'
        jnz     @f
        cmp     byte [edi+42], 0
        jz      .greater
@@:
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
        push    eax
        mov     eax, [ebp + panel1_nfa - panel1_data]
        add     esi, [ebp + panel1_files - panel1_data]
        add     edi, [ebp + panel1_files - panel1_data]
        lea     esi, [esi+eax*4+0x20]
        lea     edi, [edi+eax*4+0x20]
        pop     eax
        cmp     word [esi+40], '..'
        jnz     @f
        cmp     byte [esi+42], 0
        jz      .less
@@:     cmp     word [edi+40], '..'
        jnz     @f
        cmp     byte [edi+42], 0
        jz      .greater
@@:
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
        push    eax
        mov     eax, [ebp + panel1_nfa - panel1_data]
        add     esi, [ebp + panel1_files - panel1_data]
        add     edi, [ebp + panel1_files - panel1_data]
        lea     esi, [esi+eax*4+0x20]
        lea     edi, [edi+eax*4+0x20]
        pop     eax
        cmp     word [esi+40], '..'
        jnz     @f
        cmp     byte [esi+42], 0
        jz      .less
@@:     cmp     word [edi+40], '..'
        jnz     @f
        cmp     byte [edi+42], 0
        jz      .greater
@@:
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
        push    eax
        mov     eax, [ebp + panel1_nfa - panel1_data]
        add     esi, [ebp + panel1_files - panel1_data]
        add     edi, [ebp + panel1_files - panel1_data]
        lea     esi, [esi+eax*4+0x20]
        lea     edi, [edi+eax*4+0x20]
        pop     eax
        cmp     word [esi+40], '..'
        jnz     @f
        cmp     byte [esi+42], 0
        jz      .less
@@:     cmp     word [edi+40], '..'
        jnz     @f
        cmp     byte [edi+42], 0
        jz      .greater
@@:
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
        push    eax
        mov     eax, [ebp + panel1_nfa - panel1_data]
        add     esi, [ebp + panel1_files - panel1_data]
        add     edi, [ebp + panel1_files - panel1_data]
        lea     esi, [esi+eax*4+0x20]
        lea     edi, [edi+eax*4+0x20]
        pop     eax
        cmp     word [esi+40], '..'
        jnz     @f
        cmp     byte [esi+42], 0
        jz      .less
@@:     cmp     word [edi+40], '..'
        jnz     @f
        cmp     byte [edi+42], 0
        jz      .greater
@@:
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
        push    eax
        mov     eax, [ebp + panel1_nfa - panel1_data]
        add     esi, [ebp + panel1_files - panel1_data]
        add     edi, [ebp + panel1_files - panel1_data]
        lea     esi, [esi+eax*4+0x20]
        lea     edi, [edi+eax*4+0x20]
        pop     eax
        cmp     word [esi+40], '..'
        jnz     @f
        cmp     byte [esi+42], 0
        jz      .less
@@:     cmp     word [edi+40], '..'
        jnz     @f
        cmp     byte [edi+42], 0
        jz      .greater
@@:
        cmp     edi, esi
        ret
.greater:
        test    esi, esi
        ret
.less:
        xor     edi, edi
        stc
        ret

compare_created:
        push    eax
        mov     eax, [ebp + panel1_nfa - panel1_data]
        add     esi, [ebp + panel1_files - panel1_data]
        add     edi, [ebp + panel1_files - panel1_data]
        lea     esi, [esi+eax*4+0x20]
        lea     edi, [edi+eax*4+0x20]
        pop     eax
        cmp     word [esi+40], '..'
        jnz     @f
        cmp     byte [esi+42], 0
        jz      .less
@@:     cmp     word [edi+40], '..'
        jnz     @f
        cmp     byte [edi+42], 0
        jz      .greater
@@:
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
        push    eax
        mov     eax, [ebp + panel1_nfa - panel1_data]
        add     esi, [ebp + panel1_files - panel1_data]
        add     edi, [ebp + panel1_files - panel1_data]
        lea     esi, [esi+eax*4+0x20]
        lea     edi, [edi+eax*4+0x20]
        pop     eax
        cmp     word [esi+40], '..'
        jnz     @f
        cmp     byte [esi+42], 0
        jz      .less
@@:     cmp     word [edi+40], '..'
        jnz     @f
        cmp     byte [edi+42], 0
        jz      .greater
@@:
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
        push    eax
        mov     eax, [ebp + panel1_nfa - panel1_data]
        add     esi, [ebp + panel1_files - panel1_data]
        add     edi, [ebp + panel1_files - panel1_data]
        lea     esi, [esi+eax*4+0x20]
        lea     edi, [edi+eax*4+0x20]
        pop     eax
        cmp     word [esi+40], '..'
        jnz     @f
        cmp     byte [esi+42], 0
        jz      .less
@@:     cmp     word [edi+40], '..'
        jnz     @f
        cmp     byte [edi+42], 0
        jz      .greater
@@:
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
        push    eax
        mov     eax, [ebp + panel1_nfa - panel1_data]
        add     esi, [ebp + panel1_files - panel1_data]
        add     edi, [ebp + panel1_files - panel1_data]
        lea     esi, [esi+eax*4+0x20]
        lea     edi, [edi+eax*4+0x20]
        pop     eax
        cmp     word [esi+40], '..'
        jnz     @f
        cmp     byte [esi+42], 0
        jz      .less
@@:     cmp     word [edi+40], '..'
        jnz     @f
        cmp     byte [edi+42], 0
        jz      .greater
@@:
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


header  db      'Kolibri Far 0.14'
;.length = $ - header
        db      0

nomem_draw      db      'No memory for redraw.',0
.size = $ - nomem_draw

def_left_dir    db      '/rd/1',0
def_right_dir   db      '/hd0/1',0

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
max_width = 256
max_height = 256

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
        dw      0x42, 0
        dd      panels_OnKey.f8
        dw      0x44, 0
        dd      exit
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
dialog_main_color       db      70h
dialog_border_color     db      70h
dialog_header_color     db      70h
dialog_normal_btn_color db      70h
dialog_selected_btn_color db    30h
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
        dd      aAsm, tinypad
        dd      aInc, tinypad
        dd      aTxt, tinypad
        dd      aJpg, jpegview
        dd      aJpeg, jpegview
        dd      aGif, gifview
        dd      aWav, ac97wav
        dd      aMp3, ac97wav
        dd      aMid, midamp
        dd      aBmp, mv
        dd      aPng, archer
        dd      aRtf, rtfread
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
ac97wav db '/rd/1/AC97WAV',0

aMid db 'mid',0
midamp db '/rd/1/MIDAMP',0

aBmp db 'bmp',0
mv db '/rd/1/MV',0

aPng db 'png',0
archer db '/rd/1/@rcher',0

aRtf db 'rtf',0
rtfread db '/rd/1/RtfRead',0

bConfirmDelete  db      1

; Здесь заканчиваются конфигурационные данные

bWasE0          db      0
ctrlstate       db      0
MemForImage     dd      0

align   4
f8_confirm_dlgdata:
        dd      0
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
.flags1 dd      4
; кнопка "отменить"
        dd      2
.cnl_x1 dd      ?
        dd      2
.cnl_x2 dd      ?
        dd      2
        dd      aCancel
.flags2 dd      0

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
end if

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

console_data    rb      max_width*max_height*2

cur_header      rb      max_width
tmp             dd      ?

skinh           dd      ?
std_colors      rd      10

column_left     dd      ?
column_top      dd      ?
column_width    dd      ?
column_height   dd      ?
column_index    dd      ?

scrpos          dq      ?
viewer_right_side dq    ?

saved_file_name:
procinfo        rb      1024

driveinfo       rb      32+304
tmpname         rb      32

screens         dd      ?
num_screens     dd      ?
active_screen_vtable dd ?
active_screen_data dd   ?

aConfirmDeleteTextBuf   rb      aConfirmDeleteTextMax + 1

; stack
        align   4
        rb      512
stacktop:
; buffers for directory - may be resized dynamically
buf1    rb      4*304 + 32 + 304*304
buf2    rb      4*304 + 32 + 304*304

mem:
