align 4
draw_menu_bar:
    pushad
    mov   ecx,[skin_high]
    shl   ecx,16
    mov   cx,19
    mov   ebx,[window_width]
    sub   ebx,9
    add   ebx,5 shl 16
    mcall SF_DRAW_RECT, ,,0xcccccc
    add   ecx,37 shl 16
    mov   cx,FILE_BR_TOP_LINE-67
    mcall ; horizontal line before disk buttons
    push  ecx
    add   ecx,(FILE_BR_TOP_LINE-54) shl 16
    mov   cx,3
    mcall ; horizontal line after disk buttons
    add   ecx,16 shl 16
    mov   cx,1
    mcall ; horizontal line before sort buttons
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
    mcall SF_DRAW_RECT ; left vertical line
    push  ebx
    movzx ebx,[left_panel_x.start_x]
    add   bx,[left_panel_x.size_x]
    inc   bx
    mov   si,bx
    shl   ebx,16
    add   bx,[right_panel_x.start_x]
    sub   bx,si
    push  ecx
    sub   ecx,13
    add   ecx,13 shl 16
    mcall ; middle vertical line
    pop   ecx
    pop   ebx
    mov   ax,cx
    dec   ax
    shl   eax,16
    add   ecx,eax
    mov   cx,3
    mov   eax,[window_width] 
    mov   bx,ax
    sub   ebx,9
    mcall SF_DRAW_RECT ; horizontal line before path to the current folder
    add   ecx,15 shl 16
    mcall ; horizontal line after path to the current folder
    popad
    ret