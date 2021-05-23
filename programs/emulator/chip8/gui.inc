; draw main window
align 4
proc draw_main_window stdcall
        DEBUGF  DBG_INFO, "draw_main_window() start\n"
        mcall   12, 1       ; notify about draw beginning
        mcall   48, 3, sys_colors, sizeof.system_colors
        
        mov     edx, [sys_colors.work]         ; background color
        or      edx, 0x74000000                ; window type
        ; DEBUGF  DBG_INFO, "mainwindow w, h  = %u, %u", GFX_COLS*GFX_PIX_SIZE + 8, GFX_ROWS*GFX_PIX_SIZE + 28
        mcall   0, <50, GFX_COLS*GFX_PIX_SIZE + 9>, <50, GFX_ROWS*GFX_PIX_SIZE + 28>, , , main_window_title ; 

        stdcall draw_screen

        mcall   12, 2                  ; notify about draw ending
        DEBUGF  DBG_INFO, "draw_main_window() end\n"
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
        mov     dword [color], COLOR_CELL ; white
        cmp     byte [esi + gfx], 0 ; check if cell is 0 or not 0
        jne     @f
        mov     dword [color], COLOR_BACK ;  black
    @@:
        mov     ebx, dword [col_ind]
        imul    ebx, GFX_PIX_SIZE ; now ebx - x coord of rect
        mov     ecx, dword [row_ind]
        imul    ecx, GFX_PIX_SIZE ; now ecx - y coord of rect
        mov     edx, dword [color]
        stdcall imgbuf_draw_rect, ebx, ecx, edx

        inc     esi
        jmp     .loop1

.loop1_end:
        stdcall imgbuf_send_to_window 
        popad
        ret
endp

; copy imgbuf contents to the emulator window
align 4
proc imgbuf_send_to_window stdcall
        DEBUGF  DBG_INFO, "sending to window...\n"
        push    eax ebx ecx edx
        mov     eax, 7
        mov     ebx, dword [imgbuf_ptr]
        mov     ecx, IMGBUF_WIDTH
        shl     ecx, 16
        add     ecx, IMGBUF_HEIGHT
        xor     edx, edx
        int     0x40
        pop     edx ecx ebx eax
        ret
endp


; in internal buffer draw rect filled with given color at position (rect_x, rect_y) within window
align 4
proc imgbuf_draw_rect stdcall, rect_x: dword, rect_y: dword, rect_color: dword
        DEBUGF  DBG_INFO, "imgbuf_draw_rect(%u, %u, %x)\n", [rect_x], [rect_y], [rect_color]
        push    eax ebx ecx edx esi edi ebp

        mov     ebx, dword [rect_y]
        imul    ebx, IMGBUF_WIDTH
        add     ebx, dword [rect_x] ; now ebx - index of first pixel of rect

        mov     edi, dword [imgbuf_ptr]
        mov     ebp, dword [rect_color]

        xor     ecx, ecx
.for_i:
        cmp     ecx, GFX_PIX_SIZE
        jae     .ret

        xor     edx, edx
.for_j:
        cmp     edx, GFX_PIX_SIZE
        jae     .for_j_end

        mov     esi, edx

        add     edx, ebx
        mov     eax, ecx
        imul    eax, IMGBUF_WIDTH
        add     edx, eax ; now edx is index of edx'th pixel of ecx'th row of rect

        lea     edx, [edx*3]
        add     edx, edi
        
        mov     dword [edx], ebp ; put color to pixel

        mov     edx, esi
        inc     edx
        jmp     .for_j
.for_j_end:

        inc     ecx
        jmp     .for_i

.ret:
        pop     ebp edi esi edx ecx ebx eax
        ret
endp
