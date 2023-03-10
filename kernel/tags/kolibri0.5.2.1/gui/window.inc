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
; ????: ????????? ??? ????, ??????? ? ?????? ???????,
;       ? ??? ??????? ??????? setscreen
align 4
calculatescreen:
        pushad
        cli

        mov   esi, 1
        xor   eax, eax
        xor   ebx, ebx
        mov   ecx, [0xFE00]
        mov   edx, [0xFE04]
        call  setscreen

        mov   ebp, [0x3004]        ; number of processes
        cmp   ebp, 1
        jbe   .finish
        align 4
      .new_wnd:
        movzx edi, word [0xC400 + esi * 2]
        shl   edi, 5
        add   edi, window_data
        mov   eax, [edi+0]
        mov   ebx, [edi+4]
        mov   ecx, [edi+8]
        test  ecx, ecx
        jz    .not_wnd
        add   ecx, eax
        mov   edx, [edi+12]
        add   edx, ebx
        push  esi
        movzx esi, word [0xC400 + esi * 2]
        call  setscreen
        pop   esi
      .not_wnd:
        inc   esi
        dec   ebp
        jnz   .new_wnd
      .finish:
        sti
        popad
ret



virtual at esp
  ff_xsz   dd ?
  ff_ysz   dd ?
  ff_scale dd ?
end virtual

align 4
; ??????????? ????? ??? ???? ????????? ????????
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
        imul  eax, ebx   ;ebx
        add   eax, [esp] ;eax
        add   eax, WinMapAddress
        mov   ebp, eax

        mov   edi, [edi]
;        mov   eax, esi
;        shl   eax, 5
;        add   edi, [eax+0x3000+0x10]
        pop   eax

        ; eax = x_start
        ; ebx = y_start
        ; ecx = x_size
        ; edx = y_size
        ; esi = process_number
        ; edi = &shape
        ;       [scale]
        push edx ecx ;ebx eax
        xor  ebx, ebx
        align 4
      .ff_new_y:
        xor  edx, edx
        align 4
      .ff_new_x:
        ; -- body --
        mov  ecx, [ff_scale]
        mov  eax, [ff_xsz]
        shr  eax, cl
        push ebx edx
        shr  ebx, cl
        shr  edx, cl
        imul eax, ebx
        add  eax, edx
        pop  edx ebx
        add  eax, edi ;[ff_shape]
        call .read_byte
;        cmp  byte [eax], 1
;        jne  @f
        test al,al
        jz   @f
        mov  eax, esi  ; [ff_proc]
        mov  [ebp], al
       @@:
        ; -- end body --
        inc  ebp
        inc  edx
        cmp  edx, [ff_xsz]
        jb   .ff_new_x
        sub  ebp, [ff_xsz]
        add  ebp, [0xFE00]  ; screen.x
        inc  ebp
        inc  ebx
        cmp  ebx, [ff_ysz]
        jb   .ff_new_y

        pop  ecx edx   ; eax ebx first
        add  esp, 4
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


     pushad

     test eax, eax      ; redraw display
     jnz  dspl0
     test ebx, ebx
     jnz  dspl0
     cmp  [windowtypechanged],dword 1
     jne  dspl00
     mov  [windowtypechanged],dword 0
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

     popad
     ret

uglobal
  common_colours:
     times 128 db 0x0
endg

check_window_move_request:

        pushad

        mov   edi,[window_move_pr]    ; requestor process base

        cmp   edi,0
        je    window_move_return

        shl   edi,5
        add   edi,window_data

        push  dword [edi+0]           ; save old coordinates
        push  dword [edi+4]
        push  dword [edi+8]
        push  dword [edi+12]

        mov   eax,[window_move_eax]
        mov   ebx,[window_move_ebx]
        mov   ecx,[window_move_ecx]
        mov   edx,[window_move_edx]

        cmp   eax,-1                  ; set new position and size
        je    no_x_reposition
        mov   [edi+0],eax
      no_x_reposition:
        cmp   ebx,-1
        je    no_y_reposition
        mov   [edi+4],ebx
      no_y_reposition:
        cmp   ecx,-1
        je    no_x_resizing
        mov   [edi+8],ecx
      no_x_resizing:
        cmp   edx,-1
        je    no_y_resizing
        mov   [edi+12],edx
      no_y_resizing:

        call  check_window_position

        pushad                       ; save for window fullscreen/resize
        mov   esi,edi
        sub   edi,window_data
        shr   edi,5
        shl   edi,8
        add   edi,0x80000+0x90
        mov   ecx,4
        cld
        rep   movsd
        popad

        pushad                       ; calculcate screen at new position
        mov   eax,[edi+00]
        mov   ebx,[edi+04]
        mov   ecx,[edi+8]
        mov   edx,[edi+12]
        add   ecx,eax
        add   edx,ebx
        call  calculatescreen
        popad

        pop   edx                   ; calculcate screen at old position
        pop   ecx
        pop   ebx
        pop   eax
        add   ecx,eax
        add   edx,ebx
        mov   [dlx],eax             ; save for drawlimits
        mov   [dly],ebx
        mov   [dlxe],ecx
        mov   [dlye],edx
        call  calculatescreen

        mov   [edi+31],byte 1       ; flag the process as redraw

        mov   eax,edi               ; redraw screen at old position
        xor   esi,esi
        call  redrawscreen

        mov   [0xfff5],byte 0 ; mouse pointer
        mov   [0xfff4],byte 0 ; no mouse under
        mov   [0xfb44],byte 0 ; react to mouse up/down

        mov   ecx,10          ; wait 1/10 second
      wmrl3:
        call  check_mouse_data
        call  [draw_pointer]
        mov   eax,1
        call  delay_hs
        loop  wmrl3

        mov   [window_move_pr],0

      window_move_return:

        popad

        ret




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
     ;<<< 14.11.2004 Ivan Poddubny  <faster and smaller>
;        mov   eax,r_eax
;        shl   eax,16
;        mov   ax,r_ax
        mov   eax, r_eax
        rol   eax, 16
     ;>>> 14.11.2004 Ivan Poddubny  <faster and smaller>
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

;* start code - get active process (4) - Mario79
         mov  [active_process],eax
;* end code - get active process (4) - Mario79

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

        mov   esi, edi
        sub   esi, window_data
        shr   esi, 5

        ; esi = process number

; <IP 15.08.2004>
        movzx eax, word [0xC000 + esi * 2] ; get value of the curr process
        lea   esi, [0xC400 + eax * 2]      ; get address of this process at 0xC400
; </IP 15.08.2004>

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
        add   eax, window_data
        mov   esi, eax

        mov   ebx, [edi+4]
        mov   edx, [edi+12]
        add   edx, ebx

        mov   ecx, [esi+4]    ; y check
        cmp   ecx, edx
        jae   .new_check
        mov   eax, [esi+12]
        add   ecx, eax
        cmp   ebx, ecx
        ja    .new_check

        mov   eax, [edi+0]
        mov   ecx, [edi+8]
        add   ecx, eax

        mov   edx, [esi+0]    ; x check
        cmp   edx, ecx
        jae   .new_check
        mov   ecx, [esi+8]
        add   edx, ecx
        cmp   eax, edx
        ja    .new_check

        pop   esi
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
        shl   esi, 5
        movzx esi, byte [esi + 0x3000 + 0xE]
        call  setscreen ;;;calculatescreen ; setscreen
        popad

        cmp   [0xff01], dword 1 ; if > 1 then activate process
        jbe   .not_activate

;;;        mov   eax, 10            ; wait for putimages to finish
;;;        call  delay_hs

        mov   [edi+31],byte 1  ; redraw flag for app
        mov   [0xfb44],byte 0  ; mouse down checks

        ret

     .not_activate:

;        mov   eax,5            ; wait for putimages to finish
;        call  delay_hs

        mov   [edi+31],byte 1  ; redraw flag for app

        mov   ecx, 25 ;100
     .waitflagdown:
        dec   ecx
        jz    .nowait
        mov   eax, 2
        call  delay_hs
        cmp   [edi+31], byte 0 ; wait flag to drop
        jnz   .waitflagdown
      .nowait:

;        mov   ecx,10
;      .wait:
;        mov   eax,1           ; wait for draw to finish
;        call  delay_hs
;        loop  .wait

        mov   [0xfb44],byte 0

        ret

     .do_not_draw:

        popad

        call  windowactivate
        mov   [0xfb44],byte 0  ; mouse down checks
        mov   [0xfff4],byte 0  ; no mouse background
        mov   [0xfff5],byte 0  ; draw mouse
        ret


iglobal
  window_moving   db 'K : Window - move/resize',13,10,0
  window_moved    db 'K : Window - done',13,10,0
endg

; check window touch
align 4
checkwindows:
        pushad

        cmp  [0xff01],dword 1  ; activate request from app ?
        jbe  .no_activate_request
        mov  edi,[0xff01]     ; process number
        shl  edi,5
        add  edi,window_data
        mov  ebx,[0xff01]
        movzx esi, word [0xC000 + ebx * 2]
        lea  esi, [0xC400 + esi * 2]
        call waredraw

;* start code - get active process (2) - Mario79
        mov  eax,[0xff01]
        mov  [active_process],eax
;* end code - get active process  (2) - Mario79

        mov  [0xff01],dword 0  ; activated

        popad
        ret

    .no_activate_request:
    
        cmp   [0xfb40],byte 0    ; mouse buttons pressed ?
        jne   .mouse_buttons_pressed
;        cmp   [window_minimize],2
;        jne   .no_activate_request_1
        cmp   [window_minimize],0
        je   .no_activate_request_2
        cmp   [window_minimize],1
        je    .mouse_buttons_pressed
        mov   esi,[0x3004]
        movzx edi, word [0xC400 + esi * 2]
        shl   edi, 5
        add   edi, window_data
        cmp   [edi+12],dword 0
        je    .mouse_buttons_pressed
;        jne   .no_activate_request_2
;    .no_activate_request_1:
;        cmp   [window_minimize],1
;        jge   .mouse_buttons_pressed
;        jmp   .mouse_buttons_pressed
    .no_activate_request_2:
        mov   [window_minimize],0
        popad
        ret

    .mouse_buttons_pressed:

        mov   esi,[0x3004]
        inc   esi

      cwloop:
        cmp   esi,2
        jb   .exit
        .temp_window_minimize_1:
        dec   esi
        movzx edi, word [0xC400 + esi * 2] ; ebx
        shl   edi, 5
        add   edi, window_data
;        mov   edi, ebx
        mov   ecx, [edi+0]
        mov   edx, [edi+4]

        mov   eax,ecx
        mov   ebx,edx
       cmp   [window_minimize],1
       jge   .window_minimize_no_check_mouse

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

      .window_minimize_no_check_mouse:

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

;// mike.dld [
        mov   dl,[edi+0x10+3]
        and   dl,0x0F
        cmp   dl,0x03
        jne   @f
        mov   edx, [edi+4] ; check if touch on bar
        add   edx, [_skinh]
        cmp   ebx, edx
        jae   .exit
        jmp   .continue
    @@:
;// mike.dld ]

        mov   edx, [edi+4] ; check if touch on bar
        add   edx, 21
        cmp   ebx, edx
        jae   .exit

     .continue:

        push  esi
        mov   esi, window_moving
        call  sys_msg_board_str
        pop   esi

        mov   ecx, [timer_ticks] ;[0xfdf0]    ; double-click ?
        mov   edx, ecx
        sub   edx, [latest_window_touch]
        mov   [latest_window_touch], ecx
        mov   [latest_window_touch_delta], edx

        mov   cl, [0xfb40]     ; save for shade check
        mov   [do_resize], cl
        cmp  [window_minimize],0
        je    no_emulation_righ_button
        mov   [do_resize], byte 2
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

        call  drawwindowframes

        mov   [reposition],0
        mov   [0xfb44],byte 1   ; no reaction to mouse up/down

        ; move window

      newchm:

        mov   [0xfff5],byte 1

        call  checkidle

        call  checkEgaCga

        mov   [0xfff4],byte 0

        call  check_mouse_data
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

        call  drawwindowframes

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
        cmp   eax,[edx+0x90+8]
        jge   nnepx2
        mov   eax,[edx+0x90+8]
      nnepx2:
        mov   [npxe],eax
      nnepx:

        movzx eax,word [0xfb0c]
        cmp   eax,[edi+4]
        jb    nnepy
        sub   eax,[edi+4]
        cmp   eax,23 ; [edx+0x90+12]
        jge   nnepy2
        mov   eax,23 ; [edx+0x90+12]
      nnepy2:
        mov   [npye],eax
      nnepy:

        mov   [reposition],1

        popad
      norepos_size:

        pop   bx
        pop   ax
        call  drawwindowframes

        mov   esi,[0xfb0a]
        mov   [0xf300],esi

      cwb:
        cmp   [0xfb40],byte 0
        jne   newchm 
                                     ; new position done
        call  drawwindowframes
        mov   [0xfff5],byte 1

        mov   eax,[npx]
        mov   [edi+0],eax
        mov   eax,[npy]
        mov   [edi+4],eax
        mov   eax,[npxe]
        mov   [edi+8],eax
        mov   eax,[npye]
        mov   [edi+12],eax

        cmp   [reposition],1         ; save new X and Y start
        jne   no_xy_save
        ; <IP 28.08.2004>
        push  esi edi ecx
        mov   esi,edi
        sub   edi,window_data
        shr   edi,5
        shl   edi,8
        add   edi,0x80000+0x90
        mov   ecx,2
        cld
        rep   movsd
        pop   ecx edi esi
        ; </IP 28.08.2004>
      no_xy_save:

        pushad                             ; WINDOW SHADE/FULLSCREEN

        cmp   [reposition],1
        je    no_window_sizing
        mov   edx,edi
        sub   edx,window_data
        shr   edx,5
        shl   edx,8
        add   edx,0x80000                 ; process base at 0x80000+

        cmp   [do_resize],2               ; window shade ?
        jb    no_window_shade
        mov   [reposition],1

        cmp   [window_minimize],1
        jge    for_window_minimize
        cmp   [edi+12],dword 23 
        jle    window_shade_up
        jmp   for_window_shade
    for_window_minimize:
        cmp   [edi+12],dword 0 
        je    window_shade_up
    for_window_shade:
        cmp   [window_minimize],0
        je   no_window_minimize
        cmp   [window_minimize],2
        je   window_minimize_no_shade_full ;window_shade_up
        mov   [edi+8],dword 0 ;66
        mov   [edi+12],dword 0 ;279           ; on
        jmp   no_window_shade
  no_window_minimize:  
        mov   [edi+12],dword 23           ; on
        jmp   no_window_shade
      window_shade_up:
        cmp   [window_minimize],1
        jge   window_minimize_no_shade_full
        mov   eax,[edi+0]
        add   eax,[edi+4]
        test  eax, eax
        je    shade_full
        push  edi 
        lea   esi, [edx + 0x90]
        mov   ecx,4
        cld
        rep   movsd
        pop   edi

    window_minimize_no_shade_full:
;        mov   eax,[edx+0x90]              ; off
;        mov   [edi+0],eax
;        mov   eax,[edx+0x94]              ; off
;        mov   [edi+4],eax
;        mov   eax,[edx+0x98]              ; off
;        mov   [edi+8],eax
;        mov   eax,[edx+0x9C]              ; off
;        mov   [edi+12],eax

        push  edi 
        lea   esi, [edx + 0x90]
        mov   ecx,4
        cld
        rep   movsd
        pop   edi
        jmp   no_window_shade

      shade_full:
;        mov   eax,[0xfe00]
;        mov   [edi+8],eax
        mov   eax,[0xfe04]
        sub   eax,19
        mov   [edi+12],eax
      no_window_shade:

        cmp   [do_resize],1               ; fullscreen/restore ?
        jne   no_fullscreen_restore
        cmp   [latest_window_touch_delta],dword 50
        jg    no_fullscreen_restore
        mov   [reposition],1
        mov   eax,[edi+12]
        add   eax,19
        cmp   eax,[0xfe04]
        je    restore_from_fullscreen
        mov   [edi+0],dword 0             ; set fullscreen
        mov   [edi+4],dword 0
        mov   eax,[0xfe00]
        mov   [edi+8],eax
        mov   eax,[0xfe04]
        sub   eax,19
        mov   [edi+12],eax
        jmp   no_fullscreen_restore
      restore_from_fullscreen:
        push  edi                         ; restore
;        mov   esi,edx
;        add   esi,0x90
        lea   esi, [edx + 0x90]
        mov   ecx,4
        cld
        rep   movsd
        pop   edi

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

        pushad
        mov   eax,[edi+00]
        mov   ebx,[edi+04]
        mov   ecx,[edi+8]
        mov   edx,[edi+12]
        add   ecx,eax
        add   edx,ebx
        mov   edi,[0x3004]
        shl   edi,1
        add   edi,0xc400
        movzx esi,byte [edi]
        shl   esi,5
        add   esi,0x3000+0xe
        movzx esi,byte [esi]

        sub   edi,draw_data
        shr   edi,5
        shl   edi,8
        add   edi,0x80000+0x80
        ;cmp   [edi],dword 0
        ;jne   no_rect_shaped_move
        call  setscreen
        jmp   move_calculated
      no_rect_shaped_move:
        call  calculatescreen
      move_calculated:

        popad

        mov   [edi+31],byte 1 ; mark first as redraw
        mov   [0xfff5],byte 1 ; no mouse

        push  eax ebx ecx edx
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

        mov   ecx,100         ; wait to avoid mouse residuals
      waitre2:
        call  check_mouse_data
        mov   [0xfff5],byte 1
        call  checkidle
        cmp   [edi+31],byte 0
        jz    retwm
        loop  waitre2

      retwm:

        mov   [0xfff5],byte 0 ; mouse pointer
        mov   [0xfff4],byte 0 ; no mouse under
        mov   [0xfb44],byte 0 ; react to mouse up/down

        mov    esi,window_moved
        call   sys_msg_board_str

        popad

       mov   [window_minimize],0
;        sti
        ret

;temp_mouse_1 dw 0
;temp_mouse_2 dw 0

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


