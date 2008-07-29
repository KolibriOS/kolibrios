;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


get_titlebar_height: ; edi = window draw_data pointer
	mov	al,[edi+WDATA.fl_wstyle]
	and	al,0x0F
	cmp	al,0x03
	jne	@f
	mov	eax,[_skinh]
	ret
    @@: mov	eax,21
	ret

get_rolledup_height: ; edi = window draw_data pointer
	mov	al,[edi+WDATA.fl_wstyle]
	and	al,0x0F
	cmp	al,0x03
	jne	@f
	mov	eax,[_skinh]
	add	eax,3
	ret
    @@: or	al,al
	jnz	@f
	mov	eax,21
	ret
    @@: mov	eax,21+2
	ret


setwindowdefaults:
	pushad

	xor   eax,eax
	mov   ecx,WIN_STACK
       @@:
	inc   eax
	add   ecx,2
	mov   [ecx+0x000],ax	      ; process no
	mov   [ecx+0x400],ax	      ; positions in stack
	cmp   ecx,WIN_POS-2	       ; the more high, the more surface
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

	push	edx ecx ebx eax

	mov	esi, 1
	call	setscreen

	mov	ebp, [TASK_COUNT]	 ; number of processes
	cmp	ebp, 1
	jbe	.finish
	align 4
      .new_wnd:
	movzx	edi, word [WIN_POS + esi * 2]
	shl	edi, 5

	cmp	[CURRENT_TASK+edi+TASKDATA.state], byte 9
	je	.not_wnd

	add	edi, window_data
	test	[edi+WDATA.fl_wstate], WSTATE_MINIMIZED
	jnz	.not_wnd

	mov	eax,[edi+WDATA.box.left]
	cmp	eax, [esp+RECT.right]
	ja	.out_of_bounds
	mov	ebx,[edi+WDATA.box.top]
	cmp	ebx, [esp+RECT.bottom]
	ja	.out_of_bounds
	mov	ecx,[edi+WDATA.box.width]
	add	ecx, eax
	cmp	ecx, [esp+RECT.left]
	jb	.out_of_bounds
	mov	edx,[edi+WDATA.box.height]
	add	edx, ebx
	cmp	edx, [esp+RECT.top]
	jb	.out_of_bounds

    cmp     eax, [esp+RECT.left]
	jae	@f
	mov	eax, [esp+RECT.left]
     @@:
    cmp     ebx, [esp+RECT.top]
	jae	@f
	mov	ebx, [esp+RECT.top]
     @@:
    cmp     ecx, [esp+RECT.right]
	jbe	@f
	mov	ecx, [esp+RECT.right]
     @@:
    cmp     edx, [esp+RECT.bottom]
	jbe	@f
	mov	edx, [esp+RECT.bottom]
     @@:

	push	esi
	movzx	esi, word [WIN_POS + esi * 2]
	call	setscreen
	pop	esi

      .not_wnd:
      .out_of_bounds:
	inc	esi
	dec	ebp
	jnz	.new_wnd
      .finish:

    pop     eax ebx ecx edx

	popfd
	popad
ret



virtual at esp
  ff_x	   dd ?
  ff_y	   dd ?
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
; \begin{diamond}[29.08.2006]
	cmp	esi, 1
	jz	@f
	mov	edi, esi
	shl	edi, 5
	cmp	[edi+window_data+WDATA.box.width], 0
	jnz	@f
	cmp	[edi+window_data+WDATA.box.height], 0
	jz	.ret
@@:
; \end{diamond}[29.08.2006]
	mov edi, esi ;;;word [esi*2+WIN_POS]
	shl   edi, 8
	add   edi, SLOT_BASE  ; address of random shaped window area
	cmp   [edi+APPDATA.wnd_shape], dword 0
	jne   .free_form

	; get x&y size
	sub   ecx, eax
	sub   edx, ebx
	inc   ecx
	inc   edx

	; get WinMap start
        mov   edi, [Screen_Max_X] ; screen_sx
	inc   edi
	imul  edi, ebx
	add   edi, eax
        add   edi, [_display_data]

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
        add   edi, [Screen_Max_X]
	inc   edi
	sub   edi, ecx
	dec   edx
	jnz   .new_y
.ret:
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

	push  dword [edi+APPDATA.wnd_shape_scale]  ; push scale first -> for loop

	; get WinMap start  -> ebp
	push  eax
        mov   eax, [Screen_Max_X] ; screen_sx
	inc   eax
	imul  eax, ebx
	add   eax, [esp]
        add   eax, [_display_data]
	mov   ebp, eax

	mov   edi, [edi+APPDATA.wnd_shape]
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
	mov  edx, [window_data+ecx+WDATA.box.top]
	push [window_data+ecx+WDATA.box.width]	    ; for loop - width
	mov  ecx, [window_data+ecx+WDATA.box.left]
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
        add  ebp, [Screen_Max_X]  ; screen.x
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

     test eax, eax	; redraw display
     jnz  dspl0
     test ebx, ebx
     jnz  dspl0
     cmp  [windowtypechanged],dword 1
     jne  dspl00
     mov  [windowtypechanged],dword 0
  redraw_screen_direct:
     mov  [dlx],dword 0
     mov  [dly],dword 0
     mov  eax,[Screen_Max_X]
     mov  [dlxe],eax
     mov  eax,[Screen_Max_Y]
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
     mov  esi,[TASK_BASE]
     add  esi,TASKDATA.mem_start
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
     mov  esi,[TASK_BASE]
     add  esi,TASKDATA.mem_start
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

	cmp	eax,5	    ; get screen workarea
	jne	no_get_workarea
	popad
	mov	eax,[screen_workarea.left-2]
	mov	ax,word[screen_workarea.right]
	mov	[esp+36],eax
	mov	eax,[screen_workarea.top-2]
	mov	ax,word[screen_workarea.bottom]
	mov	[esp+24],eax
	ret
   no_get_workarea:

	cmp	eax,6	    ; set screen workarea
	jne	no_set_workarea
	movsx	eax,word[esp+16+2]
	movsx	ebx,word[esp+16]
	cmp	eax,ebx
	jge	.lp1
        or      eax,eax;[Screen_Max_X]
	jl	@f
	mov	[screen_workarea.left],eax
    @@: cmp     ebx,[Screen_Max_X]
	jg	.lp1
	mov	[screen_workarea.right],ebx
  .lp1: movsx	eax,word[esp+24+2]
	movsx	ebx,word[esp+24]
	cmp	eax,ebx
	jge	.lp2
	or	eax,eax;[0xFE04]
	jl	@f
	mov	[screen_workarea.top],eax
    @@: cmp     ebx,[Screen_Max_Y]
	jg	.lp2
	mov	[screen_workarea.bottom],ebx
  .lp2: call	repos_windows
	mov	eax, 0
	mov	ebx, 0
        mov     ecx, [Screen_Max_X]
        mov     edx, [Screen_Max_Y]
    call    calculatescreen
;    jmp    redraw_screen_direct
    .exit:
	popad
	ret
   no_set_workarea:

	cmp	eax,7	    ; get skin margins
	jne	no_get_skinmargins
	popad
	mov	eax,dword[_skinmargins+0]
	mov	[esp+36],eax
	mov	eax,dword[_skinmargins+4]
	mov	[esp+24],eax
	ret
   no_get_skinmargins:

	cmp	eax,8	    ; set window skin
	jne	no_set_skin
        call    read_skin_file
        mov     [esp+32+36], eax
        test    eax, eax
        jnz     .ret
        xor     eax, eax
        xor     ebx, ebx
        mov     ecx, [Screen_Max_X]
        mov     edx, [Screen_Max_Y]
        call    calculatescreen
	jmp	redraw_screen_direct
.ret:
        popad
        ret
   no_set_skin:

     popad
     ret


repos_windows:
	mov	ecx,[TASK_COUNT]
	mov	edi, OS_BASE+0x20*2
	call    force_redraw_background
	dec	ecx
	jge    @f
	ret
    @@: mov	[edi+WDATA.fl_redraw],1
	test	[edi+WDATA.fl_wstate],WSTATE_MAXIMIZED
	jz	.lp2
	mov	eax,[screen_workarea.left]
	mov	[edi+WDATA.box.left],eax
	sub	eax,[screen_workarea.right]
	neg	eax
	mov	[edi+WDATA.box.width],eax
	mov	eax,[screen_workarea.top]
	mov	[edi+WDATA.box.top],eax
	test	[edi+WDATA.fl_wstate],WSTATE_ROLLEDUP
	jnz	.lp1
	sub	eax,[screen_workarea.bottom]
	neg	eax
	mov	[edi+WDATA.box.height],eax
  .lp1:

	call	set_window_clientbox
	add	edi,0x20
	loop	@b
	ret
  .lp2: mov	eax,[edi+WDATA.box.left]
	add	eax,[edi+WDATA.box.width]
    mov    ebx,[Screen_Max_X]
;    inc    ebx
    cmp    eax,ebx
    jle    .lp4
	mov	eax,[edi+WDATA.box.width]
    sub    eax,ebx
    jle    .lp3
	mov	[edi+WDATA.box.width],ebx
  .lp3: sub	ebx,[edi+WDATA.box.width]
	mov	[edi+WDATA.box.left],ebx
  .lp4: mov	eax,[edi+WDATA.box.top]
	add	eax,[edi+WDATA.box.height]
    mov    ebx,[Screen_Max_Y]
;    inc    ebx
    cmp    eax,ebx
    jle    .lp6
	mov	eax,[edi+WDATA.box.height]
    sub    eax,ebx
    jle    .lp5
	mov	[edi+WDATA.box.height],ebx
  .lp5: sub	ebx,[edi+WDATA.box.height]
	mov	[edi+WDATA.box.top],ebx
  .lp6: jmp	.lp1

uglobal
  common_colours:
     times 128 db 0x0
endg




check_window_position:

    pushad			     ; window inside screen ?

    movsx eax,word [edi+WDATA.box.left]
    movsx ebx,word [edi+WDATA.box.top]
    movsx ecx,word [edi+WDATA.box.width]
    movsx edx,word [edi+WDATA.box.height]

    cmp   ecx,[Screen_Max_X]	      ; check x size
    jbe   x_size_ok
    mov   ecx,[Screen_Max_X]
    mov   [edi+WDATA.box.width],ecx

  x_size_ok:

    cmp   edx,[Screen_Max_Y]	      ; check y size
    jbe   y_size_ok
    mov   edx,[Screen_Max_Y]
    mov   [edi+WDATA.box.height],edx

  y_size_ok:

    cmp   eax,0	      ; check x pos
    jnle  @f
    xor   eax,eax
    mov   [edi+WDATA.box.left],eax
    jmp   x_pos_ok
  @@:
    add   eax,ecx
    cmp   eax,[Screen_Max_X]
    jbe   x_pos_ok
    mov   eax,[Screen_Max_X]
    sub   eax,ecx
    mov   [edi+WDATA.box.left],eax

  x_pos_ok:

    cmp   ebx,0	      ; check x pos
    jnle  @f
    xor   ebx,ebx
    mov   [edi+WDATA.box.top],ebx
    jmp   y_pos_ok
  @@:
    add   ebx,edx
    cmp   ebx,[Screen_Max_Y]
    jbe   y_pos_ok
    mov   ebx,[Screen_Max_Y]
    sub   ebx,edx
    mov   [edi+WDATA.box.top],ebx

  y_pos_ok:

    popad

    ret


uglobal
  new_window_starting dd 0
endg


sys_window_mouse:

    push  eax

    mov   eax,[timer_ticks]
    cmp   [new_window_starting],eax
    jb	  swml1

    mov   [MOUSE_BACKGROUND],byte 0  ; no mouse background
    mov   [DONT_DRAW_MOUSE],byte 0  ; draw mouse

    mov   [new_window_starting],eax

  swml1:

    pop   eax

    ret




drawwindow_I_caption:

	mov   ecx,[edx+WDATA.cl_titlebar]   ; grab bar
	push  ecx
	mov   esi,edx
	mov   edx,[esi+WDATA.box.top]
	add   edx,1
	mov   ebx,[esi+WDATA.box.top]
	add   ebx,21
	mov   eax,[esi+WDATA.box.top]
	add   eax,[esi+WDATA.box.height]
	cmp   ebx,eax
	jb    .wdsizeok
	mov   ebx,eax
  .wdsizeok:
	push  ebx
  .drwi:
	mov   ebx,edx
	shl   ebx,16
	add   ebx,edx
	mov   eax,[esi+WDATA.box.left]
	inc   eax
	shl   eax,16
	add   eax,[esi+WDATA.box.left]
	add   eax,[esi+WDATA.box.width]
	sub   eax,1
	push  edx
	mov   edx,0x80000000
	mov   ecx,[esi+WDATA.cl_titlebar]
	and   ecx,edx
	cmp   ecx,edx
	jnz   .nofa
	mov   ecx,[esi+WDATA.cl_titlebar]
	sub   ecx,0x00040404
	mov   [esi+WDATA.cl_titlebar],ecx
	and   ecx,0x00ffffff
	jmp   .faj
  .nofa:
	mov   ecx,[esi+WDATA.cl_titlebar]
	and   ecx,0x00ffffff
  .faj:
	pop   edx
	mov   edi,0
	call  [draw_line]
	inc   edx
	cmp   edx,[esp]
	jb    .drwi
	add   esp,4
	pop   ecx
	mov   [esi+WDATA.cl_titlebar],ecx

	ret


drawwindow_I:

	pushad
	or	[edx+WDATA.fl_wdrawn], 4

	mov   esi,[edx+WDATA.cl_frames]   ; rectangle
	mov   eax,[edx+WDATA.box.left]
	shl   eax,16
	add   eax,[edx+WDATA.box.left]
	add   eax,[edx+WDATA.box.width]
	mov   ebx,[edx+WDATA.box.top]
	shl   ebx,16
	add   ebx,[edx+WDATA.box.top]
	add   ebx,[edx+WDATA.box.height]
	call  draw_rectangle

	and	[edx+WDATA.fl_wdrawn], not 4
	test	[edx+WDATA.fl_wdrawn], 2
	jz	@f
	call	drawwindowframes2
@@:

	call	drawwindow_I_caption

	mov   edx,[esi+WDATA.box.top]	   ; inside work area
	add   edx,21+5
	mov   ebx,[esi+WDATA.box.top]
	add   ebx,[esi+WDATA.box.height]
	cmp   edx,ebx
	jg    noinside
	mov   eax,1
	mov   ebx,21
	mov   ecx,[esi+WDATA.box.width]
	mov   edx,[esi+WDATA.box.height]
	mov   edi,[esi+WDATA.cl_workarea]
	test  edi,0x40000000
	jnz   noinside
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

	mov   ecx,esi	       ; yb,xb -> yb,xe
	mov   eax, r_eax
	rol   eax, 16
	mov   ebx,r_ebx
	shl   ebx,16
	mov   bx,r_ebx
	xor   edi, edi
	call  [draw_line]

	mov   ebx,r_bx	       ; ye,xb -> ye,xe
	shl   ebx,16
	mov   bx,r_bx
	call  [draw_line]

	mov   ecx,esi	       ; ya,xa -> ye,xa
	mov   eax,r_eax
	shl   eax,16
	mov   ax,r_eax
	mov   ebx,r_ebx
	shl   ebx,16
	mov   bx,r_bx
	mov   edi,0
	call  [draw_line]

	mov   eax,r_ax	     ; ya,xe -> ye,xe
	shl   eax,16
	mov   ax,r_ax
	call  [draw_line]

	popad
	ret


drawwindow_III_caption:

	mov   ecx,[edx+WDATA.cl_titlebar]			; GRAB BAR
	push  ecx
	mov   esi,edx
	mov   edx,[esi+WDATA.box.top]
	add   edx,4
	mov   ebx,[esi+WDATA.box.top]
	add   ebx,20
	mov   eax,[esi+WDATA.box.top]
	add   eax,[esi+WDATA.box.height]
	cmp   ebx,eax
	jb    .wdsizeok
	mov   ebx,eax
  .wdsizeok:
	push  ebx
  .drwi:
	mov   ebx,edx
	shl   ebx,16
	add   ebx,edx
	mov   eax,[esi+WDATA.box.left]
	shl   eax,16
	add   eax,[esi+WDATA.box.left]
	add   eax,[esi+WDATA.box.width]
	add   eax,4*65536-4
	mov   ecx,[esi+WDATA.cl_titlebar]
	test  ecx,0x40000000
	jz    .nofa
	add   ecx,0x040404
  .nofa:
	test  ecx,0x80000000
	jz    .nofa2
	sub   ecx,0x040404
  .nofa2:
	mov   [esi+WDATA.cl_titlebar],ecx
	and   ecx,0xffffff
	xor   edi, edi
	call  [draw_line]
	inc   edx
	cmp   edx,[esp]
	jb    .drwi
	add   esp,4
	pop   ecx
	mov   [esi+WDATA.cl_titlebar],ecx

	ret


drawwindow_III:

	pushad

	mov   edi,edx				   ; RECTANGLE
	mov   eax,[edi+WDATA.box.left]
	shl   eax,16
    mov   ax, word [edi+WDATA.box.left]
    add   ax, word [edi+WDATA.box.width]
	mov   ebx,[edi+WDATA.box.top]
	shl   ebx,16
    mov   bx, word [edi+WDATA.box.top]
    add   bx, word [edi+WDATA.box.height]
	mov   esi,[edi+WDATA.cl_frames]
	shr   esi,1
	and   esi,0x007f7f7f
	push  esi
	or	[edi+WDATA.fl_wdrawn], 4
	call  draw_rectangle
	and	[edi+WDATA.fl_wdrawn], not 4
	test	[edi+WDATA.fl_wdrawn], 2
	jz	@f
	call	drawwindowframes2
@@:
	mov   ecx,3
      dw3l:
	add   eax,1*65536-1
	add   ebx,1*65536-1
	mov   esi,[edi+WDATA.cl_frames]
	call  draw_rectangle
	dec   ecx
	jnz   dw3l
	pop   esi
	add   eax,1*65536-1
	add   ebx,1*65536-1
	call  draw_rectangle

	call	drawwindow_III_caption

	mov   edx,[esi+WDATA.box.top]			    ; WORK AREA
	add   edx,21+5
	mov   ebx,[esi+WDATA.box.top]
	add   ebx,[esi+WDATA.box.height]
	cmp   edx,ebx
	jg    noinside2
	mov   eax,5
	mov   ebx,20
	mov   ecx,[esi+WDATA.box.width]
	mov   edx,[esi+WDATA.box.height]
	sub   ecx,4
	sub   edx,4
	mov   edi,[esi+WDATA.cl_workarea]
	test  edi,0x40000000
	jnz   noinside2
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
	mov   eax, [TASK_COUNT]
	movzx eax, word [WIN_POS + eax*2]
	shl   eax, 5
	add   eax, window_data
	mov   ebx, [eax + WDATA.cl_workarea]
	and   ebx, 0x0f000000
	cmp   ebx, 0x03000000
	je   .set_WDATA_fl_redraw   ; for 3 and 4 style
	cmp   ebx, 0x04000000
	je   .set_WDATA_fl_redraw
	jmp @f
	.set_WDATA_fl_redraw:
	mov   [eax + WDATA.fl_redraw], byte 1
     @@:

	push   esi
      movzx   eax, word [esi] ; ax <- process no
      movzx   eax, word [WIN_STACK+eax*2] ; ax <- position in window stack

	xor   esi, esi	      ; drop others
      waloop:
	cmp   esi, dword [TASK_COUNT]
	jae   wacont
	inc   esi
	lea   edi, [WIN_STACK + esi*2]
	mov   bx, [edi] ; position of the current process
	cmp   bx, ax
	jbe   @f
	dec   bx       ; upper? => drop!
	mov   [edi], bx
      @@:
	jmp   waloop
      wacont:
			    ; set to no 1
	pop   esi	    ;   esi = pointer at 0xC400

      movzx   eax, word [esi]
	mov   bx, [TASK_COUNT]	; number of processes
	mov   [WIN_STACK+eax*2], bx	; this is the last (and the upper)

	; update on screen -window stack
	xor   esi, esi
      waloop2:
	mov   edi, [TASK_COUNT]
	cmp   esi, edi
	jae   wacont2
	inc   esi
	movzx ebx, word [esi*2 + WIN_STACK]
	mov   [ebx*2 + WIN_POS], si
	jmp   waloop2
      wacont2:
	mov   [KEY_COUNT], byte 0	      ; empty keyboard buffer
	mov   [BTN_COUNT], byte 0	      ; empty button buffer
	mov   [MOUSE_SCROLL_H], word 0  ; zero mouse z-index
	mov   [MOUSE_SCROLL_V], word 0  ; zero mouse z-index
	popad
	ret


; check if window is necessary to draw

checkwindowdraw:

	; edi = position in window_data+

	mov   eax, [edi + WDATA.cl_workarea]
	and   eax, 0x0f000000
	cmp   eax, 0x03000000
	je    .return_yes      ; window type 3
	cmp   eax, 0x04000000
	je    .return_yes      ; window type 4

	mov   esi, edi
	sub   esi, window_data
	shr   esi, 5

	; esi = process number

	movzx eax, word [WIN_STACK + esi * 2] ; get value of the curr process
	lea   esi, [WIN_POS + eax * 2]	    ; get address of this process at 0xC400

	push  esi

      .new_check:

	pop   esi
	add   esi, 2
	push  esi

	mov   eax, [TASK_COUNT]
	lea   eax, word [WIN_POS + eax * 2] ; number of the upper window

	cmp   esi, eax
	ja    .all_wnds_to_top

	movzx eax, word [esi]
	shl   eax, 5
	cmp   [CURRENT_TASK + eax + TASKDATA.state], byte 9
	je    .new_check		    ; skip dead windows

	lea   esi, [eax+window_data]

	mov   ebx, [edi+WDATA.box.top]	; y0
	mov   edx, [edi+WDATA.box.height]
	add   edx, ebx	    ; y0e

	mov   ecx, [esi+WDATA.box.top]	; y  ; y check
	cmp   ecx, edx
	jae   .new_check     ; y < y0e
	mov   eax, [esi+WDATA.box.height]
	add   ecx, eax	     ; ye
	cmp   ebx, ecx	     ; y0 >= ye
	ja    .new_check

	mov   eax, [edi+WDATA.box.left]   ; x0
	mov   ecx, [edi+WDATA.box.width]
	add   ecx, eax	     ; x0e

	mov   edx, [esi+WDATA.box.left]   ; x ; x check
	cmp   edx, ecx
	jae   .new_check     ; x < x0e
	mov   ecx, [esi+WDATA.box.width]
	add   edx, ecx
	cmp   eax, edx
	ja    .new_check

	pop   esi
  .return_yes:
	mov   ecx,1   ; overlap some window
	ret

  .all_wnds_to_top:

	pop   esi

	xor   ecx, ecx	     ; passed all windows to top
	ret




waredraw:     ; if redraw necessary at activate

	pushad

	call  checkwindowdraw	   ; draw window on activation ?
	test  ecx, ecx
	jz    .do_not_draw

	popad
	mov   [MOUSE_DOWN], byte 1     ; do draw mouse
	call  windowactivate

	; update screen info
	pushad
	mov   edi, [TASK_COUNT] ; the last process (number)
	movzx esi, word [WIN_POS + edi * 2]
	shl   esi, 5
	add   esi, window_data

	; coordinates of the upper window
    mov   eax, [esi + WDATA.box.left]	; cx
    mov   ebx, [esi + WDATA.box.top]	; cy
    mov   ecx, [esi + WDATA.box.width]	; sx
    mov   edx, [esi + WDATA.box.height] ; sy

	add   ecx, eax	     ; ecx = x_end
	add   edx, ebx	     ; edx = y_end

	mov   edi, [TASK_COUNT]
	movzx esi, word [WIN_POS + edi * 2]
	call  setscreen
	popad

    mov   [edi + WDATA.fl_redraw], 1  ; redraw flag for app
    mov   [MOUSE_DOWN],byte 0		  ; mouse down checks

	ret

     .do_not_draw:

	popad

	call  windowactivate
	mov   [MOUSE_DOWN],byte 0  ; mouse down checks
	mov   [MOUSE_BACKGROUND],byte 0  ; no mouse background
	mov   [DONT_DRAW_MOUSE],byte 0	; draw mouse
	ret


; eax = window number on screen
; corrupts registers and [dl*]
minimize_window:
	movzx eax, word [WIN_POS+eax*2]
	shl   eax, 5
	add   eax, window_data
	test  [eax+WDATA.fl_wstate], WSTATE_MINIMIZED
	jnz   .skip_redrawings
	pushfd
	cli
	or    [eax+WDATA.fl_wstate], WSTATE_MINIMIZED
	mov   edi, eax
	;call  calculatescreen
	mov   eax, [edi+WDATA.box.left]
	mov   [dlx], eax
	mov   ecx, eax
	add   ecx, [edi+WDATA.box.width]
	mov   [dlxe], ecx
	mov   ebx, [edi+WDATA.box.top]
	mov   [dly], ebx
	mov   edx, ebx
	add   edx, [edi+WDATA.box.height]
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
	movzx esi, word [WIN_POS+eax*2]
	mov   edi, esi
	shl   edi, 5
	add   edi, window_data
	test  [edi+WDATA.fl_wstate], WSTATE_MINIMIZED
	jz    .skip_redrawings
	mov   [edi+WDATA.fl_redraw], 1
	and   [edi+WDATA.fl_wstate], not WSTATE_MINIMIZED
	cmp   eax, [TASK_COUNT] ; the uppermost window
	jnz   .no_uppermost
	mov   eax, [edi+WDATA.box.left]
	mov   ebx, [edi+WDATA.box.top]
	mov   ecx, eax
	mov   edx, ebx
	add   ecx, [edi+WDATA.box.width]
	add   edx, [edi+WDATA.box.height]
	call  setscreen
	jmp   .done
.no_uppermost:
	mov   eax, [edi+WDATA.box.left]
	mov   ebx, [edi+WDATA.box.top]
	mov   ecx, eax
	mov   edx, ebx
	add   ecx, [edi+WDATA.box.width]
	add   edx, [edi+WDATA.box.height]
	call  calculatescreen
.done:
	mov   [MOUSE_BACKGROUND],byte 0 ; no mouse under
.skip_redrawings:
	popfd
	ret


iglobal
  window_moving   db 'K : Window - move/resize',13,10,0
  window_moved	  db 'K : Window - done',13,10,0
endg

; check window touch
align 4
checkwindows:
	pushad

	cmp   [window_minimize], 0
	je    .no_minimizing
	mov   eax, [TASK_COUNT]       ; the uppermost window
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

	cmp   [BTN_DOWN],byte 0    ; mouse buttons pressed ?
	jne  .mouse_buttons_pressed
;..................................... start 1/4 : modified by vhanla .................
	mov [bPressedMouseXY_W],0
;..................................... end 1/4 : modified by vhanla ...................
	popad
	ret
    .mouse_buttons_pressed:
;..................................... start 2/4 : modified by vhanla .................
	jmp @f
	bPressedMouseXY_W db 0x0
	@@:
;..................................... end 2/4 : modified by vhanla ...................
	mov   esi,[TASK_COUNT]
	inc   esi

;..................................... start 3/4 : modified by vhanla .................
	push ax
	cmp [bPressedMouseXY_W],0
	jnz @f
	mov [bPressedMouseXY_W],1
	mov ax,[MOUSE_X]
	mov [mx],ax
	mov ax,[MOUSE_Y]
	mov [my],ax
	@@:
	pop ax
;..................................... end 3/4 : modified by vhanla ...................

      cwloop:
	cmp   esi,2
	jb   .exit

	dec   esi
	movzx edi, word [WIN_POS + esi * 2] ; ebx
	shl   edi, 5
	add   edi, window_data
;        mov   edi, ebx
    mov   ecx, [edi + WDATA.box.left]
    mov   edx, [edi + WDATA.box.top]

	mov   eax,ecx
	mov   ebx,edx
	test	[edi+WDATA.fl_wstate],WSTATE_MINIMIZED
	jnz	cwloop

;..................................... start 4/4 : modified by vhanla .................
       movzx  eax, word [mx]; movzx eax,word[MOUSE_X]
       movzx  ebx, word [my]; movzx ebx,word[MOUSE_Y]
;..................................... endt 4/4 : modified by vhanla ..................
	cmp   ecx, eax
	jae   cwloop
	cmp   edx, ebx
	jae   cwloop
    add   ecx, [edi + WDATA.box.width]
    add   edx, [edi + WDATA.box.height]
	cmp   eax, ecx
	jae   cwloop
	cmp   ebx, edx
	jae   cwloop

	pushad
	mov   eax, esi
	mov   ebx, [TASK_COUNT]
	cmp   eax, ebx	    ; is this window active?
	jz    .move_resize_window

	; eax = position in windowing stack
	; redraw must ?
	lea   esi, [WIN_POS + esi * 2]
	call  waredraw
	add   esp, 32

      .exit:
	popad
	ret

   .move_resize_window:    ; MOVE OR RESIZE WINDOW
	popad

	; Check for user enabled fixed window
    mov   edx, [edi + WDATA.cl_titlebar]
	and   edx, 0x0f000000
	cmp   edx, 0x01000000
	jne   .window_move_enabled_for_user
	popad
	ret
      .window_move_enabled_for_user:

    test    [edi+WDATA.fl_wstate],WSTATE_ROLLEDUP
	jnz	.no_resize_2

	mov   [do_resize_from_corner],byte 0   ; resize for skinned window
    mov   edx, [edi + WDATA.cl_workarea]
	and   edx, 0x0f000000
	cmp   edx, 0x00000000						;{test for resized}
	je    .no_resize_2
	cmp   edx, 0x01000000						;{test for resized}
	je    .no_resize_2
	cmp   edx, 0x04000000						;{test for resized}
	je    .no_resize_2
;	jb    .no_resize_2 ; not type 2 wnd

    mov   edx, [edi + WDATA.box.top]
    add   edx, [edi + WDATA.box.height]
	sub   edx, 6	   ; edx = y_end - 6
	cmp   ebx, edx	   ; ebx = mouse_y
	jb    .no_resize_2
	mov   [do_resize_from_corner],byte 1
	jmp   .continue
      .no_resize_2:

	push	eax
	call	get_titlebar_height
    add     eax,[edi + WDATA.box.top]
	cmp	ebx,eax
	pop	eax
	jae	.exit

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

	mov   cl, [BTN_DOWN]	 ; save for shade check
	mov   [do_resize], cl
     no_emulation_righ_button:
    mov   ecx, [edi + WDATA.box.left]
    mov   edx, [edi + WDATA.box.top]

	push  eax ecx edx
	mov   [dlx], ecx      ; save for drawlimits
	mov   [dly], edx
    mov   eax, [edi + WDATA.box.width]
	add   ecx, eax
    mov   eax, [edi + WDATA.box.height]
	add   edx, eax
	mov   [dlxe], ecx
	mov   [dlye], edx
	pop   edx ecx eax

	sub   eax, ecx
	sub   ebx, edx

	mov   esi, [MOUSE_X]
	mov   [WIN_TEMP_XY], esi

	pushad		 ; wait for putimages to finish
;        mov   ebx,5
;        call  delay_hs
    mov   eax,[edi + WDATA.box.left]
	mov   [npx],eax
    mov   eax,[edi + WDATA.box.top]
	mov   [npy],eax
	popad

	push eax		  ; save old coordinates
    mov   ax, word [edi + WDATA.box.left]
	mov   word [oldc+BOX.left],ax
    mov   ax, word [edi + WDATA.box.top]
	mov   word [oldc+BOX.top],ax
    mov   ax, word [edi + WDATA.box.width]
	mov   word [oldc+BOX.width],ax
	mov   word [npxe],ax
    mov   ax, word [edi + WDATA.box.height]
	mov   word [oldc+BOX.height],ax
	mov   word [npye],ax
	pop eax

	call  drawwindowframes

	mov   [reposition],0
	mov   [MOUSE_DOWN],byte 1   ; no reaction to mouse up/down

	; move window

      newchm:

	mov   [DONT_DRAW_MOUSE],byte 1

	call  checkidle

	call  checkVga_N13

	mov   [MOUSE_BACKGROUND],byte 0

	call  [draw_pointer]

	pushad
	call   stack_handler
	popad

	mov   esi,[WIN_TEMP_XY]
	cmp   esi,[MOUSE_X]
	je    cwb

	mov   cx,[MOUSE_X]
	mov   dx,[MOUSE_Y]
	sub   cx,ax
	sub   dx,bx

	push  ax
	push  bx

	call  drawwindowframes

        mov   ax,[Screen_Max_X]
        mov   bx,[Screen_Max_Y]

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
	;shr   edx,5
	;shl   edx,8
	;add   edx,0x80000                 ; process base at 0x80000+
    lea   edx, [SLOT_BASE + edx*8]

	movzx eax,word [MOUSE_X]
    cmp   eax,[edi + WDATA.box.left]
	jb    nnepx
    sub   eax,[edi + WDATA.box.left]
	cmp   eax,32 ; [edx+0x90+8]
	jge   nnepx2
	mov   eax,32 ; [edx+0x90+8]
      nnepx2:
	mov   [npxe],eax
      nnepx:

	call	get_rolledup_height
	mov	ebx,eax
	movzx eax,word [MOUSE_Y]
    cmp   eax,[edi + WDATA.box.top]
	jb    nnepy
    sub   eax,[edi + WDATA.box.top]
	cmp	eax,ebx ; [edx+0x90+12]
	jge	nnepy2
	mov	eax,ebx ; [edx+0x90+12]
      nnepy2:
	mov   [npye],eax
      nnepy:

	mov   [reposition],1

	popad
      norepos_size:

	pop   bx
	pop   ax
	call  drawwindowframes

	mov   esi,[MOUSE_X]
	mov   [WIN_TEMP_XY],esi

      cwb:
	cmp   [BTN_DOWN],byte 0
	jne   newchm
				     ; new position done
	mov	[DONT_DRAW_MOUSE],byte 1
	mov	cl,0
	test	[edi+WDATA.fl_wstate],WSTATE_MAXIMIZED
	jnz	@f
	mov	cl,[reposition]
	call	drawwindowframes

	mov   eax,[npx]
    mov   [edi + WDATA.box.left],eax
	mov   eax,[npy]
    mov   [edi + WDATA.box.top],eax
	mov   eax,[npxe]
    mov   [edi + WDATA.box.width],eax
	mov   eax,[npye]
    mov   [edi + WDATA.box.height],eax
	call	set_window_clientbox

    @@: mov	[reposition],cl

	cmp   [reposition],1	     ; save new position and size
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
	add   edi,SLOT_BASE+APPDATA.saved_box
	cld
	rep   movsd
	pop   ecx edi esi
      no_bounds_save:

	pushad				   ; WINDOW SHADE/FULLSCREEN

	;{doule click not worked for 4 type window}
    mov   edx, [edi + WDATA.cl_workarea]
	and   edx, 0x0f000000
	cmp   edx, 0x00000000
	je    no_fullscreen_restore
	cmp   edx, 0x01000000
	je    no_fullscreen_restore

	cmp   [reposition],1
	je    no_window_sizing
	mov   edx,edi
	sub   edx,window_data
	shr   edx,5
	shl   edx,8
	add   edx,SLOT_BASE		    ; process base at 0x80000+

	cmp   [do_resize],2		  ; window shade ?
	jne   no_window_shade
	mov   [reposition],1

	test	[edi+WDATA.fl_wstate],WSTATE_ROLLEDUP
	jnz	wnd_rolldown
  wnd_rollup:
	or	[edi+WDATA.fl_wstate],WSTATE_ROLLEDUP
	call	get_rolledup_height
	jmp	@f
  wnd_rolldown:
	and	[edi+WDATA.fl_wstate],not WSTATE_ROLLEDUP
    mov     eax,[edx + APPDATA.saved_box.height] ; 0x90+BOX.height
	test	[edi+WDATA.fl_wstate],WSTATE_MAXIMIZED
	jz	@f
	mov	eax,[screen_workarea.bottom]
	sub	eax,[screen_workarea.top]
    @@: mov	[edi+WDATA.box.height],eax
        add     eax, [edi+WDATA.box.top]
        cmp     eax, [Screen_Max_Y]
        jbe     @f
        mov     eax, [Screen_Max_Y]
        sub     eax, [edi+WDATA.box.height]
        mov     [edi+WDATA.box.top], eax
    @@: call    check_window_position
	call	set_window_clientbox

      no_window_shade:

	push edx
	mov   edx, [edi + WDATA.cl_workarea]
	and   edx, 0x0f000000
	cmp   edx, 0x04000000
	pop edx
	je    no_fullscreen_restore

	cmp   [do_resize],1		  ; fullscreen/restore ?
	jne   no_fullscreen_restore
	cmp   [latest_window_touch_delta],dword 50
	jg    no_fullscreen_restore
	mov   [reposition],1
	test	[edi+WDATA.fl_wstate],WSTATE_MAXIMIZED
	jnz	restore_from_fullscreen
	or	[edi+WDATA.fl_wstate],WSTATE_MAXIMIZED
	mov	eax,[screen_workarea.left]
	mov	[edi+WDATA.box.left],eax
	sub	eax,[screen_workarea.right]
	neg	eax
	mov	[edi+WDATA.box.width],eax
	mov	eax,[screen_workarea.top]
	mov	[edi+WDATA.box.top],eax
	test	[edi+WDATA.fl_wstate],WSTATE_ROLLEDUP
	jnz	@f
	sub	eax,[screen_workarea.bottom]
	neg	eax
	mov	[edi+WDATA.box.height],eax
    @@:
	jmp	restore_from_fullscreen.clientbox
      restore_from_fullscreen:
	and	[edi+WDATA.fl_wstate],not WSTATE_MAXIMIZED
	push	[edi+WDATA.box.height]
	push  edi			  ; restore
	lea   esi, [edx + APPDATA.saved_box]
	mov   ecx,4
	cld
	rep   movsd
	pop   edi
	pop	eax
	test	[edi+WDATA.fl_wstate],WSTATE_ROLLEDUP
	jz	@f
	mov	[edi+WDATA.box.height],eax
    @@:
    .clientbox:
	call	set_window_clientbox

      no_fullscreen_restore:

	mov   eax,[edi+WDATA.box.top]		      ; check Y inside screen
	add   eax,[edi+WDATA.box.height]
        cmp   eax,[Screen_Max_Y]
	jbe   no_window_sizing
	mov   eax,[edi+WDATA.box.left]		       ; check X inside screen
	add   eax,[edi+WDATA.box.width]
        cmp   eax,[Screen_Max_X]
	jbe   no_window_sizing
        mov   eax,[Screen_Max_X]
	sub   eax,[edi+WDATA.box.width]
	mov   [edi+WDATA.box.left],eax
        mov   eax,[Screen_Max_Y]
	sub   eax,[edi+WDATA.box.height]
	mov   [edi+WDATA.box.top],eax
	call  set_window_clientbox
      no_window_sizing:

	popad

	cmp   [reposition],0
	je    retwm

	mov   [DONT_DRAW_MOUSE],byte 1 ; no mouse


	push  eax ebx ecx edx
	mov   eax,[edi+WDATA.box.left]
	mov   ebx,[edi+WDATA.box.top]
	mov   ecx,[edi+WDATA.box.width]
	mov   edx,[edi+WDATA.box.height]
	add   ecx,eax
	add   edx,ebx
	call  calculatescreen

	mov   eax,[oldc+BOX.left]
	mov   ebx,[oldc+BOX.top]
	mov   ecx,[oldc+BOX.width]
	mov   edx,[oldc+BOX.height]
	add   ecx,eax
	add   edx,ebx
	call  calculatescreen
	pop   edx ecx ebx eax

	mov   eax,edi
	call  redrawscreen


	mov	[edi+WDATA.fl_redraw],1

	mov   ecx,100	      ; wait to avoid mouse residuals
      waitre2:
	mov   [DONT_DRAW_MOUSE],byte 1
	call  checkidle
	cmp	[edi+WDATA.fl_redraw],0
	jz    retwm
	loop  waitre2

      retwm:

	mov   [DONT_DRAW_MOUSE],byte 0 ; mouse pointer
	mov   [MOUSE_BACKGROUND],byte 0 ; no mouse under
	mov   [MOUSE_DOWN],byte 0 ; react to mouse up/down

	mov    esi,window_moved
	call   sys_msg_board_str

	popad

	ret


uglobal
  add_window_data	     dd  0
  do_resize_from_corner      db  0x0
  reposition		     db  0x0
  latest_window_touch	     dd  0x0
  latest_window_touch_delta  dd  0x0

  do_resize db 0x0

  oldc	  dd 0x0,0x0,0x0,0x0

  dlx	  dd 0x0
  dly	  dd 0x0
  dlxe	  dd 0x0
  dlye	  dd 0x0

  npx	  dd 0x0
  npy	  dd 0x0
  npxe	  dd 0x0
  npye	  dd 0x0

  mpx	  dd 0x0
  mpy	  dd 0x0
endg


; draw negative window frames
drawwindowframes2:
	pushad
	cli
	jmp	drawwindowframes.do
drawwindowframes:
	pushad
	cli

	test	[edi+WDATA.fl_wstate],WSTATE_MAXIMIZED
	jnz	.ret
	mov	eax, [npx]
	cmp	eax, [edi+WDATA.box.left]
	jnz	.nowndframe
	mov	eax, [npxe]
	cmp	eax, [edi+WDATA.box.width]
	jnz	.nowndframe
	mov	eax, [npy]
	cmp	eax, [edi+WDATA.box.top]
	jnz	.nowndframe
	mov	eax, [npye]
	cmp	eax, [edi+WDATA.box.height]
	jnz	.nowndframe
	xor	[edi+WDATA.fl_wdrawn], 2
	test	[edi+WDATA.fl_wdrawn], 4
	jnz	.ret

.nowndframe:
.do:
	mov	edi, 1
	mov	ecx, 0x01000000
	mov   eax,[npx]
	shl   eax,16
	add   eax,[npx]
	add   eax,[npxe]
	add   eax,65536*1-1
	mov   ebx,[npy]
	shl   ebx,16
	add   ebx,[npy]
	call  [draw_line]

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
	call  [draw_line]

	mov   eax,[npx]
	shl   eax,16
	add   eax,[npx]
	mov   ebx,[npy]
	shl   ebx,16
	add   ebx,[npy]
	add   ebx,[npye]
	call  [draw_line]

	mov   eax,[npx]
	add   eax,[npxe]
	shl   eax,16
	add   eax,[npx]
	add   eax,[npxe]
	mov   ebx,[npy]
	shl   ebx,16
	add   ebx,[npy]
	add   ebx,[npye]
	call  [draw_line]

.ret:
	sti
	popad
	ret



random_shaped_window:

;
;  eax = 0    giving address of data area
;      ebx    address
;  eax = 1    shape area scale
;      ebx    2^ebx scale

     test eax, eax
     jne  rsw_no_address
     mov  eax,[current_slot]
     mov  [eax+APPDATA.wnd_shape],ebx
rsw_no_address:

     cmp  eax,1
     jne  rsw_no_scale
     mov  eax,[current_slot]
     mov  byte [eax+APPDATA.wnd_shape_scale], bl
rsw_no_scale:

     ret


