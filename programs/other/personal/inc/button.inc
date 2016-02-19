struct new_button
    x       dw ?
    y       dw ?
    w       dw ?
    h       dw ?
    f       dw ?
    face    dd ?
    frame   dd ?
    event   dd ?
ends

button_event_next:
    pusha
    mov    eax,[edit_cnt]
    cmp    eax,4
    je     @f
    inc    eax
    jmp    ben_exit
  @@:
    mov    eax,1
  ben_exit:
    mov    [edit_cnt],eax
    call    draw_edit
    call    list_draw
    popa
    ret

button_event_back:
    pusha
    mov    eax,[edit_cnt]
    cmp    eax,1
    je     @f
    dec    eax
    jmp    ben_exit
  @@:
    mov    eax,4
  beb_exit:
    mov    [edit_cnt],eax
    call    draw_edit
    call    list_draw
    popa
    ret


button_init:
    ;;  nex init ------------
    mov     ax, 36
    mov     [bnext.x],ax
    mov     ax, 8
    mov     [bnext.y],ax
    mov     ax, 26
    mov     [bnext.w],ax
    mov     ax, 19
    mov     [bnext.h],ax
    mov     ax, 0
    mov     [bnext.f],ax
    mov     eax,[sc.btn_face]
    mov     [bnext.face],eax
    mov     eax,[sc.btn_frame]
    mov     [bnext.frame],eax
    mov     eax,button_event_next
    mov     [bnext.event],eax

    mov     ax, 7
    mov     [bback.x],ax
    mov     ax, 8
    mov     [bback.y],ax
    mov     ax, 26
    mov     [bback.w],ax
    mov     ax, 19
    mov     [bback.h],ax
    mov     ax, 0
    mov     [bback.f],ax
    mov     eax,[sc.btn_face]
    mov     [bback.face],eax
    mov     eax,[sc.btn_frame]
    mov     [bback.frame],eax
    mov     eax,button_event_back
    mov     [bback.event],eax
    ret

button_calc:
    mov     eax, bnext
    call    button_focus
    mov     eax, bback
    call    button_focus
    ret

button_focus:
    pusha
    xor     ecx,ecx
    xor     ebx,ebx
    mov     edi,[mouse_x]
    mov     bx, word [eax]
    cmp     ebx,edi
    jg      button_focus_out
    add     bx, word [eax + 4]
    cmp     ebx,edi
    jb      button_focus_out
    mov     edi,[mouse_y]
    mov     bx,[eax + 2]
    cmp     ebx,edi
    jg      button_focus_out
    add     bx,[eax + 6]
    cmp     ebx,edi
    jb      button_focus_out
    ;----
    push    eax
    call    mouse_left
    mov     ecx,[mouse_l]
    cmp     ecx,0
    je      button_focus_set
    xor     ecx,ecx
    mov     [mouse_l],ecx
    call    dword [eax+18]
    
  button_focus_set:
    pop     eax
    ;----
    mov     ecx,1
  button_focus_out:
    mov     word [eax + 8],cx
    popa
    ret

button_draw:
    mov     eax, bnext
    call    button_draw_all
    mov     eax, bback
    call    button_draw_all
    ; arrow
    pusha
    mov	ecx,[sc.btn_text]
    or	ecx,0x90000000
    mcall	4,<15,11>, ,arrowa
    mcall	 ,<45,11>, ,arrowb
    popa
    ret

button_draw_all:
    pusha
    mov     ebx,[win_slot]
    cmp     ebx,1
    je      @f
    mov     ebx,[sc.btn_inface]
    mov     dword [eax + 10],ebx
    mov     ebx,[sc.btn_inframe]
    mov     dword [eax + 14],ebx
    jmp     button_draw_run
  @@: ;act
    xor     ebx,ebx
    mov     bx,word [eax + 8]
    cmp     ebx,1
    je      @f
    mov     ebx,[sc.btn_face]
    mov     dword [eax + 10],ebx
    mov     ebx,[sc.btn_frame]
    mov     dword [eax + 14],ebx
    jmp     button_draw_run
  @@: ; foc
    mov     ebx,[sc.btn_fcface]
    mov     dword [eax + 10],ebx
    mov     ebx,[sc.btn_fcframe]
    mov     dword [eax + 14],ebx

  button_draw_run:

    ; пр€моугольный фон
    mov     bx, word [eax]         ; x
    add     bx, 2                  ; x+2
    shl     ebx,16
    mov     bx, word [eax+4]       ; w
    sub     bx, 4                  ; w-4
    mov     cx, word [eax+2]       ; y
    add     cx,2                   ; y+2
    shl     ecx,16
    mov     cx, word [eax+6]       ; h
    sub     cx, 4                  ; h-4
    mov     edx, dword [eax + 10]  ; color
    push    eax
    mov     eax, 13                ; rect
    int     0x40                   ; draw
    pop     eax

    ; верхн€€ лини€ [frame]
    mov     edx, dword [eax + 14]  ; color
    mov     bx, word [eax]         ; x
    inc     bx                     ; x+1
    shl     ebx,16
    mov     bx, word [eax]         ; x2=x
    add     bx, word [eax + 4]     ; x2=x+w
    sub     bx,2                   ; x2=x2-2
    mov     cx, word [eax + 2]     ; y
    shl     ecx,16
    mov     cx, word [eax + 2]     ; y2=y
    push    eax
    mov     eax, 38                ; line
    int     0x40                   ; draw
    ; верхн€€ лини€ [3d]
    mov     edx, [sc.3d_light]     ; color
    add     ecx, 1 shl 16 + 1      ; y=y+1 y2=y2+1
    int     0x40                   ; draw
    pop     eax

    ; нижн€€ лини€ [frame]

    mov     cx, word [eax + 2]     ; y
    add     cx, word [eax + 6]     ; y = y+h
    push    cx
    shl     ecx,16
    pop     cx
    sub     ecx, 1 shl 16 + 1      ; y=y+1 y2=y2+1
    mov     edx, dword [eax + 14]  ; color
    push    eax
    mov     eax, 38                ; line
    int     0x40                   ; draw
    ; нижн€€ лини€ [3d]
    sub     ebx, 1 shl 16          ; x = x-1
    mov     edx, [sc.3d_dark]      ; color
    sub     ecx, 1 shl 16 + 1      ; y=y+1 y2=y2+1
    int     0x40                   ; draw
    pop     eax


    ; передн€€ бокова€ [frame]
    mov     edx, dword [eax + 14]  ; color
    mov     bx, word [eax]         ; x
    shl     ebx,16
    mov     bx, word [eax]         ; x2=x
    mov     cx, word [eax + 2]     ; y
    inc     cx                     ; y=y+1
    shl     ecx,16
    mov     cx, word [eax + 2]     ; y2=y
    add     cx, word [eax + 6]     ; y2=y+h
    sub     cx,2                   ; y2=y2-2
    push    eax
    mov     eax, 38                ; line
    int     0x40                   ; draw
    ; передн€€ бокова€ [3d]
    mov     edx, [sc.3d_light]     ; color
    add     ebx, 1 shl 16 + 1      ; x=x+1 x2=x2+1
    sub     ecx,1                  ; y2=y2-1
    int     0x40                   ; draw
    pop     eax

    ; задн€€ бокова€ [frame]
    mov     edx, dword [eax + 14]  ; color
    mov     bx, word [eax]         ; x
    add     bx, word [eax + 4]     ; x=x+w
    dec     bx
    push    bx
    shl     ebx,16
    pop     bx
    inc     ecx
    push    eax
    mov     eax, 38                ; line
    int     0x40                   ; draw
    ; задн€€ бокова€ [3d]
    mov     edx, [sc.3d_dark]     ; color
    sub     ebx, 1 shl 16 + 1      ; x=x+1 x2=x2+1
    int     0x40                   ; draw
    pop     eax

    popa
    ret
