draw_menu_bar:
    mov   ecx,[skin_high]
    shl   ecx,16
    mov   cx,19
    mov   ebx,[window_width]
    sub   ebx,9
    add   ebx,5 shl 16
    mcall 13,,,0xcccccc
    add   ecx,37 shl 16
    mov   cx,3
    mcall
    push  ecx
    add   ecx,16 shl 16
    mcall
    add   ecx,16 shl 16
    mov   cx,1
    mcall
    pop   ecx
    add   ecx,3 shl 16
    mov   eax,[window_high]
    shr   ecx,16
    sub   eax,ecx
    sub   eax,4
    shl   ecx,16
    add   ecx,eax
    mov   bx,2
    sub   ecx,33
    mcall 13
    push  ebx
    ror   ebx,16
    mov   bx,[right_panel_x.start_x]
    sub   bx,2
    rol   ebx,16
    mcall
    pop   ebx
    mov   ax,cx
    dec   ax
    shl   eax,16
    add   ecx,eax
    mov   cx,3
    mov   eax,[window_width] 
    mov   bx,ax
    sub   ebx,9
    mcall 13
    add   ecx,15 shl 16
    mcall
    ret