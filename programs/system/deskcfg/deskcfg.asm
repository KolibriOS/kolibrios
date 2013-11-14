    use32
    org     0x0
;-------------------------------------------------------------------------------
    db	    "MENUET01"
    dd	    1, @code, @data, @mem, @stack, 0, 0
;-------------------------------------------------------------------------------
    include "../../macros.inc"
;===============================================================================
@code:
    mcall   48, 3, color, 40
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
;-------------------------------------------------------------------------------
win.draw:
    mcall   12, 1

    mov     edx, [color.work]
    or	    edx, 0x34000000
    mcall   0, <128, 256>, <128, 360>, , , win.title

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
    mcall   8, <20, 50>, < 50, 20>, 0x20, [color.workE]
    ret
;-------------------------------------------------------------------------------
tabs.skins.draw:
    mcall   8, <40, 40>, < 60, 30>, 0x30, [color.workE]
    mcall    ,	       , < 99, 30>, 0x31
    ret
;-------------------------------------------------------------------------------
tabs.docky.draw:
    mcall   8, <50, 10>, < 50, 10>, 0x40, [color.workE]
    mcall    ,	       , < 70, 10>, 0x41
    mcall    ,	       , < 90, 10>, 0x42
    mcall    ,	       , <110, 10>, 0x43
    ret
;-------------------------------------------------------------------------------
tabs.panel.draw:
    mcall   8, <70, 20>, <70, 20>, 0x20, [color.workE]
    mcall   4, <30, 50>, [color.text], tabs.panel_title
    ret
;===============================================================================
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

@data:
;===============================================================================
    rb	    2048
@stack:
;-------------------------------------------------------------------------------
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

@mem: