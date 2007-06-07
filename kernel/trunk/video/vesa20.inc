$Revision$
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  VESA20.INC                                                  ;;
;;                                                              ;;
;;  Vesa 2.0 functions for MenuetOS                             ;;
;;                                                              ;;
;;  Copyright 2002 Ville Turjanmaa                              ;;
;;  Alexey, kgaz@crosswindws.net                                ;;
;;  - Voodoo compatible graphics                                ;;
;;  Juan M. Caravaca                                            ;;
;;  - Graphics optimimizations eg. drawline                     ;;
;;                                                              ;;
;;  See file COPYING for details                                ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; If you're planning to write your own video driver I suggest
; you replace the VESA12.INC file and see those instructions.

;ScreenWidth             equ     0xfe00
;ScreenHeight            equ     0xfe04
;BytesPerScanLine        equ     0xfe08
;LFBAddress              equ     0xfe80
;ScreenBPP               equ     0xfbf1
;WinMapAddress           equ     0x460000



;*************************************************
; getpixel
;
; in:
; eax = x coordinate
; ebx = y coordinate
;
; ret:
; ecx = 00 RR GG BB

getpixel:
     push    eax ebx edx edi
     call    dword [GETPIXEL]
     pop     edi edx ebx eax
     ret

Vesa20_getpixel24:
; eax = x
; ebx = y
     imul    ebx, [BytesPerScanLine]    ; ebx = y * y multiplier
     lea     edi, [eax+eax*2] ; edi = x*3
     add     edi, ebx         ; edi = x*3+(y*y multiplier)
     add     edi, [LFBAddress]    ; ebx = where pixel is in memory
     mov     ecx, [edi]
     and     ecx, 0xffffff
     ret

Vesa20_getpixel32:
     imul    ebx, [BytesPerScanLine]    ; ebx = y * y multiplier
     lea     edi, [ebx+eax*4] ; edi = x*4+(y*y multiplier)
     add     edi, [LFBAddress]    ; ebx = where pixel is in memory
     mov     ecx, [edi]
     and     ecx, 0xffffff
     ret

;*************************************************

virtual at esp
 putimg:
   .real_sx        dd ?
   .real_sy        dd ?
   .image_sx       dd ?
   .image_sy       dd ?
   .image_cx       dd ?
   .image_cy       dd ?
   .pti            dd ?
   .abs_cx         dd ?
   .abs_cy         dd ?
   .line_increment dd ?
   .winmap_newline dd ?
   .screen_newline dd ?
   .stack_data = 4*12
   .edi         dd      ?
   .esi         dd      ?
   .ebp         dd      ?
   .esp         dd      ?
   .ebx         dd      ?
   .edx         dd      ?
   .ecx         dd      ?
   .eax         dd      ?
   .ret_addr    dd      ?
   .arg_0       dd      ?
end virtual

align 16
; ebx = pointer
; ecx = size [x|y]
; edx = coordinates [x|y]
; ebp = pointer to 'get' function
; esi = pointer to 'init' function
; edi = parameter for 'get' function

vesa20_putimage:
     pushad
     call    [disable_mouse]
     sub     esp, putimg.stack_data
; save pointer to image
     mov     [putimg.pti], ebx
; unpack the size
     mov     eax, ecx
     and     ecx, 0xFFFF
     shr     eax, 16
     mov     [putimg.image_sx], eax
     mov     [putimg.image_sy], ecx
; unpack the coordinates
     mov     eax, edx
     and     edx, 0xFFFF
     shr     eax, 16
     mov     [putimg.image_cx], eax
     mov     [putimg.image_cy], edx
; calculate absolute (i.e. screen) coordinates
     mov     eax, [TASK_BASE]
     mov     ebx, [eax-twdw + WDATA.box.left]
     add     ebx, [putimg.image_cx]
     mov     [putimg.abs_cx], ebx
     mov     ebx, [eax-twdw + WDATA.box.top]
     add     ebx, [putimg.image_cy]
     mov     [putimg.abs_cy], ebx
; real_sx = MIN(wnd_sx-image_cx, image_sx);
     mov     ebx, [eax-twdw + WDATA.box.width] ; ebx = wnd_sx
; \begin{diamond}[20.08.2006]
; note that WDATA.box.width is one pixel less than real window x-size
     inc     ebx
; \end{diamond}[20.08.2006]
     sub     ebx, [putimg.image_cx]
     ja      @f
     add     esp, putimg.stack_data
     popad
     ret
@@:
     cmp     ebx, [putimg.image_sx]
     jbe     .end_x
     mov     ebx, [putimg.image_sx]
.end_x:
     mov     [putimg.real_sx], ebx
; init real_sy
     mov     ebx, [eax-twdw + WDATA.box.height] ; ebx = wnd_sy
; \begin{diamond}[20.08.2006]
     inc     ebx
; \end{diamond}[20.08.2006]
     sub     ebx, [putimg.image_cy]
     ja      @f
     add     esp, putimg.stack_data
     popad
     ret
@@:
     cmp     ebx, [putimg.image_sy]
     jbe     .end_y
     mov     ebx, [putimg.image_sy]
.end_y:
     mov     [putimg.real_sy], ebx
; line increment
     mov     eax, [putimg.image_sx]
     sub     eax, [putimg.real_sx]
;;     imul    eax, [putimg.source_bpp]
;     lea     eax, [eax + eax * 2]
     call    esi
     add     eax, [putimg.arg_0]
     mov     [putimg.line_increment], eax
; winmap new line increment
     mov     eax, [ScreenWidth]
     inc     eax
     sub     eax, [putimg.real_sx]
     mov     [putimg.winmap_newline], eax
; screen new line increment
     mov     eax, [BytesPerScanLine]
     mov     ecx, [putimg.real_sx]
     movzx   ebx, byte [ScreenBPP]
     shr     ebx, 3
     imul    ecx, ebx
     sub     eax, ecx
     mov     [putimg.screen_newline], eax
; pointer to image
     mov     esi, [putimg.pti]
; pointer to screen
     mov     edx, [putimg.abs_cy]
     imul    edx, [BytesPerScanLine]
     mov     eax, [putimg.abs_cx]
     movzx   ebx, byte [ScreenBPP]
     shr     ebx, 3
     imul    eax, ebx
     add     edx, eax
     add     edx, [LFBAddress]
; pointer to pixel map
     mov     eax, [putimg.abs_cy]
     imul    eax, [ScreenWidth]
     add     eax, [putimg.abs_cy]
     add     eax, [putimg.abs_cx]
     add     eax, WinMapAddress
     xchg    eax, ebp
; get process number
     mov     ebx, [CURRENT_TASK]
     cmp     byte [ScreenBPP], 32
     je      put_image_end_32
;put_image_end_24:
     mov     edi, [putimg.real_sy]
align   4
.new_line:
     mov     ecx, [putimg.real_sx]
;     push    ebp edx
align   4
.new_x:
     push    [putimg.edi]
     mov     eax, [putimg.ebp+4]
     call    eax
     cmp     [ebp], bl
     jne     .skip
;     mov     eax, [esi]        ; eax = RRBBGGRR
     mov     [edx], ax
     shr     eax, 16
     mov     [edx+2], al
.skip:
;     add     esi, 3 ;[putimg.source_bpp]
     add     edx, 3
     inc     ebp
     dec     ecx
     jnz     .new_x
;     pop     edx ebp
     add     esi, [putimg.line_increment]
     add     edx, [putimg.screen_newline] ;[BytesPerScanLine]
     add     ebp, [putimg.winmap_newline] ;[ScreenWidth]
;     inc     ebp
     dec     edi
     jnz     .new_line
.finish:
     add     esp, putimg.stack_data
     popad
     ret

put_image_end_32:
     mov     edi, [putimg.real_sy]
align   4
.new_line:
     mov     ecx, [putimg.real_sx]
;     push    ebp edx
align   4
.new_x:
     push    [putimg.edi]
     mov     eax, [putimg.ebp+4]
     call    eax
     cmp     [ebp], bl
     jne     .skip
;     mov     eax, [esi]        ; ecx = RRBBGGRR
     mov     [edx], eax
.skip:
;     add     esi, [putimg.source_bpp]
     add     edx, 4
     inc     ebp
     dec     ecx
     jnz     .new_x
;     pop     edx ebp
     add     esi, [putimg.line_increment]
     add     edx, [putimg.screen_newline] ;[BytesPerScanLine]
     add     ebp, [putimg.winmap_newline] ;[ScreenWidth]
;     inc     ebp
     dec     edi
     jnz     .new_line
.finish:
     add     esp, putimg.stack_data
     popad
     call   VGA__putimage
     mov     [EGA_counter],1
     ret


;*************************************************
align 4
__sys_putpixel:

; eax = x coordinate
; ebx = y coordinate
; ecx = ?? RR GG BB    ; 0x01000000 negation
; edi = 0x00000001 force

;;;        mov  [novesachecksum], dword 0
     pushad
     test  edi,1                 ; force ?
     jnz   .forced
; not forced:
     push  ecx               ; save 24th bit in case negative pixel wanted
     call  checkpixel
     test  ecx,ecx
     pop   ecx
     jnz   .exit
.forced:
     cmp   [ScreenWidth], eax
     jb    .exit
     cmp   [ScreenHeight], ebx
     jb    .exit
.ok:
; check if negation
     test  ecx,0x01000000
     jz    .noneg
     call  getpixel
     not   ecx
     mov   [esp+32-8],ecx
.noneg:
; OK to set pixel
     call  dword [PUTPIXEL]    ; call the real put_pixel function
.exit:
     popad
     ret

align 4
Vesa20_putpixel24:
; eax = x
; ebx = y
     imul    ebx, [BytesPerScanLine]     ; ebx = y * y multiplier
     lea     edi, [eax+eax*2]  ; edi = x*3
     mov     eax, [esp+32-8+4]
     add     edi, [LFBAddress]
     add     edi, ebx          ; ebx = where to put pixel in memory
     mov     [edi], ax
     shr     eax, 16
     mov     [edi+2], al
     ret


align 4
Vesa20_putpixel32:
; eax = x
; ebx = y
     imul    ebx, [BytesPerScanLine]     ; ebx = y * y multiplier
     lea     edi, [ebx+eax*4]  ; edi = x*4+(y*y multiplier)
     mov     eax, [esp+32-8+4] ; eax = color
     add     edi, [LFBAddress]     ; ebx = where to put pixel in memory
     mov     [edi], eax
     ret

;*************************************************

;align 4
calculate_edi:
     mov     edi, ebx
     imul    edi, [ScreenWidth]
     add     edi, ebx
     add     edi, eax
     ret

;*************************************************

; DRAWLINE

align 4
__sys_draw_line:
;     inc   [mouse_pause]
     call    [disable_mouse]

; draw a line
; eax = HIWORD = x1
;       LOWORD = x2
; ebx = HIWORD = y1
;       LOWORD = y2
; ecx = color
; edi = force ?
        pusha

dl_x1 equ esp+20
dl_y1 equ esp+16
dl_x2 equ esp+12
dl_y2 equ esp+8
dl_dx equ esp+4
dl_dy equ esp+0

     xor     edx, edx      ; clear edx
     xor     esi, esi      ; unpack arguments
     xor     ebp, ebp
     mov     si, ax        ; esi = x2
     mov     bp, bx        ; ebp = y2
     shr     eax, 16       ; eax = x1
     shr     ebx, 16       ; ebx = y1
     push    eax           ; save x1
     push    ebx           ; save y1
     push    esi           ; save x2
     push    ebp           ; save y2
; checking x-axis...
     sub     esi, eax      ; esi = x2-x1
     push    esi           ; save y2-y1
     jl      .x2lx1        ; is x2 less than x1 ?
     jg      .no_vline     ; x1 > x2 ?
     mov     edx, ebp      ; else (if x1=x2)
     call    vline
     push    edx    ; necessary to rightly restore stack frame at .exit
     jmp     .exit
.x2lx1:
     neg     esi            ; get esi absolute value
.no_vline:
; checking y-axis...
     sub     ebp, ebx       ; ebp = y2-y1
     push    ebp            ; save y2-y1
     jl      .y2ly1         ; is y2 less than y1 ?
     jg      .no_hline      ; y1 > y2 ?
     mov     edx, [dl_x2]   ; else (if y1=y2)
     call    hline
     jmp     .exit

.y2ly1:
     neg     ebp            ; get ebp absolute value
.no_hline:
     cmp     ebp, esi
     jle     .x_rules       ; |y2-y1| < |x2-x1|  ?
     cmp     [dl_y2], ebx   ; make sure y1 is at the begining
     jge     .no_reverse1
     neg     dword [dl_dx]
     mov     edx, [dl_x2]
     mov     [dl_x2], eax
     mov     [dl_x1], edx
     mov     edx, [dl_y2]
     mov     [dl_y2], ebx
     mov     [dl_y1], edx
.no_reverse1:
     mov     eax, [dl_dx]
     cdq                    ; extend eax sing to edx
     shl     eax, 16        ; using 16bit fix-point maths
     idiv    ebp            ; eax = ((x2-x1)*65536)/(y2-y1)
     mov     edx, ebp       ; edx = counter (number of pixels to draw)
     mov     ebp, 1 *65536  ; <<16   ; ebp = dy = 1.0
     mov     esi, eax       ; esi = dx
     jmp     .y_rules

.x_rules:
     cmp     [dl_x2], eax    ; make sure x1 is at the begining
     jge     .no_reverse2
     neg     dword [dl_dy]
     mov     edx, [dl_x2]
     mov     [dl_x2], eax
     mov     [dl_x1], edx
     mov     edx, [dl_y2]
     mov     [dl_y2], ebx
     mov     [dl_y1], edx
.no_reverse2:
     xor     edx, edx
     mov     eax, [dl_dy]
     cdq                    ; extend eax sing to edx
     shl     eax, 16        ; using 16bit fix-point maths
     idiv    esi            ; eax = ((y2-y1)*65536)/(x2-x1)
     mov     edx, esi       ; edx = counter (number of pixels to draw)
     mov     esi, 1 *65536  ;<< 16   ; esi = dx = 1.0
     mov     ebp, eax       ; ebp = dy
.y_rules:
     mov     eax, [dl_x1]
     mov     ebx, [dl_y1]
     shl     eax, 16
     shl     ebx, 16
align 4
.draw:
     push    eax ebx
     shr     eax, 16
     shr     ebx, 16
     call    [putpixel]
     pop     ebx eax
     add     ebx, ebp        ; y = y+dy
     add     eax, esi        ; x = x+dx
     dec     edx
     jnz     .draw
; force last drawn pixel to be at (x2,y2)
     mov     eax, [dl_x2]
     mov     ebx, [dl_y2]
     call    [putpixel]
.exit:
     add     esp, 6*4
     popa
;     dec   [mouse_pause]
     call   [draw_pointer]
     ret


hline:
; draw an horizontal line
; eax = x1
; edx = x2
; ebx = y
; ecx = color
; edi = force ?
     push    eax edx
     cmp     edx, eax      ; make sure x2 is above x1
     jge     @f
     xchg    eax, edx
align   4
@@:
     call    [putpixel]
     inc     eax
     cmp     eax, edx
     jle     @b
     pop     edx eax
     ret


vline:
; draw a vertical line
; eax = x
; ebx = y1
; edx = y2
; ecx = color
; edi = force ?
     push    ebx edx
     cmp     edx, ebx      ; make sure y2 is above y1
     jge     @f
     xchg    ebx, edx
align   4
@@:
     call    [putpixel]
     inc     ebx
     cmp     ebx, edx
     jle     @b
     pop     edx ebx
     ret


;*************************************************


virtual at esp
drbar:
     .bar_sx       dd ?
     .bar_sy       dd ?
     .bar_cx       dd ?
     .bar_cy       dd ?
     .abs_cx       dd ?
     .abs_cy       dd ?
     .real_sx      dd ?
     .real_sy      dd ?
     .color        dd ?
     .line_inc_scr dd ?
     .line_inc_map dd ?
     .stack_data = 4*11
end virtual

align 4
; eax   cx
; ebx   cy
; ecx   xe
; edx   ye
; edi   color
vesa20_drawbar:
     pushad
     call    [disable_mouse]
     sub     esp, drbar.stack_data
     mov     [drbar.color], edi
     sub     edx, ebx
     jle     .exit          ;// mike.dld, 2005-01-29
     sub     ecx, eax
     jle     .exit          ;// mike.dld, 2005-01-29
     mov     [drbar.bar_sy], edx
     mov     [drbar.bar_sx], ecx
     mov     [drbar.bar_cx], eax
     mov     [drbar.bar_cy], ebx
     mov     edi, [TASK_BASE]
     add     eax, [edi-twdw + WDATA.box.left] ; win_cx
     add     ebx, [edi-twdw + WDATA.box.top] ; win_cy
     mov     [drbar.abs_cx], eax
     mov     [drbar.abs_cy], ebx
; real_sx = MIN(wnd_sx-bar_cx, bar_sx);
     mov     ebx, [edi-twdw + WDATA.box.width] ; ebx = wnd_sx
; \begin{diamond}[20.08.2006]
; note that WDATA.box.width is one pixel less than real window x-size
     inc     ebx
; \end{diamond}[20.08.2006]
     sub     ebx, [drbar.bar_cx]
     ja      @f
.exit:                       ;// mike.dld, 2005-01-29
     add     esp, drbar.stack_data
     popad
     xor     eax, eax
     inc     eax
     ret
@@:
     cmp     ebx, [drbar.bar_sx]
     jbe     .end_x
     mov     ebx, [drbar.bar_sx]
.end_x:
     mov     [drbar.real_sx], ebx
; real_sy = MIN(wnd_sy-bar_cy, bar_sy);
     mov     ebx, [edi-twdw + WDATA.box.height] ; ebx = wnd_sy
; \begin{diamond}[20.08.2006]
     inc     ebx
; \end{diamond}
     sub     ebx, [drbar.bar_cy]
     ja      @f
     add     esp, drbar.stack_data
     popad
     xor     eax, eax
     inc     eax
     ret
@@:
     cmp     ebx, [drbar.bar_sy]
     jbe     .end_y
     mov     ebx, [drbar.bar_sy]
.end_y:
     mov     [drbar.real_sy], ebx
; line_inc_map
     mov     eax, [ScreenWidth]
     sub     eax, [drbar.real_sx]
     inc     eax
     mov     [drbar.line_inc_map], eax
; line_inc_scr
     mov     eax, [drbar.real_sx]
     movzx   ebx, byte [ScreenBPP]
     shr     ebx, 3
     imul    eax, ebx
     neg     eax
     add     eax, [BytesPerScanLine]
     mov     [drbar.line_inc_scr], eax
; pointer to screen
     mov     edx, [drbar.abs_cy]
     imul    edx, [BytesPerScanLine]
     mov     eax, [drbar.abs_cx]
;     movzx   ebx, byte [ScreenBPP]
;     shr     ebx, 3
     imul    eax, ebx
     add     edx, eax
     add     edx, [LFBAddress]
; pointer to pixel map
     mov     eax, [drbar.abs_cy]
     imul    eax, [ScreenWidth]
     add     eax, [drbar.abs_cy]
     add     eax, [drbar.abs_cx]
     add     eax, WinMapAddress
     xchg    eax, ebp
; get process number
     mov     ebx, [CURRENT_TASK]
     cmp     byte [ScreenBPP], 24
     jne     draw_bar_end_32
draw_bar_end_24:
     mov     eax, [drbar.color]    ;; BBGGRR00
     mov     bh, al                ;; bh  = BB
     shr     eax, 8                ;; eax = RRGG
; eax - color high   RRGG
; bl - process num
; bh - color low    BB
; ecx - temp
; edx - pointer to screen
; esi - counter
; edi - counter
     mov     esi, [drbar.real_sy]
align   4
.new_y:
     mov     edi, [drbar.real_sx]
align   4
.new_x:
     cmp     byte [ebp], bl
     jne     .skip
     mov     [edx], bh
     mov     [edx + 1], ax
.skip:
; add pixel
     add     edx, 3
     inc     ebp
     dec     edi
     jnz     .new_x
; add line
     add     edx, [drbar.line_inc_scr]
     add     ebp, [drbar.line_inc_map]
; <Ivan 15.10.04> drawing gradient bars
     test    eax, 0x00800000
     jz      @f
     test    bh, bh
     jz      @f
     dec     bh
@@:
; </Ivan 15.10.04>
     dec     esi
     jnz     .new_y
     add     esp, drbar.stack_data
     popad
     xor     eax, eax
     ret

draw_bar_end_32:
     mov     eax, [drbar.color]    ;; BBGGRR00
     mov     esi, [drbar.real_sy]
align   4
.new_y:
     mov     edi, [drbar.real_sx]
align   4
.new_x:
     cmp     byte [ebp], bl
     jne     .skip
     mov     [edx], eax
.skip:
; add pixel
     add     edx, 4
     inc     ebp
     dec     edi
     jnz     .new_x
; add line
     add     edx, [drbar.line_inc_scr]
     add     ebp, [drbar.line_inc_map]
; <Ivan 15.10.04> drawing gradient bars
     test    eax, 0x80000000
     jz      @f
     test    al, al
     jz      @f
     dec     al
@@:
; </Ivan 15.10.04>
     dec     esi
     jnz     .new_y
     add     esp, drbar.stack_data
     popad
     call    VGA_draw_bar
     xor     eax, eax
     mov     [EGA_counter],1
     ret

;voodoodbcplimit:

; ebp:=(y+Ywin)*(ScreenXSize+1)+(x+Xwin)+AddrBuffer


;     pusha

;     xor edx,edx
;     mov eax,ebp
;     mov ebx,[ScreenWidth] ; Screen_X_size
;     inc ebx   ; +1
;     sub eax,WinMapAddress ; -AddrBuffer
;     div ebx ;
;     mov ebx,eax ; ebx:=Y
;     mov eax,edx ; eax:=X
;     call cplimit

;     test ecx,ecx
;     jne  dbcpl12
;     popa
;     clc
;     ret
;   dbcpl12:
;     popa
;     stc
;     ret




;dbcplimit:

;        pusha

;        xor  edx,edx
;        mov  ebx,[ScreenWidth]
;        inc  ebx
;        sub  eax,WinMapAddress
;        div  ebx
;        mov  ebx,eax
;        mov  eax,edx
;        call cplimit

;        test ecx,ecx
;        jne  dbcpl1
;        popa
;        clc
;        ret
;     dbcpl1:
;        popa
;        stc
;        ret


vesa20_drawbackground_tiled:
        call    [disable_mouse]
        pushad
; External loop for all y from start to end
        mov     ebx, [draw_data+32+RECT.top]    ; y start
dp2:
        mov     ebp, [draw_data+32+RECT.left]   ; x start
; 1) Calculate pointers in WinMapAddress (does pixel belong to OS thread?) [ebp]
;                       and LFB data (output for our function) [edi]
        mov     eax, [BytesPerScanLine]
        mul     ebx
        xchg    ebp, eax
        add     ebp, eax
        add     ebp, eax
        add     ebp, eax
        cmp     [ScreenBPP], byte 24    ; 24 or 32 bpp ? - x size
        jz      @f
        add     ebp, eax
@@:
        add     ebp, [LFBAddress]
; ebp:=Y*BytesPerScanLine+X*BytesPerPixel+AddrLFB
        call    calculate_edi
        xchg    edi, ebp
; Now eax=x, ebx=y, edi->output, ebp=offset in WinMapAddress
; 2) Calculate offset in background memory block
        push    eax
        xor     edx, edx
        mov     eax, ebx
        mov     ecx, [BgrDataHeight]
        div     ecx             ; edx := y mod BgrDataHeight
        sub     ecx, edx        ; ecx := BgrDataHeight - (y mod BgrDataHeight)
        pop     eax
        push    eax
        mov     esi, edx
        imul    esi, [BgrDataWidth]             ; esi := (y mod BgrDataHeight) * BgrDataWidth
        xor     edx, edx
        div     dword [BgrDataWidth]            ; edx := x mod BgrDataWidth
        add     esi, edx        ; esi := (y mod BgrDataHeight)*BgrDataWidth + (x mod BgrDataWidth)
        pop     eax
        lea     esi, [esi*3]
        add     esi, [img_background]
        xor     edx, edx
        inc     edx
; 3) Loop through redraw rectangle and copy background data
; Registers meaning:
; eax = x, ebx = y (screen coordinates)
; ecx = deltax - number of pixels left in current tile block
; edx = 1
; esi -> bgr memory, edi -> output
; ebp = offset in WinMapAddress
dp3:
        cmp     [ebp+WinMapAddress], dl
        jnz     nbgp
        movsb
        movsb
        movsb
        jmp     @f
nbgp:
        add     esi, 3
        add     edi, 3
@@:
        cmp     [ScreenBPP], byte 25    ; 24 or 32 bpp?
        sbb     edi, -1         ; +1 for 32 bpp
; I do not use 'inc eax' because this is slightly slower then 'add eax,1'
        add     ebp, edx
        add     eax, edx
        cmp     eax, [draw_data+32+RECT.right]
        ja      dp4
        sub     ecx, edx
        jnz     dp3
; next tile block on x-axis
        mov     ecx, [BgrDataWidth]
        sub     esi, ecx
        sub     esi, ecx
        sub     esi, ecx
        jmp     dp3
dp4:
; next scan line
        inc     ebx
        cmp     ebx, [draw_data+32+RECT.bottom]
        jbe     dp2
        popad
        mov     [EGA_counter], 1
        call    VGA_drawbackground
        ret

; ----------


vesa20_drawbackground_stretch:
        call    [disable_mouse]
        pushad
; Helper variables
        mov     eax, [BgrDataWidth]
        xor     edx, edx
        mov     ecx, [ScreenWidth]
        inc     ecx
        div     ecx
        push    eax     ; quo
        push    edx     ; rem
        mov     eax, [BgrDataHeight]
        xor     edx, edx
        mov     ecx, [ScreenHeight]
        inc     ecx
        div     ecx
        push    eax
        push    edx
; External loop for all y from start to end
        mov     ebx, [draw_data+32+RECT.top]    ; y start
        mov     ebp, [draw_data+32+RECT.left]   ; x start
; 1) Calculate pointers in WinMapAddress (does pixel belong to OS thread?) [ebp]
;                       and LFB data (output for our function) [edi]
        mov     eax, [BytesPerScanLine]
        mul     ebx
        xchg    ebp, eax
        add     ebp, eax
        add     ebp, eax
        add     ebp, eax
        cmp     [ScreenBPP], byte 24    ; 24 or 32 bpp ? - x size
        jz      @f
        add     ebp, eax
@@:
        add     ebp, [LFBAddress]
; ebp:=Y*BytesPerScanLine+X*BytesPerPixel+AddrLFB
        call    calculate_edi
        xchg    edi, ebp
; Now eax=x, ebx=y, edi->output, ebp=offset in WinMapAddress
        push    ebx
; 2) Calculate offset in background memory block
        push    eax
        mov     eax, ebx
        mul     dword [BgrDataHeight]
        mov     ecx, [ScreenHeight]
        inc     ecx
        div     ecx             ; eax := y * BgrDataHeight / ScreenHeight
                                ; edx := (y * BgrDataHeight) mod ScreenHeight
        mov     esi, eax
        pop     eax
        push    edx             ; dword [esp] = (y * BgrDataHeight) mod ScreenHeight
                                ; dword [esp+4] = y * BgrDataHeight / ScreenHeight
        push    eax
        mov     ecx, [BgrDataWidth]
        lea     edx, [ecx*3]
        imul    edx, [BgrDataHeight]
        add     edx, [img_background]
        push    edx
        mul     ecx
        imul    esi, ecx
        dec     ecx
        push    ecx
        mov     ecx, [ScreenWidth]
        inc     ecx
        div     ecx             ; eax := x * BgrDataWidth / ScreenWidth
                                ; edx := (x * BgrDataWidth) mod ScreenWidth
        add     esi, eax
        lea     esi, [esi*3]
        add     esi, [img_background]
        push    ecx edx esi
; 3) Loop through redraw rectangle and copy background data
; Registers meaning:
; ecx = (x * BgrDataWidth) / ScreenWidth
; edx = (x * BgrDataWidth) mod ScreenWidth (used to fast recalculating of ecx,esi)
; esi -> bgr memory, edi -> output
; ebp = offset in WinMapAddress
; dword [esp] = saved esi
; dword [esp+4] = saved edx
; dword [esp+8] = saved ecx
; dword [esp+12] = BgrDataWidth-1, x-limit for overlapping of points
; dword [esp+16] = end of bgr memory (defines y-limit for overlapping of points)
; dword [esp+20] = x
; dword [esp+24] = (y * BgrDataHeight) mod ScreenHeight (used to fast recalculating of esi)
; dword [esp+28] = y
; precalculated constants:
; dword [esp+32] = BgrDataHeight mod ScreenHeight
; dword [esp+36] = BgrDataHeight div ScreenHeight
; dword [esp+40] = BgrDataWidth mod ScreenWidth
; dword [esp+44] = BgrDataWidth div ScreenWidth
sdp3:
        add     edx, [esp+40]
        cmp     [ebp+WinMapAddress], byte 1
        jnz     snbgp
        mov     al, [esi+2]
        shl     eax, 16
        mov     ax, [esi]
        cmp     ecx, [esp+12]
        jae     @f
        cmp     edx, [ScreenWidth]
        jb      @f
        mov     ebx, [esi+2]
        shr     ebx, 8
        call    overlapping_of_points
@@:
        mov     ebx, [esp+24]
        add     ebx, [esp+32]
        cmp     ebx, [ScreenHeight]
        jbe     @f
        mov     ebx, [BgrDataWidth]
        lea     ebx, [ebx*3]
        add     ebx, esi
        cmp     ebx, [esp+16]
        jae     @f
        mov     ebx, [ebx-1]
        shr     ebx, 8
        call    overlapping_of_points
@@:
        mov     [edi], ax
        shr     eax, 16
        mov     [edi+2], al
snbgp:
        cmp     [ScreenBPP], byte 25
        sbb     edi, -4
        add     ebp, 1
        mov     eax, [esp+20]
        add     eax, 1
        mov     [esp+20], eax
        cmp     eax, [draw_data+32+RECT.right]
        ja      sdp4
        mov     eax, [esp+44]
        add     ecx, eax
        lea     eax, [eax*3]
        add     esi, eax
;        add     edx, [esp+40]
        cmp     edx, [ScreenWidth]
        jbe     sdp3
        sub     edx, [ScreenWidth]
        add     ecx, 1
        add     esi, 3
        sub     edx, 1
        jmp     sdp3
sdp4:
; next y
        mov     ebx, [esp+28]
        add     ebx, 1
        mov     [esp+28], ebx
        cmp     ebx, [draw_data+32+RECT.bottom]
        ja      sdpdone
; advance edi, ebp to next scan line
        sub     eax, [draw_data+32+RECT.left]
        sub     ebp, eax
        add     ebp, [ScreenWidth]
        add     ebp, 1
        sub     edi, eax
        sub     edi, eax
        sub     edi, eax
        cmp     [ScreenBPP], byte 24
        jz      @f
        sub     edi, eax
@@:
        add     edi, [BytesPerScanLine]
; restore ecx,edx; advance esi to next background line
        pop     esi edx ecx
        push    ecx edx
        xor     ebx, ebx
        mov     eax, [esp+24-4]
        add     eax, [esp+32-4]
        cmp     eax, [ScreenHeight]
        jbe     @f
        sub     eax, [ScreenHeight]
        mov     ebx, 1
        sub     eax, ebx
@@:
        mov     [esp+24-4], eax
        add     ebx, [esp+36-4]
        lea     ebx, [ebx*3]
        imul    ebx, [BgrDataWidth]
        add     esi, ebx
        push    esi
        mov     eax, [draw_data+32+RECT.left]
        mov     [esp+20], eax
        jmp     sdp3
sdpdone:
        add     esp, 48
        popad
        mov     [EGA_counter],1
        call    VGA_drawbackground
        ret

overlapping_of_points:
        push    ecx edx edi
        mov     ecx, eax
        mov     edx, ebx
        movzx   eax, cl
        movzx   ebx, dl
        add     eax, ebx
        rcr     eax, 1
        movzx   edi, ax
        movzx   eax, ch
        movzx   ebx, dh
        add     eax, ebx
        rcr     eax, 1
        ror     edi, 8
        add     edi, eax
        shr     ecx, 8
        shr     edx, 8
        movzx   eax, ch
        movzx   ebx, dh
        add     eax, ebx
        rcr     eax, 1
        ror     edi, 8
        add     eax, edi
        ror     eax, 16
        pop     edi edx ecx
        ret
