use32
        db      'MENUET01'
        dd      1
        dd      start
        dd      i_end
memsize dd      mem
        dd      stacktop
        dd      0, app_path

version equ '0.64'
version_dword equ 0*10000h + 64

min_width = 54
max_width = 255
min_height = 8
max_height = 255

include 'lang.inc'
include 'font.inc'
include 'sort.inc'
include 'kglobals.inc'
include 'dialogs.inc'
include 'search.inc'
include 'viewer.inc'
include 'editor.inc'
include 'tools.inc'
include 'filetool.inc'

cursor_normal_size = (font_height*15+50)/100
cursor_big_size = font_height

start:
        mov     edi, identical_table
        mov     ecx, 0x100
        xor     eax, eax
@@:
        stosb
        inc     eax
        loop    @b
        mov     cl, 'A'
@@:
        stosb
        inc     eax
        loop    @b
        add     al, 0x20
        mov     cl, 'Z'-'A'+1
@@:
        stosb
        inc     eax
        loop    @b
        sub     al, 0x20
        mov     cl, 'А'-'Z'-1
@@:
        stosb
        inc     eax
        loop    @b
        add     al, 0x20
        mov     cl, 'Р'-'А'
@@:
        stosb
        inc     eax
        loop    @b
        add     al, 0x30
        mov     cl, 'Я'-'Р'+1
@@:
        stosb
        inc     eax
        loop    @b
        sub     al, 0x50
        mov     cl, 256-'Я'-1
@@:
        stosb
        inc     eax
        loop    @b
        mov     byte [edi-256+'Ё'], 'ё'
        mov     cl, 'A'
        inc     eax
        rep     stosb
        mov     cl, 'Z'-'A'+1
        dec     eax
        rep     stosb
        mov     cl, 'a'-'Z'-1
        inc     eax
        rep     stosb
        mov     byte [edi-'a'+'_'], 0
        mov     cl, 'z'-'a'+1
        dec     eax
        rep     stosb
        mov     cl, 'А'-'z'-1
        inc     eax
        rep     stosb
        mov     cl, 'п'-'А'+1
        dec     eax
        rep     stosb
        mov     cl, 'р'-'п'-1
        inc     eax
        rep     stosb
        mov     cl, 'ё'-'р'+1
        dec     eax
        rep     stosb
        mov     cl, 256-'ё'-1
        inc     eax
        rep     stosb
        mov     edi, SearchStringEditBuf
        mov     eax, SearchStringEditBuf.length
        stosd
        xor     eax, eax
        stosd
        stosd
        stosb
        push    68
        pop     eax
        push    11
        pop     ebx
        int     0x40
        call    init_console
        call    draw_window
        push    66
        pop     eax
        push    1
        pop     ebx
        mov     ecx, ebx
        int     40h     ; set keyboard mode to scancodes
        mov     eax, 200
        mov     [panel1_nfa], eax
        mov     [panel2_nfa], eax
        mov     ecx, 200*4 + 32 + 200*304
        call    pgalloc
        mov     [panel1_files], eax
        call    pgalloc
        mov     [panel2_files], eax
        test    eax, eax
        jz      exit
        xor     eax, eax
        cmp     [panel1_files], eax
        jz      exit
        mov     [panel1_hPlugin], eax
        mov     [panel1_parents], eax
        mov     [panel1_parents_sz], eax
        mov     [panel1_parents_alloc], eax
        mov     [panel2_hPlugin], eax
        mov     [panel2_parents], eax
        mov     [panel2_parents_sz], eax
        mov     [panel2_parents_alloc], eax
        mov     [panel1_sortmode], al   ; sort by name
        mov     [panel2_sortmode], al
        mov     al, 2                   ; "средний" формат
        mov     [panel1_colmode], eax
        mov     [panel2_colmode], eax
        mov     [num_screens], 1
        mov     ecx, 0x1000
        call    pgalloc
        mov     [screens], eax
        test    eax, eax
        jz      exit
        mov     ecx, panels_vtable
        mov     [eax], ecx
        mov     [active_screen_vtable], ecx
        mov     [active_screen_keybar], keybar_panels
; load libini.obj and kfar.ini
        mov     eax, libini_name
        mov     esi, ini_import
        push    6
        pop     ebp     ; we use version 6 of libini
        call    load_dll_and_import
        test    eax, eax
        jnz     .noini
        mov     edi, app_path
        or      ecx, -1
        repnz   scasb
        mov     dword [edi-1], '.ini'
        mov     byte [edi+3], 0
        push    1
        push    dword aConfirmDelete
        push    dword aConfirmations
        push    dword app_path
        call    [ini.get_int]
        mov     [bConfirmDelete], al
        push    0
        push    dword aConfirmDeleteIncomplete
        push    dword aConfirmations
        push    dword app_path
        call    [ini.get_int]
        mov     [bConfirmDeleteIncomplete], al
        push    2
        push    aLeftViewMode
        push    aPanels
        push    app_path
        call    [ini.get_int]
        cmp     eax, 1
        jb      @f
        cmp     eax, 4
        ja      @f
        mov     [panel1_colmode], eax
@@:
        push    2
        push    aRightViewMode
        push    aPanels
        push    app_path
        call    [ini.get_int]
        cmp     eax, 1
        jb      @f
        cmp     eax, 4
        ja      @f
        mov     [panel2_colmode], eax
@@:
        push    0
        push    aLeftSortMode
        push    aPanels
        push    app_path
        call    [ini.get_int]
        cmp     eax, 14
        jae     @f
        mov     [panel1_sortmode], al
@@:
        push    0
        push    aRightSortMode
        push    aPanels
        push    app_path
        call    [ini.get_int]
        cmp     eax, 14
        jae     @f
        mov     [panel2_sortmode], al
@@:
        push    nullstr
        push    512
        push    saved_file_name
        push    aEolStyle
        push    aEditor
        push    app_path
        call    [ini.get_str]
        mov     cl, edit.eol_dos
        mov     al, [saved_file_name]
        or      al, 20h
        cmp     al, 'd'
        jz      @f
        mov     cl, edit.eol_mac
        cmp     al, 'm'
        jz      @f
        mov     cl, edit.eol_unix
@@:
        mov     [EditEOLStyle], cl
        mov     ecx, 0x1000
        call    xpgalloc
        test    eax, eax
        jz      .skip_assoc
        mov     [associations], eax
        push    enum_associations_callback
        push    aAssociations
        push    app_path
        call    [ini.enum_keys]
.skip_assoc:
        xor     ebp, ebp
        xor     esi, esi
.shortcut_loop:
        mov     edi, saved_file_name
        push    dword nullstr
        push    512
        push    edi
        push    dword aShortcut
        push    dword aFolderShortcuts
        push    dword app_path
        call    [ini.get_str]
        cmp     byte [edi], 0
        jz      .shortcut_cont
        or      ecx, -1
        xor     eax, eax
        repnz   scasb
        not     ecx
        cmp     ecx, 2
        jz      @f
        cmp     byte [edi-2], '/'
        jnz     @f
        mov     byte [edi-2], 0
        dec     ecx
@@:
        mov     al, [aShortcut.d]
        push    esi
        inc     esi
        mov     [FolderShortcuts+(eax-'0')*4], esi
        lea     esi, [esi+ecx-1]
        push    esi
        and     esi, 0xFFF
        cmp     esi, ecx
        pop     esi
        ja      .norealloc
        mov     edx, ebp
        mov     ecx, esi
        call    xpgrealloc
        test    eax, eax
        jnz     @f
        pop     esi
        mov     edi, FolderShortcuts
        mov     ecx, 10
        rep     stosd
        jmp     .skip_shortcuts
@@:
        mov     ebp, eax
.norealloc:
        pop     edi
        add     edi, ebp
        mov     ecx, saved_file_name
@@:
        mov     al, [ecx]
        inc     ecx
        stosb
        test    al, al
        jnz     @b
.shortcut_cont:
        inc     [aShortcut.d]
        cmp     [aShortcut.d], '9'
        jbe     .shortcut_loop
        mov     esi, FolderShortcuts
        mov     ecx, 10
        dec     ebp
.l3:
        lodsd
        test    eax, eax
        jz      @f
        add     [esi-4], ebp
@@:
        loop    .l3
.skip_shortcuts:
; load plugins
        push    enum_plugins_callback
        push    aPlugins
        push    app_path
        call    [ini.enum_keys]
; calculate info for editor
        xor     ebx, ebx
        mov     ecx, [num_plugins]
        mov     esi, [plugins]
@@:
        dec     ecx
        js      @f
        mov     [esi+PluginInfo.EditInfoOffs], ebx
        add     ebx, [esi+PluginInfo.EditInfoSize]
        jnc     @f
        sbb     ebx, ebx
@@:
; поскольку размер блока в редакторе ограничен, то и размер памяти, резервируемой для плагинов, ограничен
; 512 - грубая верхняя оценка, гарантирующая работу редактора, реально плагины должны требовать меньше
        cmp     ebx, 512
        jb      @f
        or      ebx, -1
        mov     [EditPlugInfo], ebx     ; disable plugins for editor
        push    ContinueBtn
        push    1
        push    EditConfigErr_ptr
        push    2
        call    SayErr
@@:
        add     ebx, 3
        and     ebx, not 3
        mov     [EditPlugInfo], ebx
.noini:
        mov     eax, [EditPlugInfo]
        add     eax, editor_line.plugdata
        imul    eax, max_height
        add     eax, editor_data.basesize + 0xFFF
        shr     eax, 12
        mov     [EditDataSize], eax
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
        call    draw_keybar
        call    draw_cmdbar
        mov     [prev_dir], 0
        mov     ebp, panel1_data
        call    calc_colwidths
        call    read_folder
        call    draw_panel
        mov     [bSilentFolderMode], 1
        mov     ebp, panel2_data
        call    calc_colwidths
        call    read_folder
        call    draw_panel
event:
        call    get_event
        dec     eax
        jz      redraw
        dec     eax
        jz      key
; button - we have only one button, close
exit:
; close all screens
@@:
        mov     ecx, [num_screens]
        mov     eax, [screens]
        mov     ebp, [eax+ecx*8-4]
        mov     eax, [eax+ecx*8-8]
        push    ebp
        call    [eax+screen_vtable.OnExit]
        pop     ecx
        call    pgfree
        dec     [num_screens]
        jnz     @b
; unload all plugins
        mov     ecx, [num_plugins]
        imul    esi, ecx, PluginInfo.size
        add     esi, [plugins]
.unload:
        dec     ecx
        js      .unload_done
        sub     esi, PluginInfo.size
        push    ecx esi
        call    [esi+PluginInfo.unload]
        pop     esi ecx
        jmp     .unload
.unload_done:
        cmp     [ini.set_int], aIniSetInt
        jz      .nosave
        push    [panel1_colmode]
        push    aLeftViewMode
        push    aPanels
        push    app_path
        call    [ini.set_int]
        push    [panel2_colmode]
        push    aRightViewMode
        push    aPanels
        push    app_path
        call    [ini.set_int]
        movzx   eax, [panel1_sortmode]
        push    eax
        push    aLeftSortMode
        push    aPanels
        push    app_path
        call    [ini.set_int]
        movzx   eax, [panel2_sortmode]
        push    eax
        push    aRightSortMode
        push    aPanels
        push    app_path
        call    [ini.set_int]
.nosave:
if CHECK_FOR_LEAKS
        mov     ecx, [panel1_files]
        call    pgfree
        mov     ecx, [panel2_files]
        call    pgfree
        mov     ecx, [screens]
        call    pgfree
        mov     ecx, [associations]
        call    pgfree
        mov     ecx, [console_data_ptr]
        call    pgfree
        mov     ecx, [MemForImage]
        call    pgfree
        mov     esi, FolderShortcuts
        push    10
        pop     ecx
@@:
        lodsd
        test    eax, eax
        jnz     @f
        loop    @b
        jmp     .nofreefs
@@:
        mov     ecx, eax
        call    pgfree
        mov     ecx, [plugins]
        call    pgfree
        mov     ecx, [panel1_parents]
        call    pgfree
        mov     ecx, [panel2_parents]
        call    pgfree
.nofreefs:
        mov     eax, [numallocatedregions]
        test    eax, eax
        jz      @f
        mov     edi, allocatedregions
        int3
        jmp     $
@@:
end if
        or      eax, -1
        int     40h

get_event:
        push    ebx
        mov     ebx, [idle_interval]
        cmp     ebx, -1
        jz      .infinite
        push    23
        pop     eax
        int     40h
        pop     ebx
        test    eax, eax
        jnz     .ret
        mov     ebp, [active_screen_data]
        mov     eax, [active_screen_vtable]
        mov     eax, [eax+screen_vtable.OnIdle]
        test    eax, eax
        jz      get_event
        call    eax
        jmp     get_event
.infinite:
        pop     ebx
        push    10
        pop     eax
        int     40h
.ret:
        ret

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
        cmp     eax, min_width
        jae     @f
        mov     al, min_width
        mov     ch, 1
@@:
        cmp     eax, max_width
        jbe     @f
        mov     eax, max_width
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
        cmp     eax, min_height
        jae     @f
        mov     al, min_height
        mov     cl, 1
@@:
        cmp     eax, max_height
        jbe     @f
        mov     eax, max_height
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
        mov     ecx, [MemForImage]
        call    pgfree
        and     [MemForImage], 0
        call    init_console
;        call    draw_window
        call    draw_keybar
        mov     ebp, [active_screen_data]
        mov     eax, [active_screen_vtable]
        mov     [cursor_size], cursor_normal_size
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

enum_associations_callback:
; LongBool __stdcall callback(f_name,sec_name,key_name,key_value);
; [esp+4] = f_name, [esp+8] = sec_name, [esp+12] = key_name, [esp+16] = key_value
        mov     esi, [esp+12]
        mov     edi, esi
@@:
        lodsb
        test    al, al
        jnz     @b
        sub     esi, edi        ; esi = size of key name
        mov     eax, [esp+16]
@@:
        inc     esi
        inc     eax
        cmp     byte [eax-1], 0
        jnz     @b
; esi = total size of entry
        push    esi
        add     esi, [associations_size]
        mov     ecx, [associations_allocated]
@@:
        cmp     esi, ecx
        jbe     @f
        add     ecx, ecx
        jmp     @b
@@:
        pop     esi
        cmp     [associations_allocated], ecx
        jz      @f
        mov     [associations_allocated], ecx
        mov     edx, [associations]
        call    xpgrealloc
        mov     [associations], eax
        test    eax, eax
        jz      .err
@@:
        mov     eax, esi
        mov     esi, edi
        mov     edi, [associations]
        add     edi, [associations_size]
        dec     edi
        add     [associations_size], eax
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        mov     esi, [esp+16]
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
.ret:
        mov     al, 1
        ret     16
.err:
        mov     ecx, edx
        call    pgfree
        xor     eax, eax
        ret     16

enum_plugins_callback:
; LongBool __stdcall callback(f_name,sec_name,key_name,key_value);
; [esp+4] = f_name, [esp+8] = sec_name, [esp+12] = key_name, [esp+16] = key_value
        mov     esi, [esp+16]
        cmp     byte [esi], '/'
        jz      .absolute
; convert path to absolute
        mov     edi, execdata
@@:
        cmp     edi, execdata+1024
        jae     .overflow
        lodsb
        stosb
        test    al, al
        jnz     @b
        mov     edi, saved_file_name
        mov     [esp+16], edi
        mov     esi, app_path
        push    esi
        xor     ecx, ecx
@@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, '/'
        jnz     @b
        mov     ecx, esi
        jmp     @b
@@:
        pop     esi
        sub     ecx, esi
        push    edi
        add     edi, ecx
        cmp     edi, saved_file_name+1024
        pop     edi
        ja      .overflow
        rep     movsb
        mov     esi, execdata
.z:
        cmp     word [esi], '.'
        jz      .ret
        cmp     word [esi], './'
        jnz     @f
        add     esi, 2
        jmp     .z
@@:
        cmp     word [esi], '..'
        jnz     .c
        cmp     byte [esi+2], 0
        jz      .ret
        cmp     byte [esi+2], '/'
        jnz     .c
        add     esi, 3
@@:
        dec     edi
        cmp     edi, saved_file_name
        jbe     .ret
        cmp     byte [edi-1], '/'
        jnz     @b
        jmp     .z
.c:
        cmp     edi, saved_file_name+1024
        jae     .overflow
        lodsb
        stosb
        test    al, al
        jnz     .c
        jmp     .absolute
.overflow:
        mov     esi, execdata
        mov     byte [esi+1023], 0
        call    load_dll_and_import.big
.ret:
        mov     al, 1
        ret     16
.absolute:
; allocate space for plugin info
        mov     eax, [num_plugins]
        inc     eax
        mov     [num_plugins], eax
        imul    ecx, eax, PluginInfo.size
        cmp     ecx, [alloc_plugins]
        jbe     .norealloc
        mov     edx, [plugins]
        call    xpgrealloc
        test    eax, eax
        jnz     @f
.dec_ret:
        dec     [num_plugins]
        jmp     .ret
@@:
        mov     [plugins], eax
        lea     eax, [ecx+0xFFF]
        and     eax, not 0xFFF
        mov     [alloc_plugins], eax
.norealloc:
        mov     esi, [plugins]
        lea     esi, [esi+ecx-PluginInfo.size]
; load plugin DLL
        or      ebp, -1
        mov     eax, [esp+16]
        call    load_dll_and_import.do
        test    eax, eax
        jnz     .dec_ret
; find exported functions
        mov     eax, aVersion
        call    load_dll_and_import.find_exported_function
        jnc     @f
        xor     eax, eax
@@:
MIN_INTERFACE_VER = 3
MAX_INTERFACE_VER = 3
        cmp     eax, MIN_INTERFACE_VER
        jb      @f
        cmp     eax, MAX_INTERFACE_VER
        jbe     .version_ok
@@:
        push    aIncompatibleVersion
.cantload:
        push    dword [esp+4+16]
        push    aCannotLoadPlugin
        mov     eax, esp
        push    ContinueBtn
        push    1
        push    eax
        push    3
        call    SayErr
        add     esp, 12
        jmp     .dec_ret
.version_ok:
        mov     edi, esi
        mov     esi, plugin_exported
.import:
        lodsd
        test    eax, eax
        jz      .import_done
        call    load_dll_and_import.find_exported_function
        jnc     @f
        mov     eax, [esi-4+plugin_exported_default-plugin_exported]
@@:
        stosd
        jmp     .import
.import_done:
; initialize plugin
        mov     eax, aPluginLoad
        call    load_dll_and_import.find_exported_function
        jc      .ok
        push    kfar_info
        call    eax
        cmp     eax, 1
        jb      .ok
        ja      .dec_ret
        push    aInitFailed
        jmp     .cantload
.ok:
        mov     al, 1
        ret     16

plugin_unload_default:
        ret

OpenFilePlugin_default:
        xor     eax, eax
        ret     28
GetFiles_default:
        xor     eax, eax
        ret     20

ClosePlugin_default:
        ret     4

GetOpenPluginInfo_default:
        ret     8

SetFolder_default:
open_default:
        xor     eax, eax
        ret     12

new_screen:
; in: ecx=sizeof(screen data), edx->vtable
; out: ebp=pointer or NULL, eax!=0 if successful
; destroys ebx,ecx
        call    xpgalloc
        test    eax, eax
        jnz     @f
        ret
@@:
        mov     ebp, eax
        mov     eax, [screens]
        mov     ecx, [num_screens]
        inc     ecx
        shl     ecx, 3
        test    ecx, 0xFFF
        jnz     .norealloc
        push    edx
        mov     edx, eax
        call    xpgrealloc
        pop     edx
        test    eax, eax
        jnz     @f
        mov     ecx, ebp
        call    pgfree
        xor     eax, eax
        ret
@@:
        mov     [screens], eax
.norealloc:
        inc     [num_screens]
        mov     [eax+ecx-8], edx
        mov     [eax+ecx-4], ebp
        mov     eax, [num_screens]
        dec     eax
        mov     [active_screen], eax
        mov     [active_screen_vtable], edx
        mov     [active_screen_data], ebp
        or      [idle_interval], -1
        mov     edx, [edx+screen_vtable.keybar]
        test    edx, edx
        jz      @f
        mov     [active_screen_keybar], edx
@@:
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
        mov     ecx, [num_screens]
        shl     ecx, 3
        test    ecx, 0xFFF
        jnz     .norealloc
        mov     edx, [screens]
        call    pgrealloc               ; must succeed, because we decrease size
        mov     [screens], eax
.norealloc:
        pop     ecx
        call    pgfree
        dec     [active_screen]

change_screen:
        pusha
        or      [idle_interval], -1
        mov     eax, [active_screen]
        mov     esi, [screens]
        mov     ebp, [esi+eax*8+4]
        mov     eax, [esi+eax*8]
        push    eax ebp
        mov     eax, [eax+screen_vtable.OnActivate]
        test    eax, eax
        jz      @f
        call    eax
@@:
        pop     ebp eax
        mov     [active_screen_vtable], eax
        mov     [active_screen_data], ebp
        mov     edx, [eax+screen_vtable.keybar]
        test    edx, edx
        jz      @f
        mov     [active_screen_keybar], edx
@@:
        call    draw_keybar
        mov     [cursor_size], cursor_normal_size
        call    [eax+screen_vtable.OnRedraw]
        popa
        ret

F12:
        mov     eax, [cur_width]
        add     eax, 8
        mov     esi, eax
        mul     [num_screens]
        mov     ecx, eax
        call    xpgalloc
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
        push    dword aScreens
        push    eax
        call    menu
        cmp     eax, -1
        jz      @f
        sub     eax, ebx
        div     esi
        mov     [active_screen], eax
@@:
        mov     ecx, ebx
        call    pgfree
        jmp     change_screen

panels_OnKey:
; restore screen hidden by quick search box
        cmp     [bDisplayQuickSearch], 0
        jz      @f
        mov     [bDisplayQuickSearch], 0
        push    0
        push    QuickSearchDlg
        call    HideDialogBox
@@:
        mov     ebp, [active_panel]
        mov     ecx, [ebp + panel1_index - panel1_data]
        mov     edx, [ebp + panel1_start - panel1_data]
        mov     ebx, [ebp + panel1_colst - panel1_data]
        add     ebx, edx
        mov     esi, panels_ctrlkeys
        call    process_ctrl_keys
        jc      .nostdkey
        mov     [bQuickSearchMode], 0
        ret
.leaveqsmode:
        mov     [bQuickSearchMode], 0
        call    draw_image
        ret
.keepqsmode:
        push    QuickSearchDlg
        call    ShowDialogBox
        mov     [bDisplayQuickSearch], 1
        ret
.nostdkey:
        cmp     [bQuickSearchMode], 0
        jz      .noqsmode
        test    al, al
        js      .keepqsmode
        cmp     al, 40h
        jae     .leaveqsmode
        cmp     al, 0Eh
        jz      .qsbackspace
        cmp     al, 1Ch
        jz      .test_enter
        movzx   eax, al
        cmp     byte [scan2ascii+eax], 0
        jz      .leaveqsmode
        call    get_ascii_char
        mov     ecx, dword [quick_search_buf + 4]
        cmp     ecx, quicksearch_maxlen
        jae     .keepqsmode
        mov     byte [quick_search_buf + 12 + ecx], al
        mov     byte [quick_search_buf + 12 + ecx + 1], 0
        push    ecx
        call    quick_find
        pop     ecx
        jnc     @f
        mov     byte [quick_search_buf + 12 + ecx], 0
        jmp     .keepqsmode
@@:
        inc     dword [quick_search_buf + 4]
        sub     ecx, 16
        jae     @f
        xor     ecx, ecx
@@:
        mov     dword [quick_search_buf + 8], ecx
        jmp     .keepqsmode
.qsbackspace:
        mov     ecx, dword [quick_search_buf + 4]
        jecxz   .keepqsmode2
        dec     ecx
        mov     byte [quick_search_buf + 12 + ecx], 0
        dec     dword [quick_search_buf + 4]
.keepqsmode2:
        jmp     .keepqsmode
.test_enter:
        test    [ctrlstate], 0Ch        ; LCtrl or RCtrl pressed?
        jz      .keepqsmode2
        test    [ctrlstate], 3          ; Shift pressed?
        jnz     @f
        call    quick_find_next
        jmp     .keepqsmode2
@@:
        call    quick_find_prev
        jmp     .keepqsmode2
.noqsmode:
; handle Alt+<key> combinations for quick search
        test    [ctrlstate], 30h        ; LAlt or RAlt pressed?
        jz      .noalt                  ; no => skip
        test    [ctrlstate], 0Ch        ; LCtrl or RCtrl pressed?
        jnz     .noalt                  ; yes => skip
        cmp     al, 40h
        jae     .noalt
        movzx   eax, al
        cmp     byte [scan2ascii+eax], 0
        jz      .noalt
        call    get_ascii_char
        mov     edi, quick_search_buf + 12
        mov     dword [edi-12], quicksearch_maxlen-1
        mov     dword [edi-8], 1
        and     dword [edi-4], 0
        stosb
        mov     byte [edi], 0
        call    quick_find
        jnc     @f
        mov     byte [quick_search_buf + 12], 0
        and     dword [quick_search_buf + 4], 0
@@:
        mov     ebx, QuickSearchDlg
        mov     eax, [ebp + panel1_left - panel1_data]
        add     eax, 10
        mov     edx, [cur_width]
        sub     edx, 21
        cmp     eax, edx
        jb      @f
        mov     eax, edx
@@:
        mov     [ebx + dlgtemplate.x], eax
        mov     eax, [ebp + panel1_top - panel1_data]
        add     eax, [ebp + panel1_height - panel1_data]
        mov     edx, [cur_height]
        sub     edx, 2
        cmp     eax, edx
        jb      @f
        mov     eax, edx
@@:
        mov     [ebx + dlgtemplate.y], eax
        push    ebx
        call    ShowDialogBox
        mov     [bQuickSearchMode], 1
        mov     [bDisplayQuickSearch], 1
.find_letter_done:
.noalt:
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
.insert:
        lea     eax, [ecx*4]
        add     eax, [ebp + panel1_files - panel1_data]
        mov     eax, [eax]
        cmp     word [eax+40], '..'
        jnz     @f
        cmp     byte [eax+42], 0
        jz      .insert.down
@@:
        xor     byte [eax+303], 1
        mov     edx, [eax+32]
        test    byte [eax+303], 1
        mov     eax, [eax+36]
        jnz     .insert.increase
        sub     dword [ebp + panel1_selected_size - panel1_data], edx
        sbb     dword [ebp + panel1_selected_size+4 - panel1_data], eax
        dec     [ebp + panel1_selected_num - panel1_data]
        jmp     .insert.down
.insert.increase:
        add     dword [ebp + panel1_selected_size - panel1_data], edx
        adc     dword [ebp + panel1_selected_size+4 - panel1_data], eax
        inc     [ebp + panel1_selected_num - panel1_data]
.insert.down:
        inc     ecx
        cmp     ecx, [ebp + panel1_numfiles - panel1_data]
        jae     .done_redraw
        jmp     @f
.down:
        inc     ecx
        cmp     ecx, [ebp + panel1_numfiles - panel1_data]
        jae     .ret
@@:
        mov     [ebp + panel1_index - panel1_data], ecx
        cmp     ecx, ebx
        jb      .done_redraw
        sub     ecx, [ebp + panel1_colst - panel1_data]
        inc     ecx
        mov     [ebp + panel1_start - panel1_data], ecx
        jmp     .done_redraw
.left:
        test    ecx, ecx
        jnz     @f
        ret
@@:
        mov     eax, [ebp + panel1_colsz - panel1_data]
        sub     ecx, eax
        jae     @f
        xor     ecx, ecx
@@:
        mov     [ebp + panel1_index - panel1_data], ecx
.finalize_left:
        cmp     ecx, edx
        jae     .done_redraw
        sub     edx, eax
        jae     @f
        xor     edx, edx
@@:
        mov     [ebp + panel1_start - panel1_data], edx
        jmp     .done_redraw
.pgup:
        mov     eax, [ebp + panel1_colst - panel1_data]
        dec     eax
        jnz     @f
        inc     eax
@@:
        test    ecx, ecx
        jnz     @f
        ret
@@:
        sub     ecx, eax
        jae     @f
        xor     ecx, ecx
@@:
        mov     [ebp + panel1_index - panel1_data], ecx
        dec     ecx
        jmp     .finalize_left
.right:
        mov     eax, [ebp + panel1_colsz - panel1_data]
        add     ecx, eax
        cmp     ecx, [ebp + panel1_numfiles - panel1_data]
        jb      @f
        mov     ecx, [ebp + panel1_numfiles - panel1_data]
        dec     ecx
@@:
        mov     [ebp + panel1_index - panel1_data], ecx
.finalize_right:
        cmp     ecx, ebx
        jb      .done_redraw
        add     ebx, eax
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
.pgdn:
        mov     eax, [ebp + panel1_colst - panel1_data]
        dec     eax
        jnz     @f
        inc     eax
@@:
        add     ecx, eax
        cmp     ecx, [ebp + panel1_numfiles - panel1_data]
        jb      @f
        mov     ecx, [ebp + panel1_numfiles - panel1_data]
        dec     ecx
@@:
        mov     [ebp + panel1_index - panel1_data], ecx
        inc     ecx
        jmp     .finalize_right
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
        cmp     [bQuickSearchMode], 0
        jz      @f
        call    draw_image
@@:
        call    get_curfile_folder_entry
        test    byte [ecx], 10h
        jnz     .enter_folder
; todo: add <Enter> handling on plugin panel
        cmp     [ebp + panel1_hPlugin - panel1_data], 0
        jnz     .ret
; generate full file name
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
; try to open file and look for all plugins
        push    O_READ
        push    execdata
        call    open
        test    eax, eax
        jz      .noplugins      ; if can't open, just try to execute
        mov     esi, eax        ; save handle
        push    filedata_buffer_size
        push    filedata_buffer
        push    esi
        call    read
        mov     edi, eax        ; save size of read data
; test for Kolibri executable
        cmp     eax, 24h
        jb      .enter.noexec
        cmp     dword [filedata_buffer], 'MENU'
        jnz     @f
        cmp     word [filedata_buffer+4], 'ET'
        jnz     @f
.close_run:
        push    esi
        call    close
        jmp     .run_app
@@:
        cmp     dword [filedata_buffer], 'KPCK'
        jnz     @f
        cmp     dword [filedata_buffer+12], 0x26914601
        jz      .close_run
@@:
.enter.noexec:
        mov     ecx, [num_plugins]
        mov     edx, [plugins]
        sub     edx, PluginInfo.size
.plugloop:
        add     edx, PluginInfo.size
        dec     ecx
        js      .plugdone
        pushad
        push    execdata
        push    [ebp+panel1_hFile-panel1_data]
        push    [ebp+panel1_hPlugin-panel1_data]
        push    edi
        push    filedata_buffer
        call    get_curfile_folder_entry
        push    ecx
        push    esi
        call    [edx+PluginInfo.OpenFilePlugin]
        mov     [esp+28], eax
        popad
        test    eax, eax
        jz      .plugloop
        cmp     eax, -1
        jnz     .pluginok
        push    esi
        call    close
        ret
.pluginok:
; save current directory and set root directory of hPlugin
        mov     edi, eax        ; save handle
        mov     esi, execdata
@@:
        lodsb
        test    al, al
        jnz     @b
        sub     esi, execdata-9
; allocate esi bytes in buffer 'parents'
        mov     ecx, [ebp + panel1_parents_sz - panel1_data]
        add     ecx, esi
        cmp     ecx, [ebp + panel1_parents_alloc - panel1_data]
        jbe     .enter.norealloc
        push    edx
        mov     edx, [ebp + panel1_parents - panel1_data]
        call    xpgrealloc
        pop     edx
        test    eax, eax
        jnz     @f
        push    edi
        call    [edx+PluginInfo.ClosePlugin]
        ret
@@:
        mov     [ebp + panel1_parents - panel1_data], eax
.enter.norealloc:
        mov     [ebp + panel1_parents_sz - panel1_data], ecx
; save current state to the end of buffer
        sub     ecx, esi
        add     ecx, [ebp + panel1_parents - panel1_data]
        xchg    edx, [ebp + panel1_hPlugin - panel1_data]
        mov     [ecx], edx
        xchg    edi, [ebp + panel1_hFile - panel1_data]
        mov     [ecx+4], edi
        mov     byte [ecx+8], 0
        lea     edi, [ecx+9]
        lea     ecx, [esi-9]
        mov     esi, execdata
        rep     movsb
        mov     word [ebp + panel1_dir - panel1_data], '/'
        mov     eax, [ebp + panel1_hPlugin - panel1_data]
        lea     ebx, [ebp + panel1_plugin_info - panel1_data]
        and     dword [ebx], 0
        push    ebp
        push    ebx
        push    [ebp + panel1_hFile - panel1_data]
        call    [eax+PluginInfo.GetOpenPluginInfo]
        pop     ebp
        call    get_curfile_folder_entry
        mov     esi, ecx
        mov     edi, left_dotdot_entry
        cmp     ebp, panel1_data
        jz      @f
        add     edi, right_dotdot_entry-left_dotdot_entry
@@:
        mov     ecx, 10
        rep     movsd
        mov     byte [edi-40], 10h      ; attributes: folder
        mov     eax, '..'
        stosd
        jmp     .reread
.plugdone:
        push    esi
        call    close
.noplugins:
; run program or association
        call    get_curfile_folder_entry
        call    find_extension
        jc      .run_app
.run_association:
        cmp     byte [edi], 0
        jz      .l1
        cmp     byte [edi], ';'
        jnz     @f
.l1:
        ret
@@:
        mov     esi, edi
@@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, ';'
        jnz     @b
        dec     esi
        mov     byte [esi], 0
        mov     [restore_semicolon], esi
@@:
.run_association2:
        mov     [execparams], execdata
        mov     [execptr], edi
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
        mov     esi, [execptr]
@@:
        lodsb
        test    al, al
        jnz     @b
        mov     eax, [esi-5]
        or      eax, 0x20202020
        cmp     eax, 'fasm'
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
; we do not want this!
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
        xor     esi, esi
        xchg    esi, [restore_semicolon]
        test    esi, esi
        jz      @f
        mov     byte [esi], ';'
@@:
        neg     eax
        js      @f
        call    get_error_msg
        push    eax
        push    dword aRunError
        mov     eax, esp
        push    ContinueBtn
        push    1
        push    eax
        push    2
        call    SayErr
        pop     eax
        pop     eax
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
.l2:
        xor     esi, esi
        xchg    esi, [restore_semicolon]
        test    esi, esi
        jz      @f
        mov     byte [esi], ';'
@@:
        push    eax
        mov     eax, esp
        push    ContinueBtn
        push    1
        push    eax
        push    1
        call    SayErr
        pop     eax
        ret
.bigcmdline:
        mov     eax, aCmdLineTooBig
        jmp     .l2
.bigfoldername2:
        mov     esi, prev_dir
        lea     edi, [ebp + panel1_dir - panel1_data]
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
.bigfoldername:
        mov     eax, aFolderNameTooBig
        jmp     .l2
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
        mov     eax, [ebp + panel1_hPlugin - panel1_data]
        test    eax, eax
        jz      .reread
        push    ebp
        lea     esi, [ebp + panel1_dir - panel1_data]
        push    esi
        add     ecx, 40
        push    ecx
        push    [ebp + panel1_hFile - panel1_data]
        call    [eax+PluginInfo.SetFolder]
        pop     ebp
        test    al, al
        jnz     .reread
        mov     esi, prev_dir
        lea     edi, [ebp + panel1_dir - panel1_data]
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
.retd:
        ret
.reread:
        call    read_folder
.done_cmdbar:
        call    draw_cmdbar
        jmp     .done_redraw
.dotdot:
        lea     edi, [ebp + panel1_dir - panel1_data]
        cmp     word [edi], '/'
        jnz     .dotdot_noroot
        cmp     [ebp + panel1_hPlugin - panel1_data], 0
        jz      .retd
        call    close_plugin_panel
        jmp     .dotdot
.dotdot_noroot:
        mov     edx, edi
        mov     al, 0
        or      ecx, -1
        repnz   scasb
        dec     edi
        mov     al, '/'
        std
        repnz   scasb
        cld
        inc     edi
        cmp     edi, edx
        jnz     @f
        inc     edi
@@:
        push    dword [edi]
        mov     byte [edi], 0
        push    edi
        mov     eax, [ebp + panel1_hPlugin - panel1_data]
        test    eax, eax
        jz      .dotdot_native
        push    ebp
        lea     esi, [ebp + panel1_dir - panel1_data]
        push    esi
        push    aDotDot
        push    [ebp + panel1_hFile - panel1_data]
        call    [eax+PluginInfo.SetFolder]
        pop     ebp
        test    al, al
        jnz     .dotdot_native
        pop     edi
        pop     dword [edi]
        ret
.dotdot_native:
        call    read_folder
        pop     edi
        pop     dword [edi]
        push    edi
        cmp     byte [edi], '/'
        jnz     @f
        inc     edi
@@:
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
        pop     edi
        mov     byte [edi], 0
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
        xor     edx, edx
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
        add     ecx, 32+8
        push    ecx
        and     ecx, 0xFFF
        cmp     ecx, 32+8
        pop     ecx
        ja      @f
        call    xpgrealloc
        mov     edx, eax
        test    eax, eax
        jnz     @f
        pop     eax
        ret
@@:
        lea     edi, [edx+ecx-32]
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
        mov     edi, edx
        xor     esi, esi
        add     ecx, edx
@@:
        and     dword [edi], 0
        mov     dword [edi+4], esi
        mov     esi, edi
        add     edi, 32+8
        cmp     edi, ecx
        jae     @f
        mov     [esi], edi
        jmp     @b
@@:
        mov     ecx, edx
        lea     edi, [ebp + panel1_dir - panel1_data]
        cmp     [ebp + panel1_hPlugin - panel1_data], 0
        jz      .find_cur_drive_loop
        mov     edi, [ebp + panel1_parents - panel1_data]
        add     edi, 8
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
        cmp     dword [ecx], 0
        jz      @f
        pop     edi
        mov     ecx, [ecx]
        jmp     .find_cur_drive_loop
@@:
        mov     ecx, edx
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
        jnz     @f
        mov     ecx, edx
        call    pgfree
        ret
@@:
        push    eax
        push    edx
        call    close_plugin_panels
        pop     edx
        lea     edi, [ebp + panel1_dir - panel1_data]
        push    edi
        mov     esi, edi
        mov     edi, prev_dir
        mov     ecx, 1024/4
        rep     movsd
        pop     edi
        pop     esi
        add     esi, 8
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        mov     ecx, edx
        call    pgfree
        call    read_folder
        jmp     .done_redraw
.shift_f5:
; todo: copy to plugin panel
        cmp     [ebp + panel1_hPlugin - panel1_data], 0
        jz      @f
        ret
@@:
        mov     esi, ebp
        cmp     [ebp + panel1_selected_num - panel1_data], 0
        jnz     .f5_2
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
; todo: copy to plugin panel
        mov     esi, ebp
        xor     esi, panel1_data xor panel2_data
        cmp     [esi + panel1_hPlugin - panel1_data], 0
        jz      .f5_2
        ret
.f5_2:
        add     esi, panel1_dir - panel1_data
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
        jz      @f
        cmp     edi, CopyDestEditBuf+12+511
        jae     .bigfoldername
        stosb
        jmp     @b
@@:
        mov     al, '/'
        stosb
.f5_common:
        mov     eax, [ebp + panel1_hPlugin - panel1_data]
        mov     [source_hModule], eax
        mov     eax, [ebp + panel1_hFile - panel1_data]
        mov     [source_hPlugin], eax
        mov     eax, left_dotdot_entry
        cmp     ebp, panel1_data
        jz      @f
        add     eax, right_dotdot_entry-left_dotdot_entry
@@:
        mov     [default_attr], eax
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
        mov     eax, [ebp + panel1_selected_num - panel1_data]
        test    eax, eax
        jz      .f5_noselected1
        mov     ebx, eax
        push    10
        pop     ecx
        push    -'0'
@@:
        xor     edx, edx
        div     ecx
        push    edx
        test    eax, eax
        jnz     @b
@@:
        pop     eax
        add     al, '0'
        jz      @f
        stosb
        jmp     @b
@@:
if lang eq ru
        mov     dword [edi], ' эле'
        mov     dword [edi+4], 'мент'
        add     edi, 8
        cmp     ebx, 1
        jz      @f
        mov     al, 'а'
        stosb
        cmp     ebx, 4
        jbe     @f
        mov     word [edi-1], 'ов'
        inc     edi
@@:
else
        mov     dword [edi], ' ite'
        mov     byte [edi+4], 'm'
        add     edi, 5
        cmp     ebx, 1
        jz      @f
        mov     al, 's'
        stosb
@@:
end if
        jmp     .f5_selected1
.f5_noselected1:
        mov     al, '"'
        stosb
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
        mov     al, '"'
        stosb
.f5_selected1:
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
        mov     [bEndSlash], 0
        cmp     byte [edi], '/'
        jnz     @f
        cmp     edi, esi
        jz      @f
        mov     byte [edi], 0
        dec     edi
        mov     [bEndSlash], 1
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
; Сначала создаём все вышележащие папки, которые ещё не существуют
; Последний из элементов может быть как файлом, так и папкой;
; форсируем папку в случае, если хотя бы один из источников является папкой
        xor     edx, edx
        cmp     [ebp + panel1_selected_num - panel1_data], 0
        jz      .f5_noselected2
        mov     ecx, [ebp + panel1_numfiles - panel1_data]
        mov     edi, [ebp + panel1_files - panel1_data]
.scanselected2:
        mov     eax, [edi]
        add     edi, 4
        test    byte [eax+303], 1
        jz      @f
        test    byte [eax], 10h
        jz      @f
        inc     edx
@@:
        loop    .scanselected2
        jmp     .f5_selected2
.f5_noselected2:
        call    get_curfile_folder_entry
        test    byte [ecx], 10h
        setnz   dl
.f5_selected2:
        xor     eax, eax
        mov     edi, esi
.countslashloop:
        cmp     byte [edi], '/'
        jnz     @f
        inc     eax
        mov     ecx, edi
@@:
        inc     edi
        cmp     byte [edi], 0
        jnz     .countslashloop
        mov     [bNeedRestoreName], 0
; "/file-system/partition" folders already exist, so we don't need to create them
        cmp     eax, 2
        jbe     .createupdone
; we interpret destination as folder if edx!=0 or bEndSlash!=0
        test    edx, edx
        jnz     @f
        cmp     [bEndSlash], 0
        jnz     @f
        mov     edi, ecx
        dec     eax
@@:
        mov     [attrinfo.name], esi
        mov     ecx, eax
; ищем последнюю папку, которая ещё существует
.createuploop1:
        cmp     eax, 2
        jbe     .createupdone1
        mov     dl, [edi]
        mov     byte [edi], 0
        push    eax
        push    70
        pop     eax
        mov     ebx, attrinfo
        int     0x40
        test    eax, eax
        pop     eax
        mov     byte [edi], dl
        jnz     .createupcont1
        test    byte [attrinfo.attr], 10h
        jnz     .createupdone1
; the item required to be a folder, but is file
        push    aNotFolder
        push    esi
        mov     eax, esp
        push    ContinueBtn
        push    1
        push    eax
        push    2
        call    SayErr
        pop     eax
        pop     eax
.ret3:
        ret
.createupcont1:
        dec     eax
@@:
        dec     edi
        cmp     byte [edi], '/'
        jnz     @b
        jmp     .createuploop1
.createupdone1:
; создаём все папки, которые нам нужны
        cmp     eax, ecx
        jae     .createupdone
@@:
        inc     edi
        cmp     byte [edi], 0
        jz      @f
        cmp     byte [edi], '/'
        jnz     @b
@@:
        mov     dl, byte [edi]
        mov     byte [edi], 0
        push    eax
        push    RetryOrCancelBtn
        push    2
        call    makedir
        mov     byte [edi], dl
        mov     [bNeedRestoreName], 1
        pop     eax
        jnz     .ret3
        inc     eax
        jmp     .createupdone1
.createupdone:
        mov     dl, 1
        cmp     byte [edi], 1
        sbb     eax, -1
        cmp     eax, 2
        jbe     .docopy
        mov     [attrinfo.attr], 0      ; assume zero attributes if error
        push    70
        pop     eax
        mov     ebx, attrinfo
        int     0x40
        test    [attrinfo.attr], 10h
        setnz   dl
.docopy:
        call    get_curfile_folder_entry
        mov     eax, ecx
        cmp     [bNeedRestoreName], 0
        jz      @f
        cmp     [bEndSlash], 0
        jnz     @f
        cmp     [ebp + panel1_selected_num - panel1_data], 0
        jnz     @f
        test    byte [eax], 10h
        jz      @f
        mov     dl, 0
@@:
; начинаем собственно копирование
        mov     [bDestIsFolder], dl
        mov     [copy_bSkipAll], 0
        mov     [copy_bSkipAll2], 0
        mov     [copy_bSkipAll3], 0
        test    [ebp + panel1_plugin_flags - panel1_data], 2
        jnz     .copy_GetFiles
        cmp     [ebp + panel1_selected_num - panel1_data], 0
        jnz     .f5_selected3
        call    copy_file
.copydone:
        push    ebp
        call    .ctrl_r
        pop     ebp
        xor     ebp, panel1_data xor panel2_data
        jmp     .ctrl_r
.f5_selected3:
        mov     ecx, [ebp + panel1_numfiles - panel1_data]
        mov     esi, [ebp + panel1_files - panel1_data]
.f5_selected_copyloop:
        lodsd
        test    byte [eax+303], 1
        jz      .f5_selected_copycont
        mov     [bDestIsFolder], dl
        call    copy_file
        ja      .f5_multiple_cancel
        and     byte [eax+303], not 1
        pushad
        call    draw_panel
        cmp     [bNeedRestoreName], 0
        jz      @f
        mov     esi, CopyDestEditBuf+12
        call    delete_last_name_from_end
@@:
        popad
.f5_selected_copycont:
        loop    .f5_selected_copyloop
.f5_multiple_cancel:
        jmp     .copydone
.copy_GetFiles:
        mov     ecx, [ebp + panel1_selected_num - panel1_data]
        cmp     ecx, 1
        adc     ecx, 0
        shl     ecx, 2
        call    xpgalloc
        test    eax, eax
        jnz     @f
        ret
@@:
        push    ebp eax ; save
        push    copy_AddDir     ; adddir
        push    copy_AddFile    ; addfile
        push    eax             ; items
        shr     ecx, 2
        push    ecx             ; NumItems
        push    [ebp + panel1_hFile - panel1_data]
        mov     edi, eax
        call    get_curfile_folder_entry
        mov     [edi], ecx
        cmp     [ebp + panel1_selected_num - panel1_data], 0
        jz      .cgf1
        mov     esi, [ebp + panel1_files - panel1_data]
        mov     ecx, [ebp + panel1_numfiles - panel1_data]
.cgf0:
        lodsd
        test    byte [eax+303], 1
        jz      @f
        stosd
@@:
        loop    .cgf0
.cgf1:
        mov     eax, [ebp + panel1_hPlugin - panel1_data]
        call    [eax+PluginInfo.GetFiles]
        pop     ecx ebp
        call    pgfree
        jmp     .copydone

.f3:
        mov     eax, [ebp + panel1_files - panel1_data]
        mov     ecx, [eax+ecx*4]
        test    byte [ecx], 10h
        jnz     .ret2
        lea     esi, [ebp + panel1_dir - panel1_data]
        xor     eax, eax
        call    view_file
.ret2:
        ret
.f4:
        call    edit_file
        ret
.f8_has_selected:
        mov     edi, saved_file_name+511
        mov     byte [edi], 0
        mov     eax, [ebp + panel1_selected_num - panel1_data]
if lang eq ru
        cmp     eax, 1
        jz      @f
        dec     edi
        mov     byte [edi], 'а'
        cmp     eax, 4
        jbe     @f
        dec     edi
        mov     word [edi], 'ов'
@@:
        mov     dword [edi-4], 'мент'
        mov     dword [edi-8], ' эле'
        sub     edi, 8
else
        cmp     eax, 1
        jz      @f
        dec     edi
        mov     byte [edi], 's'
@@:
        mov     dword [edi-4], 'item'
        mov     byte [edi-5], ' '
        sub     edi, 5
end if
        xor     edx, edx
        push    10
        pop     ecx
@@:
        div     ecx
        add     dl, '0'
        dec     edi
        mov     [edi], dl
        xor     edx, edx
        test    eax, eax
        jnz     @b
        push    edi
        push    aConfirmDeleteText
        mov     eax, esp
        push    DeleteOrCancelBtn
        push    2
        push    eax
        push    2
        push    aDeleteCaption
        call    Message
        add     esp, 8
        test    eax, eax
        jnz     .ret2
        mov     [del_bSkipAll], 0
        mov     ecx, [ebp + panel1_numfiles - panel1_data]
        jecxz   .ret2
        mov     esi, [ebp + panel1_files - panel1_data]
.f8_loop:
        lodsd
        test    byte [eax+303], 1
        jz      @f
        call    delete_file
        ja      .f8_multiple_cancel
@@:
        loop    .f8_loop
.f8_multiple_cancel:
        jmp     .copydone
.f8:
; todo: delete files from plugin panel
        cmp     [ebp + panel1_hPlugin - panel1_data], 0
        jz      @f
        ret
@@:
        cmp     [ebp + panel1_selected_num - panel1_data], 0
        jnz     .f8_has_selected
        call    get_curfile_folder_entry
        cmp     word [ecx+40], '..'
        jnz     @f
        cmp     byte [ecx+42], 0
        jz      .f8_not_allowed
@@:
        cmp     [bConfirmDelete], 0
        jz      .f8_allowed
        lea     eax, [ecx+40]
        push    eax
        mov     esi, aConfirmDeleteText
        mov     edi, aConfirmDeleteTextBuf
        push    edi
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
        mov     eax, esp
        push    DeleteOrCancelBtn
        push    2
        push    eax
        push    2
        push    aDeleteCaption
        call    Message
        add     esp, 8
        test    eax, eax
        jz      .f8_allowed
.f8_not_allowed:
        ret
.f8_allowed:
        mov     [del_bSkipAll], 0
        mov     eax, ecx
        call    delete_file
        jmp     .copydone
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
.ctrl_r.doread:
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
; todo: context menu for plugin panel
        cmp     [ebp + panel1_hPlugin - panel1_data], 0
        jz      @f
        ret
@@:
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
; known extension
@@:
        cmp     byte [edi], 0
        jz      .menuret
        cmp     byte [edi], ';'
        jz      @f
        inc     edi
        jmp     @b
@@:
        inc     edi
@@:
        inc     edi
        cmp     byte [edi-1], ' '
        jz      @b
        cmp     byte [edi-1], 9
        jz      @b
        dec     edi
        cmp     byte [edi], 0
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
        xor     ebp, ebp
        push    edx
.menucreateloop:
        mov     eax, edi
        xor     ebx, ebx
@@:
        cmp     byte [edi], ','
        jz      @f
        cmp     byte [edi], bl
        jz      @f
        inc     edi
        jmp     @b
@@:
        xchg    bl, [edi]
        pushad
        push    nullstr
        push    1024
        push    saved_file_name
        push    eax
        push    aMenu
        push    app_path
        call    [ini.get_str]
        popad
        mov     [edi], bl
        mov     eax, saved_file_name
@@:
        cmp     byte [eax], 0
        jz      .menucreatecont
        cmp     byte [eax], ','
        jz      @f
        inc     eax
        jmp     @b
@@:
        mov     byte [eax], 0
        push    eax
        sub     eax, saved_file_name-1
        cmp     eax, ecx
        ja      @f
        mov     eax, ecx
@@:
        xchg    eax, [esp]
        inc     eax
        push    eax
@@:
        inc     eax
        cmp     byte [eax-1], 0
        jnz     @b
        sub     eax, [esp]
        add     [esp+4], eax
        pop     eax eax
        add     eax, 8
        add     ebp, eax
        push    ebp
        and     ebp, 0xFFF
        cmp     ebp, eax
        pop     ebp
        ja      @f
        push    eax
        xchg    edx, [esp+4]
        push    ecx
        mov     ecx, ebp
        call    xpgrealloc
        pop     ecx
        pop     edx
        xchg    edx, [esp]
        test    eax, eax
        xchg    eax, [esp]
        jz      .menucreated
@@:
        neg     eax
        add     eax, ebp
        add     eax, [esp]
        and     dword [eax], 0
        and     dword [eax+4], 0
        test    edx, edx
        jz      @f
        sub     eax, [esp]
        mov     [edx], eax
        add     eax, [esp]
        sub     edx, [esp]
        mov     [eax+4], edx
@@:
        mov     edx, eax
        push    esi edi
        mov     esi, saved_file_name
        lea     edi, [eax+8]
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        push    esi
        sub     esi, saved_file_name
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
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        pop     edi esi
.menucreatecont:
        inc     edi
        cmp     byte [edi-1], ','
        jz      .menucreateloop
.menucreated:
        test    edx, edx
        jz      .menuret_free
        and     dword [edx], 0
        pop     edx
        mov     ecx, edx
        add     [ecx], edx
@@:
        mov     ecx, [ecx]
        cmp     dword [ecx], 0
        jz      @f
        add     [ecx], edx
        add     [ecx+4], edx
        jmp     @b
@@:
        add     [ecx+4], edx
        push    edx
        push    1
        push    esi
        push    edx
        call    menu
        cmp     eax, -1
        jz      .menuret_free
        lea     edi, [eax+8]
@@:
        inc     edi
        cmp     byte [edi-1], 0
        jnz     @b
        mov     ebp, [active_panel]
        call    get_curfile_folder_entry
        call    .run_association2
.menuret_free:
        pop     ecx
        jmp     pgfree
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
        push    RetryOrCancelBtn
        push    2
        call    makedir
        jmp     .copydone
.change_mode:
        dec     eax
        mov     [ebp + panel1_colmode - panel1_data], eax
        call    calc_colwidths
        jmp     draw_panel
.quick_jump:
        dec     eax
        cmp     al, 10
        jnz     @f
        xor     eax, eax
@@:
        mov     eax, [FolderShortcuts+eax*4]
        test    eax, eax
        jnz     @f
        ret
@@:
        push    eax
        call    close_plugin_panels
        lea     esi, [ebp + panel1_dir - panel1_data]
        push    esi
        mov     edi, prev_dir
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        pop     edi esi
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        jmp     .reread
.greyplus:
        push    0
        mov     ecx, aSelect
        jmp     @f
.greyminus:
        push    1
        mov     ecx, aDeselect
@@:
        mov     eax, mark_dlgdata
        mov     [eax+mark_dlgdata.title-mark_dlgdata], ecx
        mov     [eax+mark_dlgdata.flags-mark_dlgdata], 0xC
        mov     edi, enter_string_buf+12
        mov     dword [edi-12], 512
        mov     dword [edi-8], 1
        and     dword [edi-4], 0
        mov     word [edi], '*'
        push    eax
        call    DialogBox
        inc     eax
        jnz     @f
.noselect:
        pop     eax
        ret
@@:
        xor     eax, eax
        mov     esi, edi
@@:
        lodsb
        mov     al, [tolower_table+eax]
        mov     [esi-1], al
        test    al, al
        jnz     @b
@@:
        mov     ecx, [ebp + panel1_numfiles - panel1_data]
        mov     ebx, [ebp + panel1_files - panel1_data]
        jecxz   .noselect
        mov     eax, [ebx]
        cmp     word [eax+40], '..'
        jnz     .selectloop
        cmp     byte [eax+42], 0
        jnz     .selectloop
        dec     ecx
        jz      .noselect
        add     ebx, 4
.selectloop:
        xor     eax, eax
        mov     esi, [ebx]
        add     esi, 40
        mov     edx, lower_file_name-1
@@:
        lodsb
        inc     edx
        mov     al, [tolower_table+eax]
        mov     [edx], al
        test    al, al
        jnz     @b
        mov     esi, lower_file_name
        call    match_mask_rev_lowercase
        jc      @f
        mov     esi, [ebx]
        mov     eax, [esi+32]
        cmp     byte [esp], 0
        jz      .doselect
        test    byte [esi+303], 1
        jz      @f
        and     byte [esi+303], not 1
        sub     dword [ebp + panel1_selected_size - panel1_data], eax
        mov     eax, [esi+36]
        sbb     dword [ebp + panel1_selected_size+4 - panel1_data], eax
        dec     dword [ebp + panel1_selected_num - panel1_data]
        jmp     @f
.doselect:
        test    byte [esi+303], 1
        jnz     @f
        or      byte [esi+303], 1
        add     dword [ebp + panel1_selected_size - panel1_data], eax
        mov     eax, [esi+36]
        adc     dword [ebp + panel1_selected_size+4 - panel1_data], eax
        inc     dword [ebp + panel1_selected_num - panel1_data]
@@:
        add     ebx, 4
        dec     ecx
        jnz     .selectloop
        pop     eax
        jmp     .done_redraw
.greyasterisk:
        mov     ecx, [ebp + panel1_numfiles - panel1_data]
        mov     esi, [ebp + panel1_files - panel1_data]
        jecxz   .galoopdone
.galoop:
        lodsd
        cmp     word [eax+40], '..'
        jnz     @f
        cmp     byte [eax+42], 0
        jz      .gacont
@@:
        xor     byte [eax+303], 1
        mov     edx, [eax+32]
        test    byte [eax+303], 1
        jz      .gadel
        add     dword [ebp + panel1_selected_size - panel1_data], edx
        mov     edx, [eax+36]
        adc     dword [ebp + panel1_selected_size+4 - panel1_data], edx
        inc     dword [ebp + panel1_selected_num - panel1_data]
        jmp     .gacont
.gadel:
        sub     dword [ebp + panel1_selected_size - panel1_data], edx
        mov     edx, [eax+36]
        sbb     dword [ebp + panel1_selected_size+4 - panel1_data], edx
        dec     dword [ebp + panel1_selected_num - panel1_data]
.gacont:
        loop    .galoop
.galoopdone:
        jmp     .done_redraw

@@:
        call    close_plugin_panel
close_plugin_panels:
        cmp     [ebp + panel1_hPlugin - panel1_data], 0
        jnz     @b
        ret

close_plugin_panel:
; close plugin and restore old directory
        mov     esi, [ebp + panel1_parents - panel1_data]
        add     esi, [ebp + panel1_parents_sz - panel1_data]
@@:
        dec     esi
        cmp     byte [esi-1], 0
        jnz     @b
        push    esi
        lea     edi, [ebp + panel1_dir - panel1_data]
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        pop     esi
        sub     esi, 9
        mov     edx, [esi]      ; hPlugin
        mov     ebx, [esi+4]    ; hFile
        sub     esi, [ebp + panel1_parents - panel1_data]
        mov     [ebp + panel1_parents_sz - panel1_data], esi
        xchg    edx, [ebp + panel1_hPlugin - panel1_data]
        xchg    ebx, [ebp + panel1_hFile - panel1_data]
        push    edx ebx
        lea     ebx, [ebp + panel1_plugin_info - panel1_data]
        and     dword [ebx], 0
        mov     eax, [ebp + panel1_hPlugin - panel1_data]
        test    eax, eax
        jz      @f
        push    ebp
        push    ebx
        push    [ebp + panel1_hFile - panel1_data]
        call    [eax+PluginInfo.GetOpenPluginInfo]
        pop     ebp
@@:
        pop     ebx edx

close_handle_if_unused:
; edx=hPlugin, ebx=hFile
        push    ebp
        xor     ecx, ecx
@@:
        mov     eax, [screens]
        mov     ebp, [eax+ecx*8+4]
        mov     eax, [eax+ecx*8]
        call    [eax+screen_vtable.IsHandleUsed]
        jz      .used
        inc     ecx
        cmp     ecx, [num_screens]
        jb      @b
        push    ebx
        call    [edx+PluginInfo.ClosePlugin]
.used:
        pop     ebp
        ret

panels_IsHandleUsed:
; edx=hPlugin, ebx=hFile
        mov     ebp, panel1_data
        call    .1
        jz      .ret
        mov     ebp, panel2_data

.1:
        cmp     edx, [ebp+panel1_hPlugin-panel1_data]
        jnz     @f
        cmp     ebx, [ebp+panel1_hFile-panel1_data]
        jz      .ret
@@:
        mov     esi, [ebp + panel1_parents_sz - panel1_data]
        test    esi, esi
        jnz     @f
        inc     esi
.ret:
        ret
@@:
        add     esi, [ebp + panel1_parents - panel1_data]
@@:
        dec     esi
        cmp     byte [esi-1], 0
        jnz     @b
        sub     esi, 9
        cmp     edx, [esi]      ; hPlugin
        jnz     .no
        mov     ebx, [esi+4]    ; hFile
        jz      .ret
.no:
        cmp     esi, [ebp + panel1_parents - panel1_data]
        jnz     @b
        inc     esi
        ret

panels_OnExit:
; close plugin panels
        mov     ebp, panel1_data
        call    close_plugin_panels
        mov     ebp, panel2_data
        call    close_plugin_panels
        ret

panels_OnRedraw:
        or      [cursor_x], -1
        or      [cursor_y], -1
        call    draw_cmdbar
        mov     ebp, panel1_data
        call    calc_colwidths
        call    draw_panel
        mov     ebp, panel2_data
        call    calc_colwidths
        call    draw_panel
        ret

init_console:
        mov     ecx, [console_data_ptr]
        call    pgfree
        mov     eax, [cur_width]
        mul     [cur_height]
        mov     ecx, eax
        push    ecx
        shl     ecx, 2
        call    pgalloc
        pop     ecx
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

quick_find:
        cmp     [ebp + panel1_numfiles - panel1_data], 0
        jz      .nof
        mov     ecx, [ebp + panel1_index - panel1_data]
.scanloop:
        mov     edi, ecx
        shl     edi, 2
        add     edi, [ebp + panel1_files - panel1_data]
        mov     edi, [edi]
        add     edi, 40
        mov     esi, quick_search_buf + 12
@@:
        lodsb
        test    al, al
        jz      .ok
        call    match_symbol
        jnz     .no
        inc     edi
        jmp     @b
.no:
        inc     ecx
        cmp     ecx, [ebp + panel1_numfiles - panel1_data]
        jb      @f
        xor     ecx, ecx
@@:
        cmp     ecx, [ebp + panel1_index - panel1_data]
        jnz     .scanloop
.nof:
        stc
        ret
.ok:
        cmp     ecx, [ebp + panel1_index - panel1_data]
        jz      .ret
        mov     [ebp + panel1_index - panel1_data], ecx
        mov     eax, [ebp + panel1_height - panel1_data]
        shr     eax, 1
        sub     ecx, eax
        jae     @f
        xor     ecx, ecx
@@:
        mov     eax, [ebp + panel1_numfiles - panel1_data]
        sub     eax, [ebp + panel1_colst - panel1_data]
        jnc     @f
        xor     eax, eax
        xor     ecx, ecx
@@:
        cmp     ecx, eax
        jb      @f
        mov     ecx, eax
@@:
        mov     [ebp + panel1_start - panel1_data], ecx
        call    draw_panel
.ret:
        clc
        ret

quick_find_next:
        cmp     [ebp + panel1_numfiles - panel1_data], 0
        jz      quick_find.nof
        mov     ecx, [ebp + panel1_index - panel1_data]
        jmp     quick_find.no

quick_find_prev:
        cmp     [ebp + panel1_numfiles - panel1_data], 0
        jz      quick_find.nof
        mov     ecx, [ebp + panel1_index - panel1_data]
.scanloop:
        dec     ecx
        jns     @f
        mov     ecx, [ebp + panel1_numfiles - panel1_data]
        dec     ecx
@@:
        cmp     ecx, [ebp + panel1_index - panel1_data]
        jz      quick_find.nof
        mov     edi, ecx
        shl     edi, 2
        add     edi, [ebp + panel1_files - panel1_data]
        mov     edi, [edi]
        add     edi, 40
        mov     esi, quick_search_buf + 12
@@:
        lodsb
        test    al, al
        jz      quick_find.ok
        call    match_symbol
        jnz     .scanloop
        inc     edi
        jmp     @b

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
        push    3
        pop     edx
        cmp     [ebp + panel1_hPlugin - panel1_data], 0
        jz      .native
        mov     esi, [ebp + panel1_parents - panel1_data]
        add     esi, [ebp + panel1_parents_sz - panel1_data]
@@:
        dec     esi
        cmp     byte [esi-1], 0
        jz      @f
        cmp     byte [esi-1], '/'
        jnz     @b
@@:
        push    ecx edi
        shr     ecx, 1
@@:
        lodsb
        test    al, al
        jz      @f
        stosb
        loop    @b
@@:
        sub     edi, [esp]
        sub     [esp+4], edi
        add     [esp], edi
        pop     edi ecx
        lea     esi, [ebp + panel1_dir - panel1_data]
        cmp     byte [esi+1], 0
        jnz     @f
        inc     esi
@@:
        jmp     .main
.native:
        lea     esi, [ebp + panel1_dir - panel1_data]
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
.main:
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
        mov     esi, [esp+4]
        mov     al, '/'
        stosb
        dec     ecx
        jz      .nodir
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
        mov     al, 65
        mov     ebx, [MemForImage]
        test    ebx, ebx
        jz      @f
        mov     ecx, [cur_width]
        imul    ecx, font_width*10000h
        mov     cx, word [cur_height]
        imul    cx, font_height
        mov     edx, [skinh]
        add     edx, 5*10000h
        mov     esi, 8
        mov     edi, console_colors
        xor     ebp, ebp
        int     0x40
@@:
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
        mov     [bMemForImageValidData], byte 1
        cmp     [MemForImage], 0
        jnz     .allocated
; allocate memory for image
        mov     ecx, [cur_width]
        imul    ecx, [cur_height]
        imul    ecx, font_width*font_height
        call    pgalloc
        test    eax, eax
        jz      draw_image.nomem
        mov     [MemForImage], eax
        mov     [bMemForImageValidData], byte 0
.allocated:
        push    ebp
        and     [max_x], 0
        or      [min_x], -1
        and     [max_y], 0
        or      [min_y], -1
        mov     eax, [cursor_y]
        mul     [cur_width]
        add     eax, [cursor_x]
        add     eax, eax
        add     eax, [console_data_ptr]
        xchg    [cur_cursor_pos], eax
        mov     [old_cursor_pos], eax
        mov     edi, [MemForImage]
        mov     esi, [console_data_ptr]
        mov     ecx, [cur_height]
.lh:
        push    ecx
        mov     ecx, [cur_width]
.lw:
        push    ecx
        mov     eax, [cur_width]
        imul    eax, [cur_height]
        mov     ebx, [esi]
        cmp     [bMemForImageValidData], 0
        jz      @f
        cmp     esi, [cur_cursor_pos]
        jz      @f
        cmp     esi, [old_cursor_pos]
        jz      @f
        cmp     bx, [eax*2+esi]
        jnz     @f
        inc     esi
        inc     esi
        jmp     .skip_symbol
@@:
        mov     [eax*2+esi], bx
        cmp     ecx, [min_x]
        ja      @f
        mov     [min_x], ecx
@@:
        cmp     ecx, [max_x]
        jb      @f
        mov     [max_x], ecx
@@:
        mov     eax, [esp+4]
        mov     [min_y], eax
        cmp     eax, [max_y]
        jb      @f
        mov     [max_y], eax
@@:
        push    edi
        xor     eax, eax
        mov     al, [esi+1]
        and     al, 0xF
        mov     ebx, eax
        mov     al, [esi+1]
        shr     al, 4
        mov     ebp, eax
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
        mov     eax, [cur_width]
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
        pop     edi
.skip_symbol:
        pop     ecx
        add     edi, font_width
        dec     ecx
        jnz     .lw
        mov     eax, [cur_width]
        imul    eax, (font_height-1)*font_width
        add     edi, eax
        pop     ecx
        dec     ecx
        jnz     .lh
; cursor
        mov     eax, [cursor_y]
        inc     eax
        jz      .nocursor
        mul     [cur_width]
        imul    eax, font_height*font_width
        mov     edx, [cursor_x]
        inc     edx
        imul    edx, font_width
        add     eax, edx
        add     eax, [MemForImage]
        mov     edx, [cur_width]
        imul    edx, font_width
        neg     edx
        mov     ecx, [cursor_size]
.cursor_loop:
        push    ecx
        mov     ecx, font_width
        add     eax, edx
        push    eax
@@:
;;        add     byte [eax-1], 0x10
        xor     byte [eax-1], 7
        sub     eax, 1
        loop    @b
        pop     eax
        pop     ecx
        loop    .cursor_loop
.nocursor:
        cmp     [min_y], -1
        jz      .nodraw
        mov     ecx, [cur_width]
        mov     ebx, [cur_height]
        mov     eax, ebx
        sub     ebx, [max_y]
        sub     eax, [min_y]
        sub     eax, ebx
        inc     eax
        imul    ebp, eax, font_height
        mov     edx, ecx
        sub     edx, [max_x]
        imul    edx, font_width
        mov     eax, edx
        shl     edx, 16
        imul    dx, bx, font_height
        imul    ebx, [cur_width]
        mov     ecx, [max_x]
        sub     ecx, [min_x]
        inc     ecx
        imul    ecx, font_width*10000h
        add     ecx, ebp
        imul    ebx, font_width*font_height
        add     ebx, [MemForImage]
        add     ebx, eax
        add     edx, [skinh]
        add     edx, 5*10000h
        imul    esi, [cur_width], font_width
        mov     ebp, ecx
        shr     ebp, 16
        sub     esi, ebp
        mov     ebp, esi
        push    65
        pop     eax
        mov     edi, console_colors
        push    8
        pop     esi
        int     40h
.nodraw:
        pop     ebp
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
        mov     esi, [active_screen_keybar]
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
        cmp     [bDisplayQuickSearch], 0
        jz      @f
        push    QuickSearchDlg
        call    DrawDialogBox
@@:
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

calc_colwidths:
; in: ebp->panel data
        imul    esi, [ebp + panel1_colmode - panel1_data], PanelMode.size
        add     esi, colmodes
        lodsd
        mov     ecx, eax
        lea     edx, [ecx-1]
        xor     ebx, ebx
        add     esi, PanelMode.ColumnWidths-4
        push    ecx esi
        xor     eax, eax
.loop:
        lodsb
        add     edx, eax
        cmp     al, 1
        adc     ebx, 0
        loop    .loop
        pop     esi ecx
        lea     edi, [ebp + panel1_colwidths - panel1_data]
        test    ebx, ebx
        jz      .loop2
        mov     eax, [ebp + panel1_width - panel1_data]
        dec     eax
        dec     eax
        sub     eax, edx
        jae     @f
        xor     eax, eax
@@:
        xor     edx, edx
        div     ebx
        test    eax, eax
        jnz     @f
        xor     edx, edx
        inc     eax
@@:
        push    eax
.loop2:
        movzx   eax, byte [esi]
        inc     esi
        test    eax, eax
        jnz     @f
        pop     eax
        push    eax
        dec     ebx
        cmp     ebx, edx
        jae     @f
        inc     eax
@@:
        stosd
        loop    .loop2
        pop     eax
        and     dword [edi], 0
        ret

GetPanelTitle_default:
        mov     edi, [esp+8]
        mov     ecx, 1024
        mov     esi, [esp+12]
        test    esi, esi
        jz      .nohost
        mov     edx, esi
@@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, '/'
        jnz     @b
        mov     edx, esi
        jmp     @b
@@:
        mov     esi, edx
@@:
        lodsb
        stosb
        test    al, al
        loopnz  @b
        dec     edi
        inc     ecx
.nohost:
        mov     esi, [esp+16]
        cmp     word [esi], '/'
        jz      .nodir
@@:
        lodsb
        stosb
        test    al, al
        loopnz  @b
        dec     edi
.nodir:
        mov     byte [edi], 0
        ret     10h

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

        imul    esi, [ebp + panel1_colmode - panel1_data], PanelMode.size
        add     esi, colmodes
        lodsd
        mov     ecx, eax        ; number of columns
        lea     ebx, [ebp + panel1_colwidths - panel1_data]
        mov     eax, [ebp + panel1_left - panel1_data]
        inc     eax
        mov     [column_left], eax
        mov     eax, [ebp + panel1_top - panel1_data]
        inc     eax
        mov     [column_top], eax
        mov     eax, [ebp + panel1_height - panel1_data]
        sub     eax, 4
        mov     [column_height], eax
        dec     eax
        mov     [ebp + panel1_colsz - panel1_data], eax
        and     [ebp + panel1_colst - panel1_data], 0
        mov     eax, [ebp + panel1_start - panel1_data]
        mov     [column_index], eax
        mov     [last_column_index], eax
.columns_loop:
        push    ecx
        mov     eax, [ebx]
        add     ebx, 4
        mov     [column_width], eax
        cmp     ecx, 1
        jz      .skip_right_border
        add     eax, [column_left]
        mov     edx, [ebp + panel1_top - panel1_data]
        call    get_console_ptr
        mov     ah, [panel_header_color]
        mov     al, 0xD1
        mov     [edi], ax
        add     edi, [cur_width]
        add     edi, [cur_width]
        mov     ecx, [column_height]
        mov     al, 0xB3
@@:
        mov     [edi], ax
        add     edi, [cur_width]
        add     edi, [cur_width]
        loop    @b
        mov     al, 0xC1
        stosw
.skip_right_border:
        mov     eax, [column_left]
        mov     edx, [column_top]
        call    get_console_ptr
; заголовок столбца
        push    edi
        mov     edx, [esi]
        and     edx, 0xF
        mov     edx, [column_headers+edx*4]
        movzx   ecx, byte [edx]
        inc     edx
        cmp     ecx, [column_width]
        jb      @f
        mov     ecx, [column_width]
@@:
        push    ecx
        sub     ecx, [column_width]
        neg     ecx
        shr     ecx, 1
        mov     al, ' '
        mov     ah, [column_header_color]
        rep     stosw
        pop     ecx
        jecxz   .skipcopyhdr
        push    ecx
@@:
        mov     al, [edx]
        inc     edx
        stosw
        loop    @b
        pop     ecx
.skipcopyhdr:
        sub     ecx, [column_width]
        neg     ecx
        inc     ecx
        shr     ecx, 1
        mov     al, ' '
        rep     stosw
        pop     edi
        add     edi, [cur_width]
        add     edi, [cur_width]
; сам столбец
        mov     eax, [esi]
        and     eax, 0xF
        push    ebx esi
        call    dword [draw_column_proc+eax*4]
        pop     esi ebx
        inc     esi
        mov     eax, [column_width]
        inc     eax
        add     [column_left], eax
        pop     ecx
        dec     ecx
        jnz     .columns_loop
; Заголовок панели (текущая папка)
        lea     esi, [ebp + panel1_dir - panel1_data]
        mov     eax, [ebp + panel1_hPlugin - panel1_data]
        test    eax, eax
        jz      .native
        push    ebp
        push    esi
        mov     esi, [ebp + panel1_parents - panel1_data]
        add     esi, [ebp + panel1_parents_sz - panel1_data]
@@:
        dec     esi
        cmp     byte [esi-1], 0
        jnz     @b
        push    esi
        push    execdata
        push    [ebp + panel1_hFile - panel1_data]
        call    [eax+PluginInfo.GetPanelTitle]
        pop     ebp
        mov     esi, execdata
.native:
        mov     edi, cur_header
        mov     ecx, [ebp + panel1_width - panel1_data]
        sub     ecx, 6
        cmp     byte [esi], '/'
        jnz     .copy_rest
        dec     ecx
        movsb
@@:
        lodsb
        stosb
        dec     ecx
        test    al, al
        jz      .header_created
        cmp     al, '/'
        jnz     @b
.copy_rest:
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
        mov     eax, [ebp + panel1_selected_num - panel1_data]
        test    eax, eax
        jz      .skip_selected_info
; Информация о выделенных файлах
        push    dword [ebp + panel1_selected_size+4 - panel1_data]
        push    dword [ebp + panel1_selected_size - panel1_data]
        call    fill_total_info
        mov     eax, [ebp + panel1_width - panel1_data]
        sub     eax, 2
        cmp     ecx, eax
        jbe     @f
        mov     ecx, eax
        mov     edi, saved_file_name+512
        sub     edi, eax
        mov     byte [edi], '.'
        mov     word [edi+1], '..'
@@:
        mov     esi, edi
        sub     eax, ecx
        shr     eax, 1
        inc     eax
        add     eax, [ebp + panel1_left - panel1_data]
        mov     edx, [ebp + panel1_top - panel1_data]
        add     edx, [ebp + panel1_height - panel1_data]
        sub     edx, 3
        call    get_console_ptr
        mov     ah, [panel_numselected_color]
@@:
        lodsb
        stosw
        loop    @b
.skip_selected_info:
; Информация об общем числе и размере файлов панели
        mov     eax, [ebp + panel1_total_num - panel1_data]
        push    dword [ebp + panel1_total_size+4 - panel1_data]
        push    dword [ebp + panel1_total_size - panel1_data]
        call    fill_total_info
        mov     eax, [ebp + panel1_width - panel1_data]
        sub     eax, 2
        cmp     ecx, eax
        jbe     @f
        mov     ecx, eax
        mov     byte [edi+ecx-3], '.'
        mov     word [edi+ecx-2], '..'
@@:
        sub     eax, ecx
        shr     eax, 1
        inc     eax
        add     eax, [ebp + panel1_left - panel1_data]
        add     edx, [ebp + panel1_top - panel1_data]
        add     edx, [ebp + panel1_height - panel1_data]
        dec     edx
        mov     esi, edi
        call    get_console_ptr
        mov     ah, [panel_number_color]
@@:
        lodsb
        stosw
        loop    @b
        cmp     [ebp + panel1_numfiles - panel1_data], 0
        jz      .skip_curinfo
; Информация о текущем файле
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
        cmp     [ebp + panel1_colmode - panel1_data], 3
        jz      .show_curname
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
.show_curname:
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

fill_total_info:
        mov     edi, saved_file_name+511
        mov     byte [edi], ' '
if lang eq ru
        mov     byte [edi-1], 'е'
        dec     edi
        cmp     eax, 1
        jz      @f
        mov     word [edi-1], 'ах'
        dec     edi
@@:
        mov     dword [edi-4], 'файл'
        mov     byte [edi-5], ' '
        sub     edi, 5
else
        cmp     eax, 1
        jz      @f
        dec     edi
        mov     byte [edi], 's'
@@:
        mov     dword [edi-4], 'file'
        mov     byte [edi-5], ' '
        sub     edi, 5
end if
        xor     edx, edx
        push    10
        pop     ecx
@@:
        div     ecx
        dec     edi
        add     dl, '0'
        mov     [edi], dl
        xor     edx, edx
        test    eax, eax
        jnz     @b
if lang eq ru
        mov     dword [edi-4], 'т в '
        mov     dword [edi-8], ' бай'
        sub     edi, 8
else
        mov     dword [edi-4], ' in '
        mov     dword [edi-8], 'ytes'
        mov     word [edi-10], ' b'
        sub     edi, 10
end if
        lea     esi, [edi-3]
        mov     edx, [esp+8]
        mov     eax, [esp+4]
.selsizel:
        push    edx
        mov     ebx, edx
        xor     edx, edx
        add     ebx, ebx
        adc     edx, edx
        push    ebx
        push    edx
        add     ebx, ebx
        adc     edx, edx
        add     ebx, [esp+4]
        adc     edx, [esp]
        add     esp, 8
        add     eax, ebx
        adc     edx, 0
        div     ecx
        dec     edi
        cmp     edi, esi
        jae     @f
        mov     byte [edi], ','
        dec     edi
        sub     esi, 4
@@:
        pop     ebx
        add     dl, '0'
        mov     byte [edi], dl
        xchg    eax, ebx
        mul     [muldiv10]
        add     eax, ebx
        adc     edx, 0
        mov     ebx, eax
        or      ebx, edx
        jnz     .selsizel
        dec     edi
        mov     byte [edi], ' '
        mov     ecx, saved_file_name+512
        sub     ecx, edi
        ret     8

get_file_color:
        mov     ah, [esi + 6]
        cmp     ebp, [active_panel]
        jnz     @f
        cmp     ecx, [ebp + panel1_index - panel1_data]
        jnz     @f
        mov     ah, [esi + 7]
@@:
        test    byte [esi + 303], 1
        jnz     @f
        mov     ah, [esi + 4]
        cmp     ebp, [active_panel]
        jnz     @f
        cmp     ecx, [ebp + panel1_index - panel1_data]
        jnz     @f
        mov     ah, [esi + 5]
@@:
        ret

draw_name_column:
        mov     eax, [column_index]
        mov     [last_column_index], eax
        mov     edx, [ebp + panel1_numfiles - panel1_data]
        mov     ecx, [column_height]
        dec     ecx
.l:
        cmp     [column_index], edx
        jae     .ret
        push    ecx
        mov     ecx, [column_index]
        mov     esi, [ebp + panel1_files - panel1_data]
        mov     esi, [esi+ecx*4]
        mov     ebx, [esi]
        call    get_file_color
        add     esi, 40
        mov     ecx, [column_width]
        push    edi
@@:
        lodsb
        test    al, al
        jz      @f
        stosw
        loop    @b
        lodsb
        test    al, al
        jz      @f
        mov     byte [edi], '}'
        jmp     .continue
@@:
        test    bl, 10h
        jnz     .noalignext
        mov     ebx, [ebp + panel1_colmode - panel1_data]
; sizeof(PanelMode) = 40
        lea     ebx, [ebx+ebx*4]
        cmp     [colmodes+ebx*8+PanelMode.bAlignExtensions], 0
        jz      .noalignext
        push    ecx
        sub     ecx, [column_width]
        neg     ecx
        jz      .noalignext2
        dec     esi
@@:
        dec     esi
        cmp     byte [esi], '.'
        loopnz  @b
        jnz     .noalignext2
        inc     esi
        sub     ecx, [column_width]
        neg     ecx
        sub     ecx, [esp]
        sub     edi, ecx
        sub     edi, ecx
        dec     ecx
        mov     ebx, [esp+4]
        cmp     ecx, 3
        ja      @f
        mov     cl, 3
@@:
        sub     ecx, [column_width]
        sub     ebx, edi
        sar     ebx, 1
        sub     ebx, ecx
        pop     ecx
        inc     ecx
        push    0
        cmp     ecx, ebx
        jbe     @f
        sub     ecx, ebx
        mov     [esp], ecx
        mov     ecx, ebx
@@:
        mov     al, ' '
        rep     stosw
@@:
        lodsb
        test    al, al
        jz      .noalignext2
        stosw
        jmp     @b
.noalignext2:
        pop     ecx
.noalignext:
        mov     al, ' '
        rep     stosw
.continue:
        pop     edi
        add     edi, [cur_width]
        add     edi, [cur_width]
        inc     [column_index]
        pop     ecx
        dec     ecx
        jnz     .l
.ret:
        mov     eax, [ebp + panel1_colsz - panel1_data]
        add     [ebp + panel1_colst - panel1_data], eax
        cmp     ebp, panel1_data
        jnz     .ret2
; Число экранов
; calculate number of viewer and editor screens
        xor     ebx, ebx
        xor     edx, edx
        mov     ecx, [num_screens]
        mov     esi, [screens]
.3:
        lodsd
        cmp     eax, viewer_vtable
        jnz     @f
        inc     ebx
@@:
        cmp     eax, editor_vtable
        jnz     @f
        inc     edx
@@:
        lodsd
        loop    .3
        mov     eax, ebx
        mov     esi, edx
        or      eax, edx
        jz      .ret2
        xor     eax, eax
        xor     edx, edx
        call    get_console_ptr
        mov     al, '['
        stosb
        mov     al, [panel_nscreens_color]
        stosb
        xchg    eax, ebx
.5:
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
        stosb
        mov     al, bl
        stosb
        jmp     @b
@@:
        test    esi, esi
        jz      .4
        mov     al, '+'
        stosb
        mov     al, bl
        stosb
        xor     eax, eax
        xchg    eax, esi
        jmp     .5
.4:
        mov     al, ']'
        mov     ah, [panel_nscreens_color]
        stosw
.ret2:
draw_empty_column:
        ret

draw_size_column:
        add     edi, [column_width]
        add     edi, [column_width]
        dec     edi
        dec     edi
        std
        mov     ecx, [column_height]
        dec     ecx
        push    [last_column_index]
.l:
        mov     edx, [ebp + panel1_numfiles - panel1_data]
        cmp     [last_column_index], edx
        jae     .ret
        push    ecx
        push    edi
        mov     ecx, [last_column_index]
        mov     esi, [ebp + panel1_files - panel1_data]
        mov     esi, [esi+ecx*4]
        call    get_file_color
        mov     ecx, [column_width]
@@:
        mov     ebx, eax
        cmp     word [esi+40], '..'
        jnz     .nodotdot
        cmp     byte [esi+42], 0
        jnz     .nodotdot
if lang eq ru
        mov     al, 'х'
        stosw
        mov     al, 'р'
        stosw
        mov     al, 'е'
        stosw
        mov     al, 'в'
        stosw
        mov     al, 'В'
        stosw
        sub     ecx, 5
else
        mov     al, ' '
        stosw
        stosw
        mov     al, 'p'
        stosw
        mov     al, 'U'
        stosw
        sub     ecx, 4
end if
        jmp     .size_written
.nodotdot:
        test    byte [esi], 10h
        jz      .nofolder
if lang eq ru
        mov     al, 'а'
        stosw
        mov     al, 'к'
        stosw
        mov     al, 'п'
        stosw
        mov     al, 'а'
        stosw
        mov     al, 'П'
        stosw
        sub     ecx, 5
else
        mov     al, 'r'
        stosw
        mov     al, 'e'
        stosw
        mov     al, 'd'
        stosw
        mov     al, 'l'
        stosw
        mov     al, 'o'
        stosw
        mov     al, 'F'
        stosw
        sub     ecx, 6
end if
        jmp     .size_written
.nofolder:
        mov     eax, [esi+32]
        mov     edx, [esi+36]
        test    edx, edx
        jz      .less_4G
        cmp     edx, 10000/4*1024
        jb      .giga
        mov     al, 'T'
        shr     edx, 10
        jmp     .write_letter
.giga:
        mov     al, 'G'
        shl     edx, 2
        jmp     .write_letter
.less_4G:
        mov     edx, eax
        cmp     eax, 1000000
        jb      .byte
        cmp     eax, 10000*1024
        jb      .kilo
        mov     al, 'M'
        shr     edx, 20
        jmp     .write_letter
.kilo:
        mov     al, 'K'
        shr     edx, 10
.write_letter:
        mov     ah, bh
        stosw
        mov     al, ' '
        stosw
        dec     ecx
        dec     ecx
.byte:
        xchg    eax, edx
        xor     edx, edx
        div     [_10d]
        xchg    eax, edx
        add     al, '0'
        mov     ah, bh
        stosw
        dec     ecx
        test    edx, edx
        jnz     .byte
.size_written:
        mov     eax, ebx
        test    ecx, ecx
        jle     @f
        mov     al, ' '
        rep     stosw
@@:
        mov     byte [edi+1], ah
        pop     edi
        add     edi, [cur_width]
        add     edi, [cur_width]
        inc     [last_column_index]
        pop     ecx
        dec     ecx
        jnz     .l
.ret:
        pop     [last_column_index]
        cld
        ret

draw_date_column:
        mov     ecx, [column_height]
        dec     ecx
        push    [last_column_index]
.l:
        mov     edx, [ebp + panel1_numfiles - panel1_data]
        cmp     [last_column_index], edx
        jae     .ret
        push    ecx
        push    edi
        mov     ecx, [last_column_index]
        mov     esi, [ebp + panel1_files - panel1_data]
        mov     esi, [esi+ecx*4]
        call    get_file_color
        mov     bh, ah
        mov     byte [edi-1], bh
        mov     al, [esi+28]
        aam
        add     eax, '00'
        push    eax
        mov     al, ah
        mov     ah, bh
        stosw
        pop     eax
        mov     ah, bh
        stosw
        mov     al, '.'
        stosw
        mov     al, [esi+29]
        aam
        add     eax, '00'
        push    eax
        mov     al, ah
        mov     ah, bh
        stosw
        pop     eax
        mov     ah, bh
        stosw
        mov     al, '.'
        stosw
        movzx   eax, word [esi+30]
        xor     edx, edx
        div     [_10d]
        xchg    eax, edx
        add     al, '0'
        mov     ah, bh
        mov     [edi+2], ax
        xchg    eax, edx
        xor     edx, edx
        div     [_10d]
        xchg    eax, edx
        add     al, '0'
        mov     ah, bh
        stosw
        pop     edi
        add     edi, [cur_width]
        add     edi, [cur_width]
        inc     [last_column_index]
        pop     ecx
        dec     ecx
        jnz     .l
.ret:
        pop     [last_column_index]
        cld
        ret

draw_time_column:
        mov     ecx, [column_height]
        dec     ecx
        push    [last_column_index]
.l:
        mov     edx, [ebp + panel1_numfiles - panel1_data]
        cmp     [last_column_index], edx
        jae     .ret
        push    ecx
        push    edi
        mov     ecx, [last_column_index]
        mov     esi, [ebp + panel1_files - panel1_data]
        mov     esi, [esi+ecx*4]
        call    get_file_color
        mov     bh, ah
        mov     byte [edi-1], bh
        mov     al, [esi+26]
        aam
        add     eax, '00'
        push    eax
        mov     al, ah
        mov     ah, bh
        stosw
        pop     eax
        mov     ah, bh
        stosw
        mov     al, ':'
        stosw
        mov     al, [esi+25]
        aam
        add     eax, '00'
        push    eax
        mov     al, ah
        mov     ah, bh
        stosw
        pop     eax
        mov     ah, bh
        stosw
        pop     edi
        add     edi, [cur_width]
        add     edi, [cur_width]
        inc     [last_column_index]
        pop     ecx
        dec     ecx
        jnz     .l
.ret:
        pop     [last_column_index]
        cld
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
        xor     eax, eax
        mov     [ebp + panel1_total_num - panel1_data], eax
        mov     dword [ebp + panel1_total_size - panel1_data], eax
        mov     dword [ebp + panel1_total_size+4 - panel1_data], eax
        mov     [ebp + panel1_selected_num - panel1_data], eax
        mov     dword [ebp + panel1_selected_size - panel1_data], eax
        mov     dword [ebp + panel1_selected_size+4 - panel1_data], eax
.retry:
        mov     eax, [ebp + panel1_hPlugin - panel1_data]
        test    eax, eax
        jz      .native
        mov     ecx, [dirinfo.size]
        test    [ebp + panel1_plugin_flags - panel1_data], 1
        jz      @f
        dec     ecx     ; reserve one entry for '..'
@@:
        push    ebp
        push    [dirinfo.dirdata]
        push    ecx
        push    0
        push    [ebp + panel1_hFile - panel1_data]
        call    [eax + PluginInfo.ReadFolder]
        pop     ebp
        mov     ebx, [dirinfo.dirdata]
        mov     ebx, [ebx+4]
        jmp     .read
.native:
        push    70
        pop     eax
        mov     ebx, dirinfo
        int     40h
.read:
        test    eax, eax
        jz      .ok
        cmp     eax, 6
        jz      .ok
; Failed to read folder, notify user
        cmp     [bSilentFolderMode], 0
        jnz     .dont_notify
        push    dword aContinue
        push    dword aRetry
        mov     edx, esp
        call    get_error_msg
        push    [dirinfo.name]
        push    dword aCannotReadFolder
        push    eax
        mov     eax, esp
        push    edx
        push    2
        push    eax
        push    3
        call    SayErr
        add     esp, 5*4
        test    eax, eax
        jz      .retry
.dont_notify:
; If not on plugin panel, try to return to previous directory
        cmp     [ebp + panel1_hPlugin - panel1_data], 0
        jnz     @f
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
; Try to read parent folder
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
        cmp     [ebp + panel1_hPlugin - panel1_data], 0
        jz      .4
        cmp     edx, 1
        ja      .up
; чтение с панели плагина обломалось по полной
; при этом мы уже в корне
; делаем вид, что функция чтения вернула 0 элементов
; (если нужен вход "..", он будет добавлен автоматически)
        xor     ebx, ebx        ; 0 items read
        mov     eax, [dirinfo.dirdata]
        mov     [eax+8], ebx    ; 0 items total
        jmp     .ok
.4:
        cmp     edx, 2
        jbe     .noup
.up:
        stosb
        jmp     read_folder
.noup:
; There is no parent folder, and we are not on plugin panel
; Try to read ramdisk
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
        push    dword [eax+8]
        mov     ecx, [ebp + panel1_files - panel1_data]
        call    pgfree
        pop     ecx
        add     ecx, 0xF
        and     ecx, not 0xF
        push    ecx
        imul    ecx, 4+304
        add     ecx, 32
        call    xpgalloc
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
        xor     edx, edx
        mov     [ebp + panel1_start - panel1_data], edx
        mov     [ebp + panel1_index - panel1_data], edx
        mov     [ebp + panel1_start - panel1_data], edx
        mov     edi, [ebp + panel1_files - panel1_data]
        mov     eax, [ebp + panel1_nfa - panel1_data]
        lea     eax, [edi + eax*4 + 32]
        mov     ecx, [eax-32+4]
        test    ecx, ecx
        jz      .loopdone
; Игнорируем специальные входы, соответствующие папке '.' и метке тома
.ptrinit:
        cmp     word [eax+40], '.'
        jz      .loopcont
        test    byte [eax], 8
        jnz     .loopcont
        mov     byte [eax+303], 0
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
        xor     ebx, ebx
        test    byte [eax], 10h
        setz    bl
        add     [ebp + panel1_total_num - panel1_data], ebx
        mov     ebx, dword [eax+32]
        add     dword [ebp + panel1_total_size - panel1_data], ebx
        mov     ebx, dword [eax+36]
        adc     dword [ebp + panel1_total_size+4 - panel1_data], ebx
.dotdot:
; подсветка
;        call    insert_last_dot
        call    highlight_init
;        call    delete_last_dot
.loopcont:
        add     eax, 304
        dec     ecx
        jnz     .ptrinit
.loopdone:
        push    edi
        sub     edi, [ebp + panel1_files - panel1_data]
        shr     edi, 2
        mov     [ebp + panel1_numfiles - panel1_data], edi
        pop     edi
        test    edx, edx
        jnz     @f
        test    [ebp + panel1_plugin_flags - panel1_data], 1
        jz      @f
        mov     ecx, [ebp + panel1_numfiles - panel1_data]
        inc     [ebp + panel1_numfiles - panel1_data]
        lea     esi, [edi-4]
        std
        rep     movsd
        cld
        mov     eax, left_dotdot_entry
        cmp     ebp, panel1_data
        jz      .zq
        add     eax, right_dotdot_entry-left_dotdot_entry
.zq:
        stosd
        call    highlight_init
@@:
; Сортировка
sort_files:
        movzx   eax, [ebp + panel1_sortmode - panel1_data]
        mov     ebx, [compare_fns + eax*4]
.mode:
        mov     edx, [ebp + panel1_files - panel1_data]
        mov     ecx, [ebp + panel1_numfiles - panel1_data]
        jecxz   .skip
        mov     eax, [edx]
        cmp     word [eax+40], '..'
        jnz     .nodotdot
        cmp     byte [eax+42], 0
        jnz     .nodotdot
        dec     ecx
        add     edx, 4
.nodotdot:
        call    sort
.skip:
        mov     [bSilentFolderMode], 0  ; leave silent mode
        ret

highlight_init:
        pushad
        mov     ebp, eax
        lea     esi, [ebp+40]
        mov     edi, lower_file_name
        mov     edx, edi
        xor     eax, eax
@@:
        lodsb
        mov     al, [tolower_table+eax]
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
        mov     [ebp+4], ah
        mov     ah, [ebx + highlight.CursorColor]
        test    ah, ah
        jnz     @f
        mov     ah, [panel_cursor_color]
@@:
        mov     [ebp+5], ah
        mov     ah, [ebx + highlight.SelectedColor]
        test    ah, ah
        jnz     @f
        mov     ah, [panel_selected_color]
@@:
        mov     [ebp+6], ah
        mov     ah, [ebx + highlight.SelectedCursorColor]
        test    ah, ah
        jnz     @f
        mov     ah, [panel_selected_cursor_color]
@@:
        mov     [ebp+7], ah
        jmp     .doname
.highlight_test_failed:
        inc     ecx
        cmp     ecx, [highlight_num_groups]
        jb      .highlight_test_loop
.nohighlight:
        mov     ah, [panel_normal_color]
        mov     [ebp+4], ah
        mov     ah, [panel_cursor_color]
        mov     [ebp+5], ah
        mov     ah, [panel_selected_color]
        mov     [ebp+6], ah
        mov     ah, [panel_selected_cursor_color]
        mov     [ebp+7], ah
.doname:
        popad
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

ReadFolder_default:
        mov     eax, 2
        ret     10h

if 0
; Following subroutines work, but are slow
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

;tolower:
;        push    ecx
;        movzx   ecx, al
;        mov     al, [tolower_table+ecx]
;        pop     ecx
;        ret

match_symbol:
; in: al,[edi]=symbols
; out: flags as 'cmp al,[edi]'
        push    eax ecx
        movzx   ecx, al
        mov     al, [tolower_table+ecx]
        movzx   ecx, byte [edi]
        cmp     al, [tolower_table+ecx]
        pop     ecx eax
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
        cmp     al, ';'
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
        cmp     al, ';'
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
        jz      @f
        cmp     al, ';'
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
        dec     edi
        cmp     edi, edx
        jb      .done_fail
        cmp     al, '?'
        jz      .mask_symbol
        cmp     al, ']'
        jz      .list_check
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
        dec     edi
        jmp     .asterisk
@@:
        cmp     byte [esi-1], ']'
        jz      .asterisk_common
; the mask is ...<normal-symbol>*...
.asterisk_normal:
        mov     al, [esi-1]
@@:
        dec     edi
        cmp     edi, edx
        jb      .done_fail
        cmp     al, [edi]
        jnz     @b
@@:
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
        mov     edi, [associations]
        test    edi, edi
        jz      .notfound
.find_loop:
        push    esi edi
        call    strcmpi
        pop     edi esi
        pushf
@@:
        inc     edi
        cmp     byte [edi-1], 0
        jnz     @b
        popf
        jz      .found
@@:
        inc     edi
        cmp     byte [edi-1], 0
        jnz     @b
        cmp     byte [edi], 0
        jnz     .find_loop
.notfound:
; unknown extension
        stc
.found:
        pop     esi
        ret

header  db      'Kolibri Far ',version,0

nomem_draw      db      'No memory for redraw.',0
.size = $ - nomem_draw

def_left_dir    db      '/rd/1',0
def_right_dir   db      '/hd0/1',0

bSilentFolderMode db    1

bQuickSearchMode db     0
bDisplayQuickSearch db  0

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

align 4
_10d dd 10
_100d dd 100
muldiv10 dd 429496729
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

keybar_editor:
if lang eq ru
; без клавиш-модификаторов
        db      'Помощь'
        db      'Сохран'
        db      '      '
        db      '      '
        db      '      '
        db      'Просм '
        db      'Поиск '
keybar_cp2:
        db      'cp1251'
        db      '      '
        db      'Выход '
        db      'Модули'
        db      'Экраны'
; Shift
        db      '      '
        db      'Сохр.в'
        db      '      '
        db      'Редак.'
        db      '      '
        db      '      '
        db      'Дальше'
        db      'Таблиц'
        db      '      '
        db      'СхрВых'
        db      '      '
        db      '      '
; Ctrl
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      'Замена'
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
        db      '      '
        db      'Строка'
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
        db      'Save  '
        db      '      '
        db      '      '
        db      '      '
        db      'View  '
        db      'Search'
keybar_cp2:
        db      'cp1251'
        db      '      '
        db      'Quit  '
        db      'Plugin'
        db      'Screen'
; Shift
        db      '      '
        db      'SaveAs'
        db      '      '
        db      'Edit..'
        db      '      '
        db      '      '
        db      'Next  '
        db      'Table '
        db      '      '
        db      'SaveQ '
        db      '      '
        db      '      '
; Ctrl
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      '      '
        db      'Replac'
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
        db      '      '
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
cursor_size     dd      cursor_normal_size
cur_cursor_pos  dd      -1
old_cursor_pos  dd      -1

idle_interval   dd      -1

active_panel    dd      panel1_data

associations    dd      0
associations_size dd    1               ; terminating zero
associations_allocated dd 0x1000        ; start with one page

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
        .OnActivate     dd      ?
        .OnIdle         dd      ?
        .OnKey          dd      ?
        .keybar         dd      ?
        .getname        dd      ?
        .OnExit         dd      ?
        .IsHandleUsed   dd      ?
end virtual

panels_vtable:
        dd      panels_OnRedraw
        dd      0
        dd      0
        dd      panels_OnKey
        dd      keybar_panels
        dd      panels_getname
        dd      panels_OnExit
        dd      panels_IsHandleUsed

viewer_vtable:
        dd      viewer_OnRedraw
        dd      0
        dd      0
        dd      viewer_OnKey
        dd      keybar_viewer
        dd      viewer_getname
        dd      viewer_OnExit
        dd      viewer_IsHandleUsed

editor_vtable:
        dd      editor_OnRedraw
        dd      0
        dd      0
        dd      editor_OnKey
        dd      keybar_editor
        dd      editor_getname
        dd      editor_OnExit
        dd      editor_IsHandleUsed

filesearch_vtable:
        dd      filesearch_OnRedraw
        dd      filesearch_OnActivate
        dd      filesearch_OnIdle
        dd      filesearch_OnKey
        dd      0
        dd      filesearch_getname
        dd      filesearch_OnExit
        dd      filesearch_IsHandleUsed

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
.koi8r = 2
.unicode = 3

.names:
        db      'cp866   '
        db      'cp1251  '
        db      'koi8-r  '
        db      'Unicode '

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
; koi8-r
        db      0xC4,0xB3,0xDA,0xBF,0xC0,0xD9,0xC3,0xB4,0xC2,0xC1,0xC5,0xDF,0xDC,0xDB,0xDD,0xDE
        db      0xB0,0xB1,0xB2,0xF4,0xFE,0xF9,0xFB,0xF7,0xF3,0xF2,0xFF,0xF5,0xF8,0xFD,0xFA,0xF6
        db      0xCD,0xBA,0xD5,0xF1,0xD6,0xC9,0xB8,0xB7,0xBB,0xD4,0xD3,0xC8,0xBE,0xBD,0xBC,0xC6
        db      0xC7,0xCC,0xB5,0xF0,0xB6,0xB9,0xD1,0xD2,0xCB,0xCF,0xD0,0xCA,0xD8,0xD7,0xCE,0xFC
        db      0xEE,0xA0,0xA1,0xE6,0xA4,0xA5,0xE4,0xA3,0xE5,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE
        db      0xAF,0xEF,0xE0,0xE1,0xE2,0xE3,0xA6,0xA2,0xEC,0xEB,0xA7,0xE8,0xED,0xE9,0xE7,0xEA
        db      0x9E,0x80,0x81,0x96,0x84,0x85,0x94,0x83,0x95,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E
        db      0x8F,0x9F,0x90,0x91,0x92,0x93,0x86,0x82,0x9C,0x9B,0x87,0x98,0x9D,0x99,0x97,0x9A

.menu:
        db      .cp866
.menu.1:
        dd      .menu.2
        dd      0
if lang eq ru
        db      '&DOS текст (cp866)',0
else
        db      '&DOS text (cp866)',0
end if
        db      .cp1251
.menu.2:
        dd      .menu.3
        dd      .menu.1
if lang eq ru
        db      '&Windows текст (cp1251)',0
else
        db      '&Windows text (cp1251)',0
end if
        db      .koi8r
.menu.3:
        dd      .menu.4
        dd      .menu.2
if lang eq ru
        db      '&Linux текст (koi8-r)',0
else
        db      '&Linux text (koi8-r)',0
end if
        db      .unicode
.menu.4:
        dd      0
        dd      .menu.3
        db      '&Unicode',0

active_screen   dd      0
viewer_tabsize  dd      8
editor_tabsize  dd      8

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
scan2ascii:
        db      0,0,'1234567890-=',0,0
        db      'qwertyuiop[]',0,0,'as'
        db      'dfghjkl;',27h,'`',0,'\zxcv'
        db      'bnm,./',0,0,0,' ',0,0,0,0,0,0

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
        dw      0x51, 0
        dd      panels_OnKey.pgdn
        dw      0x49, 0
        dd      panels_OnKey.pgup
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
        dw      0x3E, 0
        dd      panels_OnKey.f4
        dw      0x3F, 0
        dd      panels_OnKey.f5
        dw      0x3F, 1
        dd      panels_OnKey.shift_f5
        dw      0x41, 0
        dd      panels_OnKey.f7
        dw      0x41, 0x100
        dd      panels_OnKey.alt_f7
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
        dw      2, 0x30
        dd      panels_OnKey.change_mode
        dw      3, 0x30
        dd      panels_OnKey.change_mode
        dw      4, 0x30
        dd      panels_OnKey.change_mode
        dw      5, 0x30
        dd      panels_OnKey.change_mode
repeat 10
        dw      %+1, 0x40
        dd      panels_OnKey.quick_jump
end repeat
        dw      0x52, 0
        dd      panels_OnKey.insert
        dw      0x4E, 0
        dd      panels_OnKey.greyplus
        dw      0x4A, 0
        dd      panels_OnKey.greyminus
        dw      0x37, 0
        dd      panels_OnKey.greyasterisk
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
        dw      0x41, 0
        dd      viewer_OnKey.f7
        dw      0x41, 1
        dd      viewer_OnKey.shift_f7
        dw      0x42, 0
        dd      viewer_OnKey.f8
        dw      0x42, 1
        dd      viewer_OnKey.shift_f8
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

editor_ctrlkeys:
        dw      1, 0
        dd      editor_OnKey.exit_confirm
        dw      0x0E, 0
        dd      editor_OnKey.backspace
        dw      0x3C, 0
        dd      editor_OnKey.f2
        dw      0x41, 0
        dd      editor_OnKey.f7
        dw      0x41, 1
        dd      editor_OnKey.shift_f7
        dw      0x43, 0x100
        dd      alt_f9
        dw      0x44, 0
        dd      editor_OnKey.exit_confirm
        dw      0x44, 1
        dd      editor_OnKey.exit_save
        dw      0x47, 0
        dd      editor_OnKey.home
        dw      0x48, 0
        dd      editor_OnKey.up
        dw      0x49, 0
        dd      editor_OnKey.pgup
        dw      0x4B, 0
        dd      editor_OnKey.left
        dw      0x4D, 0
        dd      editor_OnKey.right
        dw      0x4F, 0
        dd      editor_OnKey.end
        dw      0x50, 0
        dd      editor_OnKey.down
        dw      0x51, 0
        dd      editor_OnKey.pgdn
        dw      0x52, 0
        dd      editor_OnKey.ins
        dw      0x53, 0
        dd      editor_OnKey.del
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
panel_selected_color    db      1Eh
panel_border_color      db      1Bh
panel_cursor_color      db      30h
panel_selected_cursor_color db  3Eh
panel_header_color      db      1Bh
panel_active_header_color db    30h
column_header_color     db      1Eh
panel_number_color      db      1Bh
panel_numselected_color db      3Eh
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
dialog_list_color       db      70h
dialog_selected_list_color db   0Fh
dialog_scroll_list_color db     70h
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
warning_list_color      db      3Fh
warning_selected_list_color db  70h
warning_scroll_list_color db    3Fh
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
view_selected_color     db      30h
view_status_color       db      30h
view_arrows_color       db      1Eh
; Редактор
edit_normal_color       db      1Bh
edit_status_color       db      30h

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
        .IncludeAttributes      db ?
        .NormalColor            db ?
        .CursorColor            db ?
        .SelectedColor          db ?
        .SelectedCursorColor    db ?
        .Mask:                  ; ASCIIZ-string
end virtual

; all highlight masks must be in lowercase!
highlight_group0:
        db      2
        db      13h, 38h, 0, 0
        db      '*',0
highlight_group1:
        db      4
        db      13h, 38h, 0, 0
        db      '*',0
highlight_group2:
        db      10h
        db      1Fh, 3Fh, 0, 0
        db      '*|..',0
highlight_group3:
        db      10h
        db      0, 0, 0, 0
        db      '..',0
highlight_group4:
        db      0
        db      1Ah, 3Ah, 0, 0
        db      '*.exe,*.com,*.bat,*.cmd,*.kex',0
highlight_group5:
        db      0
        db      1Ah, 3Ah, 0, 0
        db      '*|*.*,readme,makefile',0
highlight_group6:
        db      0
        db      1Dh, 3Dh, 0, 0
        db      '*.rar,*.zip,*.[zj],*.[bg7]z,*.[bg]zip,*.tar,*.t[ag]z,*.ar[cj],*.r[0-9][0-9],'
        db      '*.a[0-9][0-9],*.bz2,*.cab,*.msi,*.jar,*.lha,*.lzh,*.ha,*.ac[bei],*.pa[ck],'
        db      '*.rk,*.cpio,*.rpm,*.zoo,*.hqx,*.sit,*.ice,*.uc2,*.ain,*.imp,*.777,*.ufa,*.boa,'
        db      '*.bs[2a],*.sea,*.hpk,*.ddi,*.x2,*.rkv,*.[lw]sz,*.h[ay]p,*.lim,*.sqz,*.chz',0
highlight_group7:
        db      0
        db      16h, 36h, 0, 0
        db      '*.bak,*.tmp',0
highlight_group8:
        db      0
        db      17h, 37h, 0, 0
        db      '*.asm,*.inc',0
highlight_group9:
        db      10h
        db      1Fh, 3Fh, 0, 0
        db      '*',0

bConfirmDelete  db      1
bConfirmDeleteIncomplete db 0

FolderShortcuts dd      0,0,0,0,0,0,0,0,0,0

; Здесь заканчиваются конфигурационные данные

bWasE0          db      0
ctrlstate       db      0
MemForImage     dd      0
restore_semicolon dd    0
bForHex         db      0

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
        rb      8
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
        dd      ?
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
        dd      ?, ?
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

; диалог выделения/снятия
mark_dlgdata:
        dd      1
        dd      -1, -1
        dd      37, 1
        dd      4, 2
.title  dd      ?
        dd      ?, ?
        dd      0, 0
        dd      1
; поле редактирования
        dd      3
        dd      1, 0, 35, 0
        dd      enter_string_buf
.flags  dd      ?

; диалог быстрого поиска в панели (Alt+буквы)
QuickSearchDlg:
        dd      5
.x      dd      ?
.y      dd      ?
        dd      20, 1
        dd      1, 1
        dd      aSearch
        dd      ?, ?
        dd      0, 0
        dd      1
; поле редактирования
        dd      3
        dd      1, 0, 18, 0
        dd      quick_search_buf
        dd      1Ch

; диалог поиска в файле для просмотрщика и редактора
find_in_file_dlgdata:
        dd      1
.x      dd      -1
.y      dd      -1
.width  dd      ?
.height dd      7
        dd      4, 2
        dd      aSearch
        dd      ?, ?
        dd      0, 0
        dd      8
; Строка "Искать"
        dd      1
        dd      1,0,aSearchForLen,0
        dd      aSearchFor
        dd      0
; поле редактирования с текстом для поиска
        dd      3
        dd      1,1
.width2 dd      ?
        dd      1
        dd      SearchStringEditBuf
.flags0 dd      0xC
; горизонтальный разделитель
        dd      4
        dd      -1,2
        dd      -1,2
        dd      0
        dd      0
; флажок "Учитывать регистр"
        dd      5
        dd      1,3
        dd      -1,3
        dd      aCaseSensitive
.flags_case dd  18h     ; default: search is case sensitive
; флажок "Только целые слова"
        dd      5
        dd      1,4
        dd      -1,4
        dd      aWholeWords
.flags_whole dd 8       ; default: do NOT search whole words only
; горизонтальный разделитель
        dd      4
        dd      -1,5
        dd      -1,5
        dd      0
        dd      0
; кнопка "Искать"
.search_btn:
        dd      2
.search_x1 dd   ?
        dd      6
.search_x2 dd   ?
        dd      6
        dd      aSearchB
.flags1 dd      18h
; кнопка "отменить"
        dd      2
.cnl_x1 dd      ?
        dd      6
.cnl_x2 dd      ?
        dd      6
        dd      aCancelB
.flags2 dd      8

; первый диалог поиска в файлах (запрос)
filesearch_query_template:
        dd      1
.x      dd      ?
.y      dd      ?
.width  dd      ?
.height dd      9
        dd      4, 2
        dd      aFileSearch
        dd      ?
        dd      0
        dd      0, 0
        dd      10
; строка-приглашение для ввода маски
        dd      1
        dd      1,0,aFileMasksLen,0
        dd      aFileMasks
        dd      0
; поле ввода для маски
        dd      3
        dd      1, 1
.width2 dd      ?
        dd      1
.editptr1 dd    ?
        dd      0xC
; строка-приглашение для текста поиска
        dd      1
        dd      1,2,aContainingTextLen,2
        dd      aContainingText
        dd      0
; поле ввода для текста поиска
        dd      3
        dd      1, 3
.width3 dd      ?
        dd      3
.editptr2 dd    ?
        dd      8
; горизонтальный разделитель
        dd      4
        dd      -1,4
        dd      -1,4
        dd      0
        dd      0
; флажок "Учитывать регистр"
        dd      5
        dd      1,5
        dd      -1,5
        dd      aCaseSensitive
.flags_case dd  ?       ; will be initialized from find_in_file_dlgdata
; флажок "Только целые слова"
        dd      5
        dd      1,6
        dd      -1,6
        dd      aWholeWords
.flags_whole dd ?       ; will be initialized from find_in_file_dlgdata
; горизонтальный разделитель
        dd      4
        dd      -1,7
        dd      -1,7
        dd      0
        dd      0
; кнопка "Искать"
.search_btn:
        dd      2
.search_x1 dd   ?
        dd      8
.search_x2 dd   ?
        dd      8
        dd      aSearchB
        dd      18h
; кнопка "отменить"
        dd      2
.cnl_x1 dd      ?
        dd      8
.cnl_x2 dd      ?
        dd      8
        dd      aCancelB
        dd      8
.size = $ - filesearch_query_template

; второй диалог поиска в файлах (сканирование)
filesearch_search_template:
        dd      1
.x      dd      ?
.y      dd      ?
.width  dd      ?
.height dd      ?
        dd      4, 2
.caption dd     ?
        dd      ?
        dd      0
        dd      0, 0
        dd      9
; список найденных файлов
        dd      6
        dd      0, 0
.width1 dd      ?
.height1 dd     ?
.data1  dd      ?
        dd      0
; горизонтальный разделитель
        dd      4
        dd      -1
.y2     dd      ?
        dd      -1
        dd      ?
        dd      0, 0
; строка "Поиск <string> в:" либо "Поиск закончен..."
        dd      1
        dd      1
.y3     dd      ?
.width3 dd      ?
        dd      ?
.data3  dd      ?
        dd      0
; строка с текущей папкой
        dd      1
.x4     dd      ?
.y4     dd      ?
.width4 dd      ?
        dd      ?
.data4  dd      ?
        dd      0
; горизонтальный разделитель
        dd      4
        dd      -1
.y5     dd      ?
        dd      -1
        dd      ?
        dd      0, 0
; кнопки
.btn1:
        dd      2
.btn1x1 dd      ?
.btn1y  dd      ?
.btn1x2 dd      ?
        dd      ?
        dd      aNewSearch
        dd      8
.btn2:
        dd      2
.btn2x1 dd      ?
.btn2y  dd      ?
.btn2x2 dd      ?
        dd      ?
        dd      aGoto
        dd      1Ch
.btn3:
        dd      2
.btn3x1 dd      ?
.btn3y  dd      ?
.btn3x2 dd      ?
        dd      ?
        dd      aView
        dd      8
.btn4:
        dd      2
.btn4x1 dd      ?
.btn4y  dd      ?
.btn4x2 dd      ?
        dd      ?
        dd      aCancelB2
        dd      8
.size = $ - filesearch_search_template

RetryOrCancelBtn:
        dd      aRetry
        dd      a_Cancel
DeleteOrKeepBtn:
        dd      a_Delete
        dd      aKeep
RetryOrIgnoreBtn:
        dd      aRetry
        dd      aIgnore
DeleteOrCancelBtn:
        dd      a_Delete
        dd      a_Cancel
DeleteErrorBtn:
        dd      aRetry
SkipOrCancelBtn:
        dd      aSkip
        dd      aSkipAll
        dd      a_Cancel
ContinueBtn:
        dd      aContinue
EditorExitBtn:
        dd      aSave
        dd      aDontSave
        dd      aContinueEdit
YesOrNoBtn:
        dd      aYes
        dd      aNo

aCannotOpenFile_ptr     dd      aCannotOpenFile
aCannotCreateThread_ptr dd      aCannotCreateThread
ConfirmCancelMsg        dd      aConfirmCancel
EditConfigErr_ptr:
        dd      aEditConfigErr1
        dd      aEditConfigErr2
if lang eq ru
aDeleteCaption          db      'Удаление',0
aConfirmDeleteText      db      'Вы хотите удалить',0
aDeleteFolder           db      ' папку',0
aConfirmDeleteTextMax = $ - aConfirmDeleteText - 2
aDeleteFile             db      ' файл',0
aCancelB                db      '[ Отменить ]',0
aCancelBLength = $ - aCancelB - 1
aCancelB2               db      '[ Отмена ]',0
aCancelB2Length = $ - aCancelB2 - 1
aCopyCaption            db      'Копирование',0
aCopy                   db      '[ Копировать ]',0
aCopyLength = $ - aCopy - 1
a_Continue              db      '[ Продолжить ]',0
a_ContinueLength = $ - a_Continue - 1
aCopy1                  db      'Копировать ',0
aCopy2                  db      ' в:',0
aError                  db      'Ошибка',0
aContinue               db      'Продолжить',0
aRetry                  db      'Повторить',0
a_Cancel                db      'Отменить',0
a_Delete                db      'Удалить',0
aSkip                   db      'Пропустить',0
aSkipAll                db      'Пропустить все',0
aYes                    db      'Да',0
aNo                     db      'Нет',0
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
aNotFolder              db      'не является папкой',0
aIgnore                 db      'Игнорировать',0
aMkDirCaption           db      'Создание папки',0
aMkDir                  db      'Создать папку',0
aMkDirLen = $ - aMkDir - 1
aCannotMakeFolder       db      'Не могу создать папку',0
aName                   db      3,'Имя'
aSize                   db      6,'Размер'
aDate                   db      4,'Дата'
aTime                   db      5,'Время'
aCannotLoadDLL          db      'Не могу загрузить DLL',0
aCannotLoadPlugin       db      'Не могу загрузить плагин',0
aInvalidDLL             db      'Файл не найден или имеет неверный формат',0
aMissingExport          db      'Необходимая функция не найдена',0
aInitFailed             db      'Ошибка при инициализации',0
aIncompatibleVersion    db      'Несовместимая версия',0
aTables                 db      'Таблицы',0
aSelect                 db      'Пометить',0
aDeselect               db      'Снять',0
aCannotOpenFile         db      'Ошибка при открытии файла',0
aCannotCreateThread     db      'Ошибка при создании потока',0
aCannotSetFolder        db      'Не могу зайти в папку',0
aSearch                 db      'Поиск',0
aSearchB                db      '[ Искать ]',0
aSearchBLength = $ - aSearchB - 1
aSearchFor              db      'Искать',0
aSearchForLen = $ - aSearchFor - 1
aCaseSensitive          db      'Учитывать регистр',0
aWholeWords             db      'Только целые слова',0
aReverseSearch          db      'Обратный поиск',0
aStringNotFound         db      'Строка не найдена',0
aFileSearch             db      'Поиск файла',0
aFileMasks              db      'Одна или несколько масок файлов:',0
aFileMasksLen = $ - aFileMasks - 1
aContainingText         db      'Содержащих текст:',0
aContainingTextLen = $ - aContainingText - 1
aSearchingIn            db      'Поиск "" в:',0
aSearchingInLen = $ - aSearchingIn - 1
aSearchDone             db      'Поиск закончен. Найдено ? файл(ов)',0
aSearchDoneLen = $ - aSearchDone - 1
aNewSearch              db      '[ Новый поиск ]',0
aNewSearchLen = $ - aNewSearch - 1
aGoto                   db      '[ Перейти ]',0
aGotoLen = $ - aGoto - 1
aView                   db      '[ Смотреть ]',0
aViewLen = $ - aView - 1
aEditConfigErr1         db      'Ошибка в конфигурации плагинов для редактора.',0
aEditConfigErr2         db      'Попробуйте убрать лишние плагины.',0
aEditNoMemory           db      'Файл слишком велик для загрузки в редактор.',0
aLine                   db      '  Строка'
aCol                    db      '   Кол '
aEditorTitle            db      'Редактор',0
aFileModified           db      'Файл был изменён',0
aSave                   db      'Сохранить',0
aDontSave               db      'Не сохранять',0
aContinueEdit           db      'Продолжить редактирование',0
aCannotSaveToPlugin     db      'Сохранение файлов на панелях плагинов не поддерживается',0
aCannotSearchOnPlugin   db      'Поиск на панелях плагинов не поддерживается',0
aCancelled              db      'Действие было прервано',0
aConfirmCancel          db      'Вы действительно хотите отменить действие?',0
else
aDeleteCaption          db      'Delete',0
aConfirmDeleteText      db      'Do you wish to delete',0
aDeleteFolder           db      ' the folder',0
aConfirmDeleteTextMax = $ - aConfirmDeleteText - 2
aDeleteFile             db      ' the file',0
aDelete                 db      ' Delete ',0
aDeleteLength = $ - aDelete - 1
aCancel                 db      ' Cancel ',0
aCancelLength = $ - aCancel - 1
aCancelB                db      '[ Cancel ]',0
aCancelBLength = $ - aCancelB - 1
aCancelB2 = aCancelB
aCancelB2Length = $ - aCancelB2 - 1
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
aSkip                   db      'Skip',0
aSkipAll                db      'Skip all',0
aYes                    db      'Yes',0
aNo                     db      'No',0
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
aCannotWriteFile        db      'Cannot write to the file',0
aCannotDeleteFile       db      'Cannot delete the file',0
aCannotDeleteFolder     db      'Cannot delete the folder',0
aNotFolder              db      'is not a folder',0
aIgnore                 db      'Ignore',0
aMkDirCaption           db      'Make folder',0
aMkDir                  db      'Create the folder',0
aMkDirLen = $ - aMkDir - 1
aCannotMakeFolder       db      'Cannot create folder',0
aName                   db      4,'Name'
aSize                   db      4,'Size'
aDate                   db      4,'Date'
aTime                   db      4,'Time'
aCannotLoadDLL          db      'Cannot load DLL',0
aCannotLoadPlugin       db      'Cannot load plugin',0
aInvalidDLL             db      'File is not found or invalid',0
aMissingExport          db      'Required function is not present',0
aInitFailed             db      'Initialization failed',0
aIncompatibleVersion    db      'Incompatible version',0
aTables                 db      'Tables',0
aSelect                 db      'Select',0
aDeselect               db      'Deselect',0
aCannotOpenFile         db      'Cannot open the file',0
aCannotCreateThread     db      'Cannot create a thread',0
aCannotSetFolder        db      'Cannot enter to the folder',0
aSearch                 db      'Search',0
aSearchB                db      '[ Search ]',0
aSearchBLength = $ - aSearchB - 1
aSearchFor              db      'Search for',0
aSearchForLen = $ - aSearch - 1
aCaseSensitive          db      'Case sensitive',0
aWholeWords             db      'Whole words',0
aReverseSearch          db      'Reverse search',0
aStringNotFound         db      'Could not find the string',0
aFileSearch             db      'Find file',0
aFileMasks              db      'A file mask or several file masks:',0
aFileMasksLen = $ - aFileMasks - 1
aContainingText         db      'Containing text:',0
aContainingTextLen = $ - aContainingText - 1
aSearchingIn            db      'Searching "" in:',0
aSearchingInLen = $ - aSearchingIn - 1
aSearchDone             db      'Search done. Found ? file(s)',0
aSearchDoneLen = $ - aSearchDone - 1
aNewSearch              db      '[ New search ]',0
aNewSearchLen = $ - aNewSearch - 1
aGoto                   db      '[ Go to ]',0
aGotoLen = $ - aGoto - 1
aView                   db      '[ View ]',0
aViewLen = $ - aView - 1
aEditConfigErr1         db      'Error in configuration of plugins for the editor.',0
aEditConfigErr2         db      'Try to remove unnecessary plugins.',0
aEditNoMemory           db      'The file is too big for the editor.',0
aLine                   db      '    Line'
aCol                    db      '   Col '
aEditorTitle            db      'Editor',0
aFileModified           db      'File has been modified',0
aSave                   db      'Save',0
aDontSave               db      'Do not save',0
aContinueEdit           db      'Continue editing',0
aCannotSaveToPlugin     db      'Saving is not supported for plugin panels',0
aCannotSearchOnPlugin   db      'The search on plugin panels is not supported yet',0
aCancelled              db      'Operation has been interrupted',0
aConfirmCancel          db      'Do you really want to cancel it?',0
end if

aOk                     db      'OK',0
aNoMemory               db      'No memory!'
nullstr                 db      0
aUntitled               db      'untitled',0
aDotDot                 db      '..',0,0
standard_dll_path:
libini_name             db      '/sys/lib/'
standard_dll_path_size = $ - standard_dll_path
                        db      'libini.obj',0
aStart                  db      'START',0
aLibInit                db      'lib_init',0
aVersion                db      'version',0
aIniGetInt              db      'ini.get_int',0
aIniGetStr              db      'ini.get_str',0
aIniSetInt              db      'ini.set_int',0
aIniEnumKeys            db      'ini.enum_keys',0
aPluginLoad             db      'plugin_load',0
aPluginUnload           db      'plugin_unload',0
aGetattr                db      'getattr',0
aOpen                   db      'open',0
aRead                   db      'read',0
aSetpos                 db      'setpos',0
aClose                  db      'close',0
aOpenFilePlugin         db      'OpenFilePlugin',0
aClosePlugin            db      'ClosePlugin',0
aReadFolder             db      'ReadFolder',0
aSetFolder              db      'SetFolder',0
aGetOpenPluginInfo      db      'GetOpenPluginInfo',0
aGetPanelTitle          db      'GetPanelTitle',0
aGetFiles               db      'GetFiles',0
aEditInfoSize           db      'EditInfoSize',0

aConfirmations          db      'Confirmations',0
aConfirmDelete          db      'Delete',0
aConfirmDeleteIncomplete db     'DeleteIncomplete',0

aPanels                 db      'Panels',0
aLeftViewMode           db      'LeftViewMode',0
aRightViewMode          db      'RightViewMode',0
aLeftSortMode           db      'LeftSortMode',0
aRightSortMode          db      'RightSortMode',0

aEditor                 db      'Editor',0
aEolStyle               db      'EOLStyle',0

aAssociations           db      'Associations',0
aPlugins                db      'Plugins',0
aMenu                   db      'Menu',0
aFolderShortcuts        db      'FolderShortcuts',0
aShortcut               db      'Shortcut'
.d                      db      '0',0

align 4
ini_import:
ini.get_int     dd      aIniGetInt
ini.get_str     dd      aIniGetStr
ini.enum_keys   dd      aIniEnumKeys
ini.set_int     dd      aIniSetInt
                dd      0

plugin_exported:
        dd      aPluginUnload
        dd      aGetattr
        dd      aOpen
        dd      aRead
        dd      aSetpos
        dd      aClose
        dd      aOpenFilePlugin
        dd      aClosePlugin
        dd      aReadFolder
        dd      aSetFolder
        dd      aGetOpenPluginInfo
        dd      aGetPanelTitle
        dd      aGetFiles
        dd      aEditInfoSize
        dd      0
plugin_exported_default:
        dd      plugin_unload_default
        dd      getattr_default
        dd      open_default
        dd      read
        dd      setpos_default
        dd      close
        dd      OpenFilePlugin_default
        dd      ClosePlugin_default
        dd      ReadFolder_default
        dd      SetFolder_default
        dd      GetOpenPluginInfo_default
        dd      GetPanelTitle_default
        dd      GetFiles_default
        dd      0       ; default value for EditInfoSize

kfar_info:
        dd      .size
        dd      version_dword
        dd      open
        dd      open2
        dd      read
        dd      -1      ; write: to be implemented
        dd      seek
        dd      tell
        dd      -1      ; flush: to be implemented
        dd      filesize
        dd      close
        dd      xpgalloc
        dd      xpgrealloc
        dd      pgfree
        dd      getfreemem
        dd      libini_alloc
        dd      libini_realloc
        dd      libini_free
        dd      menu
        dd      menu_centered_in
        dd      DialogBox
        dd      SayErr
        dd      Message
        dd      cur_width
.size = $ - kfar_info

plugins         dd      0
num_plugins     dd      0
alloc_plugins   dd      0

EditPlugInfo    dd      0
EditEOLStyle    db      edit.eol_unix

virtual at 0
PluginInfo:
.unload         dd      ?
.getattr        dd      ?
.open           dd      ?
.read           dd      ?
.setpos         dd      ?
.close          dd      ?
.OpenFilePlugin dd      ?
.ClosePlugin    dd      ?
.ReadFolder     dd      ?
.SetFolder      dd      ?
.GetOpenPluginInfo dd   ?
.GetPanelTitle  dd      ?
.GetFiles       dd      ?
.EditInfoSize   dd      ?
.EditInfoOffs   dd      ?
.size = $
end virtual

virtual at 0
PanelMode:
; up to 16 columns on one panel
.NumColumns     dd      ?
; available column types:
COLUMN_TYPE_NONE = 0
COLUMN_TYPE_NAME = 1
        COLUMN_NAME_MARK = 10h          ; (reserved)
        COLUMN_NAME_NOPATH = 20h        ; (reserved)
        COLUMN_NAME_RIGHTALIGN = 40h    ; (reserved)
COLUMN_TYPE_SIZE = 2
        COLUMN_SIZE_COMMA = 10h         ; (reserved)
COLUMN_TYPE_PACKED_SIZE = 3             ; (reserved)
COLUMN_TYPE_DATE = 4
COLUMN_TYPE_TIME = 5
COLUMN_TYPE_DATETIME = 6                ; (reserved)
COLUMN_TYPE_DATETIME_CREATION = 7       ; (reserved)
COLUMN_TYPE_DATETIME_ACCESS = 8         ; (reserved)
COLUMN_TYPE_ATTRIBUTES = 9              ; (reserved)
COLUMN_TYPE_DESCRIPTION = 10            ; (reserved)
COLUMN_TYPE_OWNER = 11                  ; (reserved)
COLUMN_TYPE_NUMLINKS = 12               ; (reserved)
COLUMN_TYPE_CUSTOM = 13                 ; (reserved)
.ColumnTypes    rb      16

.ColumnWidths   rb      16
.bFullScreen    db      ?               ; (reserved)
.bAlignExtensions db    ?
                rb      2
.size = $
end virtual

align 4
column_headers:
        dd      nullstr
        dd      aName
        dd      aSize
        dd      nullstr
        dd      aDate
        dd      aTime
draw_column_proc:
        dd      draw_empty_column
        dd      draw_name_column
        dd      draw_size_column
        dd      -1
        dd      draw_date_column
        dd      draw_time_column
colmodes:
; режим 0 : NM,SC,D
        dd      3
        db      COLUMN_TYPE_NAME+COLUMN_NAME_MARK, COLUMN_TYPE_SIZE+COLUMN_SIZE_COMMA
                db      COLUMN_TYPE_DATE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        db      0, 10, 8
                times 13 db 0
        db      0, 1
                times 2 db 0
; режим 1 : N,N,N
        dd      3
        db      COLUMN_TYPE_NAME, COLUMN_TYPE_NAME, COLUMN_TYPE_NAME
                times 13 db 0
        times 16 db 0
        db      0, 1
                times 2 db 0
; режим 2 : N,N
        dd      2
        db      COLUMN_TYPE_NAME, COLUMN_TYPE_NAME
                times 14 db 0
        times 16 db 0
        db      0, 0
                times 2 db 0
; режим 3 : N,S,D,T
        dd      4
        db      COLUMN_TYPE_NAME, COLUMN_TYPE_SIZE, COLUMN_TYPE_DATE, COLUMN_TYPE_TIME
                times 12 db 0
        db      0, 6, 8, 5
                times 12 db 0
        db      0, 1
                times 2 db 0
; режим 4 : N,S
        dd      2
        db      COLUMN_TYPE_NAME, COLUMN_TYPE_SIZE
                times 14 db 0
        db      0, 6
                times 14 db 0
        db      0, 0
                times 2 db 0
; режим 5 : N,S,P,DM,DC,DA,A
; режим 6 : N,Z
; режим 7 : N,S,Z
; режим 8 : N,S,O
; режим 9 : N,S,LN

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
panel1_hPlugin  dd      ?
panel1_hFile    dd      ?
panel1_parents          dd      ?
panel1_parents_sz       dd      ?
panel1_parents_alloc    dd      ?
panel1_colmode          dd      ?
panel1_colwidths        rd      16+1
panel1_total_num        dd      ?
panel1_total_size       dq      ?
panel1_selected_num     dd      ?
panel1_selected_size    dq      ?
panel1_plugin_info:
panel1_plugin_flags     dd      ?
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
panel2_hPlugin  dd      ?
panel2_hFile    dd      ?
panel2_parents          dd      ?
panel2_parents_sz       dd      ?
panel2_parents_alloc    dd      ?
panel2_colmode          dd      ?
panel2_colwidths        rd      16+1
panel2_total_num        dd      ?
panel2_total_size       dq      ?
panel2_selected_num     dd      ?
panel2_selected_size    dq      ?
panel2_plugin_info:
panel2_plugin_flags     dd      ?
panel2_dir      rb      1024

;console_data    rb      max_width*max_height*2

nomem_dlgsavearea       rb      8 + (12+4)*(3+3)*2

quicksearch_savearea    rb      22*3*2
quicksearch_maxlen = 64
quick_search_buf        rb      12 + quicksearch_maxlen

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
last_column_index dd    ?

scrpos          dq      ?
viewer_right_side dq    ?

EditDataSize    dd      ?
EditBlockStart  dd      ?
EditBlockSize   dd      ?

saved_file_name:
procinfo        rb      1024
lower_file_name = procinfo + 512

app_path        rb      1100

error_msg       rb      128

prev_dir        rb      1024

driveinfo       rb      32+304
tmpname         rb      32

screens         dd      ?
num_screens     dd      ?
active_screen_vtable dd ?
active_screen_data dd   ?
active_screen_keybar dd ?

default_attr    dd      ?
left_dotdot_entry       rb      40+4    ; 40 bytes for attributes + '..'
right_dotdot_entry      rb      40+4

aConfirmDeleteTextBuf   rb      aConfirmDeleteTextMax + 1
CopySourceTextBuf       rb      512
CopyDestEditBuf         rb      12+512+1
.length = $ - CopyDestEditBuf - 13

SearchStringEditBuf     rb      12
SearchString            rb      253+1
SearchStringEditBuf.length = $ - SearchString - 1
                        db      ?       ; used for output (string -> "string")

enter_string_buf        rb      12+512+1

bMemForImageValidData   db      ?

align 4
identical_table rb      256
tolower_table   rb      256
isspace_table   rb      256
composite_table rb      256
layout          rb      128

copy_buffer_size = 65536
copy_buffer     rb      copy_buffer_size

filedata_buffer_size = 1024
filedata_buffer rb      filedata_buffer_size

source_hModule  dd      ?
source_hPlugin  dd      ?
source_hFile    dd      ?
; data for directory delete
; If directory nested level is >1024, then its full name is too big,
; so we see the overflow when creating full name (we check for this!)
del_dir_stack   rd      1024
del_dir_stack_ptr dd    ?
del_dir_query_size = 32
del_dir_query_area rb   32+304*del_dir_query_size

label copy_dir_stack dword at del_dir_stack
label copy_dir_stack_ptr dword at del_dir_stack_ptr
copy_dir_query_size = del_dir_query_size
copy_dir_query_area = del_dir_query_area
del_bSkipAll    db      ?       ; for directory errors
label copy_bSkipAll byte at del_bSkipAll
copy_bSkipAll2  db      ?       ; for file read/write errors
copy_bSkipAll3  db      ?       ; for SetFolder errors

bEndSlash       db      ?
bDestIsFolder   db      ?
bNeedRestoreName db     ?

; stack
        align   4
        rb      32768
stacktop:

mem:
