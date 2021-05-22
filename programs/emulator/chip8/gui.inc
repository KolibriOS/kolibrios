; draw main window
align 4
proc draw_main_window stdcall
        mcall   12, 1       ; notify about draw beginning
        mcall   48, 3, sys_colors, sizeof.system_colors
        
        mov     edx, [sys_colors.work]         ; background color
        or      edx, 0x74000000                ; window type
        ; DEBUGF  DBG_INFO, "mainwindow w, h  = %u, %u", GFX_COLS*GFX_PIX_SIZE + 8, GFX_ROWS*GFX_PIX_SIZE + 28
        mcall   0, <50, GFX_COLS*GFX_PIX_SIZE + 8>, <50, GFX_ROWS*GFX_PIX_SIZE + 28>, , , main_window_title ; 

        stdcall draw_screen

        mcall   12, 2                  ; notify about draw ending
        ret
endp

; draw screen
align 4
proc draw_screen stdcall
        pushad
        locals
            row_ind dd ?
            col_ind dd ?
            color   dd ?
        endl

        xor     esi, esi
.loop1:
        cmp     esi, GFX_SIZE
        jae     .loop1_end
        xor     edx, edx
        mov     eax, esi
        mov     ecx, GFX_COLS
        div     ecx ; eax = row index, edx = col index
        mov     dword [row_ind], eax
        mov     dword [col_ind], edx
        mov     dword [color], 0x80FFFFFF ; white
        cmp     byte [esi + gfx], 0 ; check if cell is 0 or not 0
        jne     @f
        mov     dword [color], 0x80000000 ;  black
    @@:
        mov     ebx, dword [col_ind]
        imul    ebx, GFX_PIX_SIZE
        ;add     ebx, WINDOW_BORDER
        shl     ebx, 16
        add     ebx, GFX_PIX_SIZE

        mov     ecx, dword [row_ind]
        imul    ecx, GFX_PIX_SIZE
        ;add     ecx, WINDOW_BORDER
        shl     ecx, 16
        add     ecx, GFX_PIX_SIZE

        mov     eax, 13
        mov     edx, dword [color]
        int     0x40

        inc     esi
        jmp     .loop1

.loop1_end:
        popad
        ret
endp