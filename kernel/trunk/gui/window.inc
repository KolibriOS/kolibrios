get_titlebar_height: ; edi = window draw_data pointer
        mov     al,[edi+WDATA.fl_wstyle]
        and     al,0x0F
        cmp     al,0x03
        jne     @f
        mov     eax,[_skinh]
        ret
    @@: mov     eax,21
        ret

get_rolledup_height: ; edi = window draw_data pointer
        mov     al,[edi+WDATA.fl_wstyle]
        and     al,0x0F
        cmp     al,0x03
        jne     @f
        mov     eax,[_skinh]
        add     eax,3
        ret
    @@: or      al,al
        jnz     @f
        mov     eax,21
        ret
    @@: mov     eax,21+2
        ret


setwindowdefaults:
        pushad

        xor   eax,eax
        mov   ecx,0xc000
       @@:
        inc   eax
        add   ecx,2
        mov   [ecx+0x000],ax          ; process no
        mov   [ecx+0x400],ax          ; positions in stack
        cmp   ecx,0xc400-2            ; the more high, the more surface
        jnz   @b

        popad
        ret



; eax = cx
; ebx = cy
; ecx = ex
; edx = ey
; идея: перебрать все окна, начиная с самого нижнего,
;       и для попавших в заданную область
;       частей окон вызвать setscreen
align 4
calculatescreen:
        pushad
        pushfd
        cli

        push    edx ecx ebx eax

        mov     esi, 1
        call    setscreen

        mov     ebp, [0x3004]        ; number of processes
        cmp     ebp, 1
        jbe     .finish
        align 4
      .new_wnd:
        movzx   edi, word [0xC400 + esi * 2]
        shl     edi, 5

        cmp     [0x3000+edi+0xa], byte 9
        je      .not_wnd

        add     edi, window_data
        test    [edi+WDATA.fl_wstate], WSTATE_MINIMIZED
        jnz     .not_wnd

        mov     eax, [edi+WDATA.left]
        cmp     eax, [esp+RECT.right]
        ja      .out_of_bounds
        mov     ebx, [edi+WDATA.top]
        cmp     ebx, [esp+RECT.bottom]
        ja      .out_of_bounds
        mov     ecx, [edi+WDATA.width]
        add     ecx, eax
        cmp     ecx, [esp+RECT.left]
        jb      .out_of_bounds
        mov     edx, [edi+WDATA.height]
        add     edx, ebx
        cmp     edx, [esp+RECT.top]
        jb      .out_of_bounds

	cmp     eax, [esp+RECT.left]
        jae     @f
        mov     eax, [esp+RECT.left]
     @@:
	cmp     ebx, [esp+RECT.top]
        jae     @f
        mov     ebx, [esp+RECT.top]
     @@:
	cmp     ecx, [esp+RECT.right]
        jbe     @f
        mov     ecx, [esp+RECT.right]
     @@:
	cmp     edx, [esp+RECT.bottom]
        jbe     @f
        mov     edx, [esp+RECT.bottom]
     @@:

        push    esi
        movzx   esi, word [0xC400 + esi * 2]
        call    setscreen
        pop     esi

      .not_wnd:
      .out_of_bounds:
        inc     esi
        dec     ebp
        jnz     .new_wnd
      .finish:

	pop     eax ebx ecx edx

        popfd
        popad
ret



virtual at esp
  ff_x     dd ?
  ff_y     dd ?
  ff_width dd ?
  ff_xsz   dd ?
  ff_ysz   dd ?
  ff_scale dd ?
end virtual

align 4
; резервирует место под окно заданного процесса
setscreen:
;  eax  x start
;  ebx  y start
;  ecx  x end
;  edx  y end
;  esi  process number
pushad
        mov edi, esi ;;;word [esi*2+0xc400]
        shl   edi, 8
        add   edi, 0x80000+0x80  ; address of random shaped window area
        cmp   [edi], dword 0
        jne   .free_form

        ; get x&y size
        sub   ecx, eax
        sub   edx, ebx
        inc   ecx
        inc   edx

        ; get WinMap start
        mov   edi, [0xFE00] ; screen_sx
        inc   edi
        imul  edi, ebx
        add   edi, eax
        add   edi, WinMapAddress

  .new_y:
        push  ecx ; sx
        push  edx

        mov   edx, esi
        align 4
  .new_x:
        mov   byte [edi], dl
        inc   edi
        dec   ecx
        jnz   .new_x

        pop   edx
        pop   ecx
        add   edi, [0xFE00]
        inc   edi
        sub   edi, ecx
        dec   edx
        jnz   .new_y
 popad
 ret
  .read_byte:
   ;eax - address
   ;esi - slot
        push  eax
        push  ebx
        push  ecx
        push  edx
        mov   edx,eax
        mov   eax,esi
        lea   ebx,[esp+12]
        mov   ecx,1
        call  read_process_memory
        pop   edx
        pop   ecx
        pop   ebx
        pop   eax
        ret  
  .free_form:

        ;  for (y=0; y <= x_size; y++)
        ;      for (x=0; x <= x_size; x++)
        ;          if (shape[coord(x,y,scale)]==1)
        ;             set_pixel(x, y, process_number);

        sub  ecx, eax
        sub  edx, ebx
        inc  ecx
        inc  edx

        push  dword [edi+4]  ; push scale first -> for loop

        ; get WinMap start  -> ebp
        push  eax
        mov   eax, [0xFE00] ; screen_sx
        inc   eax
        imul  eax, ebx
        add   eax, [esp]
        add   eax, WinMapAddress
        mov   ebp, eax

        mov   edi, [edi]
        pop   eax

        ; eax = x_start
        ; ebx = y_start
        ; ecx = x_size
        ; edx = y_size
        ; esi = process_number
        ; edi = &shape
        ;       [scale]
        push edx ecx ; for loop - x,y size

        mov  ecx, esi
        shl  ecx, 5
        mov  edx, [window_data+ecx+WDATA.top]
        push [window_data+ecx+WDATA.width]      ; for loop - width
        mov  ecx, [window_data+ecx+WDATA.left]
        sub  ebx, edx
        sub  eax, ecx
        push ebx eax ; for loop - x,y

        add  [ff_xsz], eax
        add  [ff_ysz], ebx

        mov  ebx, [ff_y]

      .ff_new_y:
        mov  edx, [ff_x]

      .ff_new_x:
        ; -- body --
        mov  ecx, [ff_scale]
        mov  eax, [ff_width]
        inc  eax
        shr  eax, cl
        push ebx edx
        shr  ebx, cl
        shr  edx, cl
        imul eax, ebx
        add  eax, edx
        pop  edx ebx
        add  eax, edi
        call .read_byte
        test al,al
        jz   @f
        mov  eax, esi
        mov  [ebp], al
       @@:
        ; -- end body --
        inc  ebp
        inc  edx
        cmp  edx, [ff_xsz]
        jb   .ff_new_x
        sub  ebp, [ff_xsz]
        add  ebp, [ff_x]
        add  ebp, [0xFE00]  ; screen.x
        inc  ebp
        inc  ebx
        cmp  ebx, [ff_ysz]
        jb   .ff_new_y

        add  esp, 24
popad
ret


display_settings:

;    eax = 0         ; DISPLAY redraw
;          ebx = 0   ; all
;
;    eax = 1         ; BUTTON type
;          ebx = 0   ; flat
;          ebx = 1   ; 3D
;    eax = 2         ; set WINDOW colours
;          ebx = pointer to table
;          ecx = number of bytes define
;    eax = 3         ; get WINDOW colours
;          ebx = pointer to table
;          ecx = number of bytes wanted
;    eax = 4         ; get skin height
;          input  : nothing
;          output : eax = skin height in pixel
;    eax = 5         ; get screen workarea
;          input  : nothing
;          output : eax = [left]*65536+[right]
;                   ebx = [top]*65536+[bottom]
;    eax = 6         ; set screen workarea
;          input  : ecx = [left]*65536+[right]
;                   edx = [top]*65536+[bottom]
;          output : nothing
;    eax = 7         ; get skin margins
;          input  : nothing
;          output : eax = [left]*65536+[right]
;                   ebx = [top]*65536+[bottom]
;    eax = 8         ; set window skin
;          input  : ecx = pointer to file info block
;          output : eax = FS error code


     pushad

     test eax, eax      ; redraw display
     jnz  dspl0
     test ebx, ebx
     jnz  dspl0
     cmp  [windowtypechanged],dword 1
     jne  dspl00
     mov  [windowtypechanged],dword 0
  redraw_screen_direct:
     mov  [dlx],dword 0
     mov  [dly],dword 0
     mov  eax,[0xfe00]
     mov  [dlxe],eax
     mov  eax,[0xfe04]
     mov  [dlye],eax
     mov  eax,window_data
     call redrawscreen
   dspl00:
     popad
     ret
   dspl0:

     cmp  eax,1       ; button type
     jne  dspl1
     and  ebx,1
     cmp  ebx,[buttontype]
     je   dspl9
     mov  [buttontype],ebx
     mov  [windowtypechanged],dword 1
    dspl9:
     popad
     ret
   dspl1:

     cmp  eax,2       ; set common window colours
     jne  no_com_colours
     mov  [windowtypechanged],dword 1
     mov  esi,[0x3010]
     add  esi,0x10
     add  ebx,[esi]
     mov  esi,ebx
     mov  edi,common_colours
     and  ecx,127
     cld
     rep  movsb
     popad
     ret
   no_com_colours:

     cmp  eax,3       ; get common window colours
     jne  no_get_com
     mov  esi,[0x3010]
     add  esi,0x10
     add  ebx,[esi]
     mov  edi,ebx
     mov  esi,common_colours
     and  ecx,127
     cld
     rep  movsb
     popad
     ret
   no_get_com:

     cmp  eax,4       ; get skin height
     jne  no_skin_height
     popad
     mov  eax,[_skinh]
     mov  [esp+36],eax
     ret
   no_skin_height:

        cmp     eax,5       ; get screen workarea
        jne     no_get_workarea
        popad
        mov     eax,[screen_workarea.left-2]
        mov     ax,word[screen_workarea.right]
        mov     [esp+36],eax
        mov     eax,[screen_workarea.top-2]
        mov     ax,word[screen_workarea.bottom]
        mov     [esp+24],eax
        ret
   no_get_workarea:

        cmp     eax,6       ; set screen workarea
        jne     no_set_workarea
        movsx   eax,word[esp+16+2]
        movsx   ebx,word[esp+16]
        cmp     eax,ebx
        jge     .lp1
        or      eax,eax;[0xFE00]
        jl      @f
        mov     [screen_workarea.left],eax
    @@: cmp     ebx,[0xFE00]
        jg      .lp1
        mov     [screen_workarea.right],ebx
  .lp1: movsx   eax,word[esp+24+2]
        movsx   ebx,word[esp+24]
        cmp     eax,ebx
        jge     .lp2
        or      eax,eax;[0xFE04]
        jl      @f
        mov     [screen_workarea.top],eax
    @@: cmp     ebx,[0xFE04]
        jg      .lp2
        mov     [screen_workarea.bottom],ebx
  .lp2: call	repos_windows
        mov     eax, 0
        mov     ebx, 0
        mov     ecx, [0xfe00]
        mov     edx, [0xfe04]
	call	calculatescreen
;	jmp	redraw_screen_direct
    .exit:
        popad
        ret
   no_set_workarea:

        cmp     eax,7       ; get skin margins
        jne     no_get_skinmargins
        popad
        mov     eax,dword[_skinmargins+0]
        mov     [esp+36],eax
        mov     eax,dword[_skinmargins+4]
        mov     [esp+24],eax
        ret
   no_get_skinmargins:

        cmp     eax,8       ; set window skin
        jne     no_set_skin
        mov     eax,ebx
        mov     edi,[0x3010]
        add     ebx,[edi+0x10]        ; abs start of info block
        pushd   [ebx+0] [ebx+4] [ebx+8] [ebx+12]
        mov     dword[ebx+0],0        ; read
        mov     dword[ebx+4],0        ; from the beginning
        mov     dword[ebx+8],64       ; 32 KBytes maximum
        mov     ecx,skin_data+64*512
        sub     ecx,[edi+0x10]
        mov     dword[ebx+12],ecx     ; destination
        push	eax
        pushad
        call    file_system
        popad
        pop	eax
        popd    [ebx+12] [ebx+8] [ebx+4] [ebx+0]
        cmp     eax,ERROR_SUCCESS
        je      @f
        cmp     eax,ERROR_END_OF_FILE
        jne     .exit
    @@: cmp     [skin_data+64*512+SKIN_HEADER.ident],'SKIN'
        mov     eax,ERROR_UNKNOWN_FS
        jne     .exit
        mov     esi,skin_data+64*512
        mov     edi,skin_data
        mov     ecx,(64*512)/4
        rep     movsd
        call    parse_skin_data
	pushad
        mov     eax, 0
        mov     ebx, 0
        mov     ecx, [0xfe00]
        mov     edx, [0xfe04]
        call    calculatescreen
	popad
        mov     dword[esp+32+36],0
        jmp     redraw_screen_direct
  .exit:
        mov     [esp+32+36],eax
        popad
        ret
   no_set_skin:

     popad
     ret


repos_windows:
        mov     ecx,[0x3004]
        mov     esi,0x20*2
	mov	byte[0x0000fff0],1
        dec     ecx
        jge	@f
        ret
    @@: mov	[esi+WDATA.fl_redraw],1
	test    [esi+WDATA.fl_wstate],WSTATE_MAXIMIZED
        jz      .lp2
        mov     eax,[screen_workarea.left]
        mov     [esi+WDATA.left],eax
        sub     eax,[screen_workarea.right]
        neg     eax
        mov     [esi+WDATA.width],eax
        mov     eax,[screen_workarea.top]
        mov     [esi+WDATA.top],eax
        test    [esi+WDATA.fl_wstate],WSTATE_ROLLEDUP
        jnz     .lp1
        sub     eax,[screen_workarea.bottom]
        neg     eax
        mov     [esi+WDATA.height],eax
  .lp1: add     esi,0x20
        loop    @b
        ret
  .lp2: mov	eax,[esi+WDATA.left]
	add	eax,[esi+WDATA.width]
	mov	ebx,[0x0000fe00]
;	inc	ebx
	cmp	eax,ebx
	jle	.lp4
	mov	eax,[esi+WDATA.width]
	sub	eax,ebx
	jle	.lp3
	mov	[esi+WDATA.width],ebx
  .lp3: sub	ebx,[esi+WDATA.width]
	mov	[esi+WDATA.left],ebx
  .lp4: mov	eax,[esi+WDATA.top]
	add	eax,[esi+WDATA.height]
	mov	ebx,[0x0000fe04]
;	inc	ebx
	cmp	eax,ebx
	jle	.lp6
	mov	eax,[esi+WDATA.height]
	sub	eax,ebx
	jle	.lp5
	mov	[esi+WDATA.height],ebx
  .lp5: sub	ebx,[esi+WDATA.height]
	mov	[esi+WDATA.top],ebx
  .lp6: add     esi,0x20
        loop    @b
	ret

uglobal
  common_colours:
     times 128 db 0x0
endg




check_window_position:

    pushad                           ; window inside screen ?

    movzx eax,word [edi+0]
    movzx ebx,word [edi+4]
    movzx ecx,word [edi+8]
    movzx edx,word [edi+12]

    mov   esi,ecx             ; check x pos
    add   esi,eax
    cmp   esi,[0xfe00]
    jbe   x_pos_ok
    mov   [edi+0],dword 0
    xor   eax, eax
  x_pos_ok:

    mov   esi,edx             ; check y pos
    add   esi,ebx
    cmp   esi,[0xfe04]
    jbe   y_pos_ok
    mov   [edi+4],dword 0
    mov   ebx,0
  y_pos_ok:

    mov   esi,ecx             ; check x size
    add   esi,eax
    cmp   esi,[0xfe00]
    jbe   x_size_ok
    mov   ecx,[0xfe00]
    mov   [edi+8],ecx
  x_size_ok:

    mov   esi,edx             ; check y size
    add   esi,ebx
    cmp   esi,[0xfe04]
    jbe   y_size_ok
    mov   edx,[0xfe04]
    mov   [edi+12],edx
  y_size_ok:

    popad

    ret


uglobal
  new_window_starting dd 0
endg


sys_window_mouse:

    push  eax

    mov   eax,[timer_ticks] ;[0xfdf0]
    cmp   [new_window_starting],eax
    jb    swml1

    mov   [0xfff4],byte 0  ; no mouse background
    mov   [0xfff5],byte 0  ; draw mouse

    mov   [new_window_starting],eax

  swml1:

    pop   eax

    ret

drawwindow_I:

        pushad

        mov   esi,[edx+24]   ; rectangle
        mov   eax,[edx+0]
        shl   eax,16
        add   eax,[edx+0]
        add   eax,[edx+8]
        mov   ebx,[edx+04]
        shl   ebx,16
        add   ebx,[edx+4]
        add   ebx,[edx+12]
        call  draw_rectangle

        mov   ecx,[edx+20]   ; grab bar
        push  ecx
        mov   esi,edx
        mov   edx,[esi+04]
        add   edx,1
        mov   ebx,[esi+04]
        add   ebx,25
        mov   eax,[esi+04]
        add   eax,[esi+12]
        cmp   ebx,eax
        jb    wdsizeok
        mov   ebx,eax
      wdsizeok:
        push  ebx
      drwi:
        mov   ebx,edx
        shl   ebx,16
        add   ebx,edx
        mov   eax,[esi+00]
        inc   eax
        shl   eax,16
        add   eax,[esi+00]
        add   eax,[esi+8]
        sub   eax,1
        push  edx
        mov   edx,0x80000000
        mov   ecx,[esi+20]
        and   ecx,edx
        cmp   ecx,edx
        jnz   nofa
        mov   ecx,[esi+20]
        sub   ecx,0x00040404
        mov   [esi+20],ecx
        and   ecx,0x00ffffff
        jmp   faj
      nofa:
        mov   ecx,[esi+20]
        and   ecx,0x00ffffff
      faj:
        pop   edx
        mov   edi,0
        call  [draw_line]
        inc   edx
        cmp   edx,[esp]
        jb    drwi
        add   esp,4
        pop   ecx
        mov   [esi+20],ecx

        mov   edx,[esi+04]      ; inside work area
        add   edx,21+5
        mov   ebx,[esi+04]
        add   ebx,[esi+12]
        cmp   edx,ebx
        jg    noinside
        mov   eax,1
        mov   ebx,21
        mov   ecx,[esi+8]
        mov   edx,[esi+12]
        mov   edi,[esi+16]
        call  [drawbar]
      noinside:

        popad

        ret


draw_rectangle:

r_eax equ [esp+28]   ; x start
r_ax  equ [esp+30]   ; x end
r_ebx equ [esp+16]   ; y start
r_bx  equ [esp+18]   ; y end
;esi                 ; color

        pushad

        mov   ecx,esi          ; yb,xb -> yb,xe
        mov   eax, r_eax
        rol   eax, 16
        mov   ebx,r_ebx
        shl   ebx,16
        mov   bx,r_ebx
        xor   edi, edi
        call  [draw_line]

        mov   ebx,r_bx         ; ye,xb -> ye,xe
        shl   ebx,16
        mov   bx,r_bx
        call  [draw_line]

        mov   ecx,esi          ; ya,xa -> ye,xa
        mov   eax,r_eax
        shl   eax,16
        mov   ax,r_eax
        mov   ebx,r_ebx
        shl   ebx,16
        mov   bx,r_bx
        mov   edi,0
        call  [draw_line]

        mov   eax,r_ax       ; ya,xe -> ye,xe
        shl   eax,16
        mov   ax,r_ax
        call  [draw_line]

        popad
        ret


drawwindow_III:

        pushad

        mov   edi,edx                              ; RECTANGLE
        mov   eax,[edi+0]
        shl   eax,16
        mov   ax,[edi+0]
        add   ax,[edi+8]
        mov   ebx,[edi+4]
        shl   ebx,16
        mov   bx,[edi+4]
        add   bx,[edi+12]
        mov   esi,[edi+24]
        shr   esi,1
        and   esi,0x007f7f7f
        push  esi
        call  draw_rectangle
        mov   ecx,3
      dw3l:
        add   eax,1*65536-1
        add   ebx,1*65536-1
        mov   esi,[edi+24]
        call  draw_rectangle
        dec   ecx
        jnz   dw3l
        pop   esi
        add   eax,1*65536-1
        add   ebx,1*65536-1
        call  draw_rectangle

        mov   ecx,[edx+20]                       ; GRAB BAR
        push  ecx
        mov   esi,edx
        mov   edx,[esi+04]
        add   edx,4
        mov   ebx,[esi+04]
        add   ebx,20
        mov   eax,[esi+04]
        add   eax,[esi+12]
        cmp   ebx,eax
        jb    wdsizeok2
        mov   ebx,eax
      wdsizeok2:
        push  ebx
      drwi2:
        mov   ebx,edx
        shl   ebx,16
        add   ebx,edx
        mov   eax,[esi+00]
        shl   eax,16
        add   eax,[esi+00]
        add   eax,[esi+8]
        add   eax,4*65536-4
        mov   ecx,[esi+20]
        test  ecx,0x40000000
        jz    nofa3
        add   ecx,0x040404
      nofa3:
        test  ecx,0x80000000
        jz    nofa2
        sub   ecx,0x040404
      nofa2:
        mov   [esi+20],ecx
        and   ecx,0xffffff
        xor   edi, edi
        call  [draw_line]
        inc   edx
        cmp   edx,[esp]
        jb    drwi2
        add   esp,4
        pop   ecx
        mov   [esi+20],ecx

        mov   edx,[esi+04]                       ; WORK AREA
        add   edx,21+5
        mov   ebx,[esi+04]
        add   ebx,[esi+12]
        cmp   edx,ebx
        jg    noinside2
        mov   eax,5
        mov   ebx,20
        mov   ecx,[esi+8]
        mov   edx,[esi+12]
        sub   ecx,4
        sub   edx,4
        mov   edi,[esi+16]
        call  [drawbar]
      noinside2:

        popad

        ret



; activate window
align 4
windowactivate:

        ; esi = abs mem position in stack 0xC400+

        pushad

     ; if type of current active window is 3,
     ; it must be redrawn
        mov   eax, [0x3004]
        movzx eax, word [0xC400 + eax*2]
        shl   eax, 5
        add   eax, window_data
        mov   ebx, [eax + WDATA.cl_workarea]
        and   ebx, 0x0f000000
        cmp   ebx, 0x03000000
        jne   @f
        mov   [eax + WDATA.fl_redraw], byte 1
     @@:

        push   esi
      movzx   eax, word [esi] ; ax <- process no
      movzx   eax, word [0xC000+eax*2] ; ax <- position in window stack

        xor   esi, esi        ; drop others
      waloop:
        cmp   esi, dword [0x3004]
        jae   wacont
        inc   esi
        lea   edi, [0xC000 + esi*2]
        mov   bx, [edi] ; position of the current process
        cmp   bx, ax
        jbe   @f
        dec   bx       ; upper? => drop!
        mov   [edi], bx
      @@:
        jmp   waloop
      wacont:
                            ; set to no 1
        pop   esi           ;   esi = pointer at 0xC400

      movzx   eax, word [esi]
        mov   bx, [0x3004]  ; number of processes
        mov   [0xC000+eax*2], bx     ; this is the last (and the upper)

        ; update on screen -window stack
        xor   esi, esi
      waloop2:
        mov   edi, [0x3004]
        cmp   esi, edi
        jae   wacont2
        inc   esi
        movzx ebx, word [esi*2 + 0xC000]
        mov   [ebx*2 + 0xC400], si
        jmp   waloop2
      wacont2:
        mov   [0xf400], byte 0           ; empty keyboard buffer
        mov   [0xf500], byte 0           ; empty button buffer
        popad
        ret


; check if window is necessary to draw

checkwindowdraw:

        ; edi = position in window_data+

        mov   eax, [edi + WDATA.cl_workarea]
        and   eax, 0x0f000000
        cmp   eax, 0x03000000
        je    .return_yes      ; window type 3

        mov   esi, edi
        sub   esi, window_data
        shr   esi, 5

        ; esi = process number

        movzx eax, word [0xC000 + esi * 2] ; get value of the curr process
        lea   esi, [0xC400 + eax * 2]      ; get address of this process at 0xC400

        push  esi

      .new_check:

        pop   esi
        add   esi, 2
        push  esi

        mov   eax, [0x3004]
        lea   eax, word [0xC400 + eax * 2] ; number of the upper window

        cmp   esi, eax
        ja    .all_wnds_to_top

        movzx eax, word [esi]
        shl   eax, 5
        cmp   [0x3000 + eax + 0xa], byte 9
        je    .new_check                    ; skip dead windows

        lea   esi, [eax+window_data]

        mov   ebx, [edi+4]  ; y0
        mov   edx, [edi+12]
        add   edx, ebx      ; y0e

        mov   ecx, [esi+4]  ; y  ; y check
        cmp   ecx, edx      
        jae   .new_check     ; y < y0e
        mov   eax, [esi+12]
        add   ecx, eax       ; ye
        cmp   ebx, ecx       ; y0 >= ye
        ja    .new_check

        mov   eax, [edi+0]   ; x0
        mov   ecx, [edi+8]
        add   ecx, eax       ; x0e

        mov   edx, [esi+0]   ; x ; x check
        cmp   edx, ecx
        jae   .new_check     ; x < x0e
        mov   ecx, [esi+8]
        add   edx, ecx
        cmp   eax, edx
        ja    .new_check

        pop   esi
  .return_yes:
        mov   ecx,1   ; overlap some window
        ret

  .all_wnds_to_top:

        pop   esi

        xor   ecx, ecx       ; passed all windows to top
        ret




waredraw:     ; if redraw necessary at activate

        pushad

        call  checkwindowdraw      ; draw window on activation ?
        test  ecx, ecx
        jz    .do_not_draw

        popad
        mov   [0xfb44], byte 1     ; do draw mouse
        call  windowactivate

        ; update screen info
        pushad
        mov   edi, [0x3004] ; the last process (number)
        movzx esi, word [0xC400 + edi * 2]
        shl   esi, 5
        add   esi, window_data

        ; coordinates of the upper window
        mov   eax, [esi+00] ; cx
        mov   ebx, [esi+04] ; cy
        mov   ecx, [esi+08] ; sx
        mov   edx, [esi+12] ; sy

        add   ecx, eax       ; ecx = x_end
        add   edx, ebx       ; edx = y_end

        mov   edi, [0x3004]
        movzx esi, word [0xC400 + edi * 2]
        call  setscreen
        popad

        mov   [edi+31],byte 1  ; redraw flag for app
        mov   [0xfb44],byte 0  ; mouse down checks

        ret

     .do_not_draw:

        popad

        call  windowactivate
        mov   [0xfb44],byte 0  ; mouse down checks
        mov   [0xfff4],byte 0  ; no mouse background
        mov   [0xfff5],byte 0  ; draw mouse
        ret


; eax = window number on screen
; corrupts registers and [dl*]
minimize_window:
        movzx eax, word [0xC400+eax*2]
        shl   eax, 5
        add   eax, window_data
        test  [eax+WDATA.fl_wstate], WSTATE_MINIMIZED
        jnz   .skip_redrawings
        pushfd
        cli
        or    [eax+WDATA.fl_wstate], WSTATE_MINIMIZED
        mov   edi, eax
        ;call  calculatescreen
        mov   eax, [edi+WDATA.left]
        mov   [dlx], eax
        mov   ecx, eax
        add   ecx, [edi+WDATA.width]
        mov   [dlxe], ecx
        mov   ebx, [edi+WDATA.top]
        mov   [dly], ebx
        mov   edx, ebx
        add   edx, [edi+WDATA.height]
        mov   [dlye], edx
        call  calculatescreen
        xor   esi, esi
        xor   eax, eax
        call  redrawscreen
        popfd
.skip_redrawings:
        ret

; eax = window number on screen
; corrupts registers and [dl*]
restore_minimized_window:
        pushfd
        cli
        movzx esi, word [0xC400+eax*2]
        mov   edi, esi
        shl   edi, 5
        add   edi, window_data
        test  [edi+WDATA.fl_wstate], WSTATE_MINIMIZED
        jz    .skip_redrawings
        mov   [edi+WDATA.fl_redraw], 1
        and   [edi+WDATA.fl_wstate], not WSTATE_MINIMIZED
        cmp   eax, [0x3004] ; the uppermost window
        jnz   .no_uppermost
        mov   eax, [edi+WDATA.left]
        mov   ebx, [edi+WDATA.top]
        mov   ecx, eax
        mov   edx, ebx
        add   ecx, [edi+WDATA.width]
        add   edx, [edi+WDATA.height]
        call  setscreen
        jmp   .done
.no_uppermost:
        mov   eax, [edi+WDATA.left]
        mov   ebx, [edi+WDATA.top]
        mov   ecx, eax
        mov   edx, ebx
        add   ecx, [edi+WDATA.width]
        add   edx, [edi+WDATA.height]
        call  calculatescreen
.done:
        mov   [0xfff4],byte 0 ; no mouse under
.skip_redrawings:
        popfd
        ret


iglobal
  window_moving   db 'K : Window - move/resize',13,10,0
  window_moved    db 'K : Window - done',13,10,0
endg

; check window touch
align 4
checkwindows:
        pushad

        cmp   [window_minimize], 0
        je    .no_minimizing
        mov   eax, [0x3004]       ; the uppermost window
        mov   bl, 0
        xchg  [window_minimize], bl
        cmp   bl, 1
        jne   .restore
        call  minimize_window
        jmp   .continue
  .restore:
        call  restore_minimized_window
  .continue:
  .no_minimizing:

        cmp   [0xfb40],byte 0    ; mouse buttons pressed ?
        jne   .mouse_buttons_pressed
        popad
        ret
    .mouse_buttons_pressed:

        mov   esi,[0x3004]
        inc   esi

      cwloop:
        cmp   esi,2
        jb   .exit

        dec   esi
        movzx edi, word [0xC400 + esi * 2] ; ebx
        shl   edi, 5
        add   edi, window_data
;        mov   edi, ebx
        mov   ecx, [edi+0]
        mov   edx, [edi+4]

        mov   eax,ecx
        mov   ebx,edx
        test    [edi+WDATA.fl_wstate],WSTATE_MINIMIZED
        jnz     cwloop

       movzx  eax, word [0xfb0a]
       movzx  ebx, word [0xfb0c]
       
        cmp   ecx, eax
        jae   cwloop
        cmp   edx, ebx
        jae   cwloop
        add   ecx, [edi+8]
        add   edx, [edi+12]
        cmp   eax, ecx
        jae   cwloop
        cmp   ebx, edx
        jae   cwloop

        pushad
        mov   eax, esi
        mov   ebx, [0x3004]
        cmp   eax, ebx      ; is this window active?
        jz    .move_resize_window

        ; eax = position in windowing stack
        ; redraw must ?
        lea   esi, [0xC400 + esi * 2]
        call  waredraw
        add   esp, 32

      .exit:
        popad
        ret

   .move_resize_window:    ; MOVE OR RESIZE WINDOW

        popad

        ; Check for user enabled fixed window
        mov   edx, [edi+0x14]
        and   edx, 0x0f000000
        cmp   edx, 0x01000000
        jne   .window_move_enabled_for_user
        popad
        ret
      .window_move_enabled_for_user:

        test    [edi+WDATA.fl_wstate],WSTATE_ROLLEDUP
        jnz     .no_resize_2

        mov   [do_resize_from_corner],byte 0   ; resize for skinned window
        mov   edx, [edi+0x10]
        and   edx, 0x0f000000
        cmp   edx, 0x02000000
        jb    .no_resize_2 ; not type 2 wnd

        mov   edx, [edi+4]
        add   edx, [edi+12]
        sub   edx, 6       ; edx = y_end - 6
        cmp   ebx, edx     ; ebx = mouse_y
        jb    .no_resize_2
        mov   [do_resize_from_corner],byte 1
        jmp   .continue
      .no_resize_2:

        push    eax
        call    get_titlebar_height
        add     eax,[edi+4]
        cmp     ebx,eax
        pop     eax
        jae     .exit

     .continue:

        push  esi
        mov   esi, window_moving
        call  sys_msg_board_str
        pop   esi

        mov   ecx, [timer_ticks]     ; double-click ?
        mov   edx, ecx
        sub   edx, [latest_window_touch]
        mov   [latest_window_touch], ecx
        mov   [latest_window_touch_delta], edx

        mov   cl, [0xfb40]     ; save for shade check
        mov   [do_resize], cl
     no_emulation_righ_button:
        mov   ecx, [edi+0]
        mov   edx, [edi+4]

        push  eax ecx edx
        mov   [dlx], ecx      ; save for drawlimits
        mov   [dly], edx
        mov   eax, [edi+8]
        add   ecx, eax
        mov   eax, [edi+12]
        add   edx, eax
        mov   [dlxe], ecx
        mov   [dlye], edx
        pop   edx ecx eax

        sub   eax, ecx
        sub   ebx, edx

        mov   esi, [0xfb0a]
        mov   [0xf300], esi

        pushad           ; wait for putimages to finish
;        mov   eax,5
;        call  delay_hs
        mov   eax,[edi+0]
        mov   [npx],eax
        mov   eax,[edi+4]
        mov   [npy],eax
        popad

        push eax                  ; save old coordinates
        mov   ax,[edi+00]
        mov   word [oldc+00],ax
        mov   ax,[edi+04]
        mov   word [oldc+04],ax
        mov   ax,[edi+8]
        mov   word [oldc+8],ax
        mov   word [npxe],ax
        mov   ax,[edi+12]
        mov   word [oldc+12],ax
        mov   word [npye],ax
        pop eax

        test    [edi+WDATA.fl_wstate],WSTATE_MAXIMIZED
        jnz     @f
        call  drawwindowframes
    @@:

        mov   [reposition],0
        mov   [0xfb44],byte 1   ; no reaction to mouse up/down

        ; move window

      newchm:

        mov   [0xfff5],byte 1

        call  checkidle

        call  checkEgaCga

        mov   [0xfff4],byte 0

        call  [draw_pointer]

        pushad
        call   stack_handler
        popad

        mov   esi,[0xf300]
        cmp   esi,[0xfb0a]
        je    cwb

        mov   cx,[0xfb0a]
        mov   dx,[0xfb0c]
        sub   cx,ax
        sub   dx,bx

        push  ax
        push  bx

        test    [edi+WDATA.fl_wstate],WSTATE_MAXIMIZED
        jnz     @f
        call  drawwindowframes
    @@:

        mov   ax,[0xfe00]
        mov   bx,[0xfe04]

        cmp   [do_resize_from_corner],1
        je    no_new_position

        mov   word [npx],word 0     ; x repos ?
        cmp   ax,cx
        jb    noreposx
        mov   [reposition],1
        sub   ax,word [npxe]
        mov   word [npx],ax
        cmp   ax,cx
        jb    noreposx
        mov   word [npx],cx
      noreposx:

        mov   word [npy],word 0     ; y repos ?
        cmp   bx,dx
        jb    noreposy
        mov   [reposition],1
        sub   bx,word [npye]
        mov   word [npy],bx
        cmp   bx,dx
        jb    noreposy
        mov   word [npy],dx
      noreposy:

      no_new_position:

        cmp   [do_resize_from_corner],0    ; resize from right corner
        je    norepos_size
        pushad

        mov   edx,edi
        sub   edx,window_data
        shr   edx,5
        shl   edx,8
        add   edx,0x80000                 ; process base at 0x80000+

        movzx eax,word [0xfb0a]
        cmp   eax,[edi+0]
        jb    nnepx
        sub   eax,[edi+0]
        cmp   eax,32 ; [edx+0x90+8]
        jge   nnepx2
        mov   eax,32 ; [edx+0x90+8]
      nnepx2:
        mov   [npxe],eax
      nnepx:

        call    get_rolledup_height
        mov     ebx,eax
        movzx eax,word [0xfb0c]
        cmp   eax,[edi+4]
        jb    nnepy
        sub   eax,[edi+4]
        cmp     eax,ebx ; [edx+0x90+12]
        jge     nnepy2
        mov     eax,ebx ; [edx+0x90+12]
      nnepy2:
        mov   [npye],eax
      nnepy:

        mov   [reposition],1

        popad
      norepos_size:

        pop   bx
        pop   ax
        test    [edi+WDATA.fl_wstate],WSTATE_MAXIMIZED
        jnz     @f
        call  drawwindowframes
    @@:

        mov   esi,[0xfb0a]
        mov   [0xf300],esi

      cwb:
        cmp   [0xfb40],byte 0
        jne   newchm 
                                     ; new position done
        mov     [0xfff5],byte 1
        mov     cl,0
        test    [edi+WDATA.fl_wstate],WSTATE_MAXIMIZED
        jnz     @f
        mov     cl,[reposition]
        call    drawwindowframes

        mov   eax,[npx]
        mov   [edi+0],eax
        mov   eax,[npy]
        mov   [edi+4],eax
        mov   eax,[npxe]
        mov   [edi+8],eax
        mov   eax,[npye]
        mov   [edi+12],eax

    @@: mov     [reposition],cl

        cmp   [reposition],1         ; save new position and size
        jne   no_bounds_save
        push  esi edi ecx
        mov   esi,edi
        mov   ecx,2
        test  [edi+WDATA.fl_wstate],WSTATE_ROLLEDUP or WSTATE_MAXIMIZED
        jnz   @f
        add   ecx,2
    @@: sub   edi,window_data
        shr   edi,5
        shl   edi,8
        add   edi,0x80000+0x90
        cld
        rep   movsd
        pop   ecx edi esi
      no_bounds_save:

        pushad                             ; WINDOW SHADE/FULLSCREEN

        cmp   [reposition],1
        je    no_window_sizing
        mov   edx,edi
        sub   edx,window_data
        shr   edx,5
        shl   edx,8
        add   edx,0x80000                 ; process base at 0x80000+

        cmp   [do_resize],2               ; window shade ?
        jne   no_window_shade
        mov   [reposition],1

        test    [edi+WDATA.fl_wstate],WSTATE_ROLLEDUP
        jnz     wnd_rolldown
  wnd_rollup:
        or      [edi+WDATA.fl_wstate],WSTATE_ROLLEDUP
        call    get_rolledup_height
        jmp     @f
  wnd_rolldown:
        and     [edi+WDATA.fl_wstate],not WSTATE_ROLLEDUP
        mov     eax,[edx+0x90+BOX.height]
        test    [edi+WDATA.fl_wstate],WSTATE_MAXIMIZED
        jz      @f
        mov     eax,[screen_workarea.bottom]
        sub     eax,[screen_workarea.top]
    @@: mov     [edi+WDATA.height],eax

      no_window_shade:

        cmp   [do_resize],1               ; fullscreen/restore ?
        jne   no_fullscreen_restore
        cmp   [latest_window_touch_delta],dword 50
        jg    no_fullscreen_restore
        mov   [reposition],1
        test    [edi+WDATA.fl_wstate],WSTATE_MAXIMIZED
        jnz     restore_from_fullscreen
        or      [edi+WDATA.fl_wstate],WSTATE_MAXIMIZED
        mov     eax,[screen_workarea.left]
        mov     [edi+WDATA.left],eax
        sub     eax,[screen_workarea.right]
        neg     eax
        mov     [edi+WDATA.width],eax
        mov     eax,[screen_workarea.top]
        mov     [edi+WDATA.top],eax
        test    [edi+WDATA.fl_wstate],WSTATE_ROLLEDUP
        jnz     @f
        sub     eax,[screen_workarea.bottom]
        neg     eax
        mov     [edi+WDATA.height],eax
    @@:
        jmp   no_fullscreen_restore
      restore_from_fullscreen:
        and     [edi+WDATA.fl_wstate],not WSTATE_MAXIMIZED
        push    [edi+WDATA.height]
        push  edi                         ; restore
        lea   esi, [edx + 0x90]
        mov   ecx,4
        cld
        rep   movsd
        pop   edi
        pop     eax
        test    [edi+WDATA.fl_wstate],WSTATE_ROLLEDUP
        jz      @f
        mov     [edi+WDATA.height],eax
    @@:

      no_fullscreen_restore:

        mov   eax,[edi+4]                 ; check Y inside screen
        add   eax,[edi+12]
        cmp   eax,[0xfe04]
        jbe   no_window_sizing
        mov   eax,[edi+0]                 ; check X inside screen
        add   eax,[edi+8]
        cmp   eax,[0xfe00]
        jbe   no_window_sizing
        mov   eax,[0xfe00]
        sub   eax,[edi+8]
        mov   [edi+0],eax
        mov   eax,[0xfe04]
        sub   eax,[edi+12]
        mov   [edi+4],eax
      no_window_sizing:

        popad

        cmp   [reposition],0
        je    retwm

        mov   [0xfff5],byte 1 ; no mouse


        push  eax ebx ecx edx
        mov   eax,[edi+00]
        mov   ebx,[edi+04]
        mov   ecx,[edi+8]
        mov   edx,[edi+12]
        add   ecx,eax
        add   edx,ebx
        call  calculatescreen

        mov   eax,[oldc+00]
        mov   ebx,[oldc+04]
        mov   ecx,[oldc+8]
        mov   edx,[oldc+12]
        add   ecx,eax
        add   edx,ebx
        call  calculatescreen
        pop   edx ecx ebx eax

        mov   eax,edi
        call  redrawscreen


        mov     [edi+WDATA.fl_redraw],1

        mov   ecx,100         ; wait to avoid mouse residuals
      waitre2:
        mov   [0xfff5],byte 1
        call  checkidle
        cmp     [edi+WDATA.fl_redraw],0
        jz    retwm
        loop  waitre2

      retwm:

        mov   [0xfff5],byte 0 ; mouse pointer
        mov   [0xfff4],byte 0 ; no mouse under
        mov   [0xfb44],byte 0 ; react to mouse up/down

        mov    esi,window_moved
        call   sys_msg_board_str

        popad

        ret


uglobal
  add_window_data            dd  0
  do_resize_from_corner      db  0x0
  reposition                 db  0x0
  latest_window_touch        dd  0x0
  latest_window_touch_delta  dd  0x0

  do_resize db 0x0

  oldc    dd 0x0,0x0,0x0,0x0

  dlx     dd 0x0
  dly     dd 0x0
  dlxe    dd 0x0
  dlye    dd 0x0

  npx     dd 0x0
  npy     dd 0x0
  npxe    dd 0x0
  npye    dd 0x0

  mpx     dd 0x0
  mpy     dd 0x0
endg


; draw negative window frames

drawwindowframes:

        pushad

        mov   eax,[npx]
        shl   eax,16
        add   eax,[npx]
        add   eax,[npxe]
        add   eax,65536*1-1
        mov   ebx,[npy]
        shl   ebx,16
        add   ebx,[npy]
        mov   ecx,0x01000000
        push  edi
        mov   edi,1
        call  [draw_line]
        pop   edi

        mov   eax,[npx]
        shl   eax,16
        add   eax,[npx]
        add   eax,[npxe]
        add   eax,65536*1-1
        mov   ebx,[npy]
        add   ebx,[npye]
        shl   ebx,16
        add   ebx,[npy]
        add   ebx,[npye]
        mov   ecx,0x01000000
        push  edi
        mov   edi,1
        call  [draw_line]
        pop   edi

        mov   eax,[npx]
        shl   eax,16
        add   eax,[npx]
        mov   ebx,[npy]
        shl   ebx,16
        add   ebx,[npy]
        add   ebx,[npye]
        mov   ecx,0x01000000
        push  edi
        mov   edi,1
        call  [draw_line]
        pop   edi

        mov   eax,[npx]
        add   eax,[npxe]
        shl   eax,16
        add   eax,[npx]
        add   eax,[npxe]
        mov   ebx,[npy]
        shl   ebx,16
        add   ebx,[npy]
        add   ebx,[npye]
        mov   ecx,0x01000000
        push  edi
        mov   edi,1
        call  [draw_line]
        mov   edi,[0x3000]
        shl   edi,5
        add   edi,window_data
        mov   [edi+30],byte 1
        pop   edi

        popad

        ret



random_shaped_window:

;
;  eax = 0    giving address of data area
;      ebx    address
;  ebx = 1    shape area scale
;      ebx    2^ebx scale

     test eax, eax
     jne  rsw_no_address
     mov  eax,[0x3000]
     shl  eax,8

     mov  [eax+0x80000+0x80],ebx
   rsw_no_address:

     cmp  eax,1
     jne  rsw_no_scale
     mov  eax,[0x3000]
     shl  eax,8
     mov  [eax+0x80000+0x84],bl
   rsw_no_scale:

     ret


