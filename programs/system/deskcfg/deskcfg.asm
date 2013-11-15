    use32
    org     0x0
;-------------------------------------------------------------------------------
    db	    "MENUET01"
    dd	    1, @code, @data, @mem, @stack, 0, 0
;-------------------------------------------------------------------------------
    include "../../macros.inc"
    include "../../proc32.inc"
    include "../../dll.inc"
    ;include "../../debug.inc"
;===============================================================================
@code:
    mcall   9, buffer, -1
    mov     ecx, [buffer + 30]
    mcall   18, 21
    mov     [win.sid], eax
;-------------------------------------------------------------------------------
    mcall   68, 11
    stdcall dll.Load, @import
;-------------------------------------------------------------------------------
    mov     [tabs.index], byte 2
;-------------------------------------------------------------------------------
    mcall   48, 3, color, 40
;-------------------------------------------------------------------------------
    invoke  ini.iget, ini.docky, ini.docky.sect.cfg, ini.docky.keys.fsize, 0
    mov     [docky.fsize], al
;-------------------------------------------------------------------------------
main_loop:
    mcall   10
    cmp     eax, EV_REDRAW
    je	    event_redraw
    cmp     eax, EV_BUTTON
    je	    event_button

    jmp     main_loop
;-------------------------------------------------------------------------------
exit:
    mcall   -1
;-------------------------------------------------------------------------------
event_redraw:
    call    win.draw
    jmp     main_loop
;-------------------------------------------------------------------------------
event_button:
    mcall   17
    cmp     ah, 0x01
    je	    exit
    cmp     ah, 0x10
    je	    .tabs.walls
    cmp     ah, 0x11
    je	    .tabs.skins
    cmp     ah, 0x12
    je	    .tabs.docky
    cmp     ah, 0x13
    je	    .tabs.panel

    cmp     ah, 0x40
    je	    .docky.button_top
    cmp     ah, 0x41
    je	    .docky.button_left
    cmp     ah, 0x42
    je	    .docky.button_bottom
    cmp     ah, 0x43
    je	    .docky.button_right

    cmp     ah, 0x44
    je	    .docky.checkbox_fsize

    cmp     ah, 0x45
    je	    .docky.kill
    cmp     ah, 0x46
    je	    .docky.start

    jmp     main_loop

 .tabs.walls:
    mov     [tabs.index], byte 0
    call    win.draw
    jmp     main_loop
 .tabs.skins:
    mov     [tabs.index], byte 1
    call    win.draw
    jmp     main_loop
 .tabs.docky:
    mov     [tabs.index], byte 2
    call    win.draw
    jmp     main_loop
 .tabs.panel:
    mov     [tabs.index], byte 3
    call    win.draw
    jmp     main_loop

 .docky.button_top:
    invoke  ini.iset, ini.docky, ini.docky.sect.cfg, ini.docky.keys.location, 1
    call    docky.apply
    jmp     main_loop
 .docky.button_left:
    invoke  ini.iset, ini.docky, ini.docky.sect.cfg, ini.docky.keys.location, 2
    call    docky.apply
    jmp     main_loop
 .docky.button_bottom:
    invoke  ini.iset, ini.docky, ini.docky.sect.cfg, ini.docky.keys.location, 3
    call    docky.apply
    jmp     main_loop
 .docky.button_right:
    invoke  ini.iset, ini.docky, ini.docky.sect.cfg, ini.docky.keys.location, 4
    call    docky.apply
    jmp     main_loop

 .docky.checkbox_fsize:
    mov     al, 1
    sub     al, byte [docky.fsize]
    mov     [docky.fsize], al
    push    183
    push    docky.fsize
    call    checkbox.draw

    xor     eax, eax
    mov     al, [docky.fsize]
    invoke  ini.iset, ini.docky, ini.docky.sect.cfg, ini.docky.keys.fsize, eax
    call    docky.apply

    jmp     main_loop

 .docky.kill:
    call    docky.kill
    jmp     main_loop
 .docky.start:
    call    docky.kill
    call    docky.start
    jmp     main_loop
;-------------------------------------------------------------------------------
win.draw:
    mcall   12, 1

    mov     edx, [color.work]
    or	    edx, 0x34000000
    mcall   0, <128, 256>, <128, 299>, , , win.title

    call    tabs.draw

    mcall   12, 2

    ret
;-------------------------------------------------------------------------------
tabs.draw:
    mcall    8, <  8,  57>, < 9, 25>, 0x40000010
    mcall     , < 65,  57>,	    , 0x40000011
    mcall     , <122,  57>,	    , 0x40000012
    mcall     , <179,  58>,	    , 0x40000013

    mcall   13, <  9, 228>,	    , [color.work]

    mcall     , 	  , < 9,  1>, [color.workE]
    mcall     , 	  , <34,  1>
    mcall     , <  8,	1>, <10, 24>
    mcall     , < 65,	1>
    mcall     , <122,	1>
    mcall     , <179,	1>
    mcall     , <237,	1>

    mov     edx, [color.workE]

    cmp     [tabs.index], byte 0
    je	    .walls
    cmp     [tabs.index], byte 1
    je	    .skins
    cmp     [tabs.index], byte 2
    je	    .docky
    jmp     .panel

 .walls:
    mcall     , <  9,  56>

    mcall   4, < 22, 18>, [color.textE], tabs.walls_title, 5
    mcall    , < 79, 18>, [color.text] , tabs.skins_title
    mcall    , <136, 18>,	       , tabs.docky_title
    mcall    , <193, 18>,	       , tabs.panel_title

    call    tabs.walls.draw

    ret
 .skins:
    mcall     , < 66,  56>

    mcall   4, < 79, 18>, [color.textE], tabs.skins_title, 5
    mcall    , < 22, 18>, [color.text] , tabs.walls_title
    mcall    , <136, 18>,	       , tabs.docky_title
    mcall    , <193, 18>,	       , tabs.panel_title

    call    tabs.skins.draw

    ret
 .docky:
    mcall     , <123,  56>

    mcall   4, <136, 18>, [color.textE], tabs.docky_title, 5
    mcall    , < 79, 18>, [color.text] , tabs.skins_title
    mcall    , < 22, 18>,	       , tabs.walls_title
    mcall    , <193, 18>,	       , tabs.panel_title

    call    tabs.docky.draw

    ret
 .panel:
    mcall     , <180,  57>

    mcall   4, <193, 18>, [color.textE], tabs.panel_title, 5
    mcall    , < 79, 18>, [color.text] , tabs.skins_title
    mcall    , <136, 18>,	       , tabs.docky_title
    mcall    , < 22, 18>,	       , tabs.walls_title

    call    tabs.panel.draw

    ret
;-------------------------------------------------------------------------------
tabs.walls.draw:
    ret
;-------------------------------------------------------------------------------
tabs.skins.draw:
    ret
;-------------------------------------------------------------------------------
tabs.docky.draw:
  ; == FRAME: POSITION == ;
    mcall   13, <10, 226>, <48, 112>, [color.workE]
    mcall     , <11, 224>, <49, 110>, [color.textE]
    mcall     , <12, 222>, <50, 108>, [color.work]

    mov     ecx, [color.text]
    or	    ecx, 0xC0000000
    mcall   4, <20, 45>, , tabs.docky.frame_pos_title, , [color.work]

    mcall   8, < 81, 84>, < 64, 24>, 0x40, [color.workE]
    mcall    , < 58, 64>, < 91, 24>, 0x41
    mcall    , < 81, 84>, <118, 24>, 0x42
    mcall    , <125, 64>, < 91, 24>, 0x43

    mov     ecx, [color.textE]
    or	    ecx, 0x80000000
    mcall   4, <115,  73>, , tabs.docky.button_top
    mcall    , < 79, 100>, , tabs.docky.button_left
    mcall    , <106, 127>, , tabs.docky.button_bottom
    mcall    , <143, 100>, , tabs.docky.button_right

  ; == FRAME: SETTINGS == ;
    mcall   13, <10, 226>, <170, 40>, [color.workE]
    mcall     , <11, 224>, <171, 38>, [color.textE]
    mcall     , <12, 222>, <172, 36>, [color.work]

    mov     ecx, [color.text]
    or	    ecx, 0xC0000000
    mcall   4, <20, 167>, , tabs.docky.frame_set_title, , [color.work]

    mcall   8, <20, 206>, <182, 16>, 0x60000044

    mov     ecx, [color.text]
    or	    ecx, 0x80000000
    mcall   4, <20, 187>, , tabs.docky.checkbox_fsize_title

    push    183
    push    docky.fsize
    call    checkbox.draw

  ; == FRAME: THEARD == ;
    mcall   13, <10, 226>, <219, 44>, [color.workE]
    mcall     , <11, 224>, <220, 42>, [color.textE]
    mcall     , <12, 222>, <221, 40>, [color.work]

    mov     ecx, [color.text]
    or	    ecx, 0xC0000000
    mcall   4, <20, 217>, , tabs.docky.frame_theard_title, , [color.work]

    mcall   8, < 20,  98>, <231, 20>, 0x45, [color.workE]
    mcall    , <128,  98>,	    , 0x46

    mov     ecx, [color.textE]
    or	    ecx, 0x80000000
    mcall   4, < 54,  238>, , tabs.docky.button_close
    mcall    , <139,  238>, , tabs.docky.button_start

    ret
;-------------------------------------------------------------------------------
tabs.panel.draw:
    ret
;-------------------------------------------------------------------------------
checkbox.draw:
    pop     ebp
    pop     edi
    pop     ecx
    push    ebp

    shl     ecx, 16
    mov     cx, 16
    mcall   13, <188, 34>, , [color.workE]
    sub     ecx, 2
    add     ecx, 0x00010000
    mcall     , <189, 32>, , [color.textE]

    mov     eax, 13
    mov     edx, [color.workE]
    cmp     [edi], byte 0
    je	    .draw_off
 .draw_on:
    mcall   , <189, 6>
    mov     edi, ecx
    shr     edi, 16
    add     edi, 3
    mcall   4, <189, edi>, [color.text], checkbox.on, 5

    ret

 .draw_off:
    mov     edx, [color.work]
    mcall   , <215, 6>
    mov     edi, ecx
    shr     edi, 16
    add     edi, 3
    mcall   4, <189, edi>, [color.text], checkbox.off, 5
    ret
;-------------------------------------------------------------------------------
docky.kill:
    mov     dl, 0
    mcall   9, buffer, -1
    mov     ecx, eax
 .search:
    mcall   9, buffer
    cmp     [buffer + 10], dword "@doc"
    je	    .kill
    cmp     [buffer + 10], dword "@DOC"
    jne     .continue
 .kill:
    mov     dl, 1
    mcall   18, 2
 .continue:
    dec     ecx
    cmp     ecx, 0
    jne     .search

    ret
;-------------------------------------------------------------------------------
docky.start:
    mov     [buffer +  0], dword 7
    mov     [buffer +  4], dword 0
    mov     [buffer +  8], dword 0
    mov     [buffer + 20], dword "@doc"
    mov     [buffer + 24], word  "ky"
    mov     [buffer + 26], byte  0
    mcall   70, buffer

    mcall   5, 5
    mcall   18, 3, [win.sid]

    ret
;-------------------------------------------------------------------------------
docky.apply:
    call    docky.kill
    cmp     dl, byte 0
    je	    .end
    call    docky.start
 .end:
    ret
;===============================================================================
@import:
    library ini, "libini.obj"
    import  ini, ini.iget, "ini_get_int", ini.iset, "ini_set_int"
;===============================================================================
ini.docky:
    db	    "settings/docky.ini", 0
 .sect.cfg:
    db	    "@", 0
 .keys.fsize:
    db	    "fsize", 0
 .keys.location:
    db	    "location", 0

win.title:
    db	    "Desktop configuration", 0

tabs.walls_title:
    db	    "Walls"
tabs.skins_title:
    db	    "Skins"
tabs.docky_title:
    db	    "Docky"
tabs.panel_title:
    db	    "Panel"

tabs.docky.frame_pos_title:
    db	    " Position ", 0
tabs.docky.button_top:
    db	    "TOP", 0
tabs.docky.button_left:
    db	    "LEFT", 0
tabs.docky.button_bottom:
    db	    "BOTTOM", 0
tabs.docky.button_right:
    db	    "RIGHT", 0

tabs.docky.frame_set_title:
    db	    " Settings ", 0
tabs.docky.checkbox_fsize_title:
    db	    "Full size mode", 0

tabs.docky.frame_theard_title:
    db	    " Theard ", 0
tabs.docky.button_close:
    db	    "Close", 0
tabs.docky.button_start:
    db	    "Start/Restart", 0


checkbox.on:
    db	    "  ON "
checkbox.off:
    db	    " OFF "

@data:
;===============================================================================
    rb	    2048
@stack:
;-------------------------------------------------------------------------------
win.sid:
    rd	    1

docky.fsize:
    rb	    1

tabs.index:
    rb	    1

color:
 .frame:
    rd	    5
 .work:
    rd	    1
 .workE:
    rd	    1
 .textE:
    rd	    1
 .text:
    rd	    2

buffer:
    rb	    1024

@mem: