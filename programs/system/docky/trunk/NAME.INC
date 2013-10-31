 n_main:
    cmp     dword[dock_items.location], 1
    je	    .top
    cmp     dword[dock_items.location], 4
    je	    .right
    cmp     dword[dock_items.location], 3
    je	    .bottom
    jmp     .left
 .top:
    mov     eax, [win.height_opn]
    mov     [nwin.y], eax
    jmp     @f
 .right:
    mov     eax, [win.x_opn]
    sub     eax, 72
    mov     [nwin.x], eax
    jmp     @f
 .bottom:
    mov     eax, [win.y_opn]
    sub     eax, 20
    mov     [nwin.y], eax
    jmp     @f
 .left:
    mov     eax, [win.width_opn]
    add     eax, 4
    mov     [nwin.x], eax
  @@:
    mov     dword[nwin.width], 67
    mov     dword[nwin.height], 16

    mcall   40, 1b

    mcall   9, win.procinfo, -1
    mov     ecx, [win.procinfo + 30]
    mcall   18, 21
    and     eax, 0xFFFF
    mov     [nwin.sid], eax
;-------------------------------------------------------------------------------
n_main_loop:
    mcall   23, 1

    cmp     eax, EV_IDLE
    je	    n_event_idle
    cmp     eax, EV_REDRAW
    je	    n_event_redraw

    jmp     n_main_loop
;-------------------------------------------------------------------------------
n_event_idle:
    cmp     byte[nwin.close], 1
    jne     @f

    mov     byte[nwin.close], 0
    mcall   -1

  @@:
    cmp     byte[win.button_index], 100
    jne     @f

    mcall   67, 0, 0, 0, 0
    jmp     .end

  @@:
    cmp     byte[nwin.change_shape], 1
    jne     .end

    mov     byte[nwin.change_shape], 0
    mcall   67, [nwin.x], [nwin.y], [nwin.width], [nwin.height]

 .end:
    jmp     n_main_loop
;-------------------------------------------------------------------------------
n_event_redraw:
    mcall   12, 1

    mcall   0, <[nwin.x], [nwin.width]>, <[nwin.y], [nwin.height]>, [color.bg], [color.bg], [color.frame]

    dec     dword[nwin.width]
    mcall   13, <1, [nwin.width]>, <0, 1>, [color.bg]
    inc     dword[nwin.width]

    mov     edx, [win.button_index]
    imul    edx, 16
    add     edx, dock_items.name

    mov     eax, 0
  @@:
    inc     eax
    cmp     byte[edx+eax], 0
    jne     @b

    imul    eax, 3
    mov     ebx, 34
    sub     ebx, eax
    inc     ebx
    shl     ebx, 16
    add     ebx, 4

    mcall   4, , [color.text]

    mcall   12, 2

    jmp     n_main_loop