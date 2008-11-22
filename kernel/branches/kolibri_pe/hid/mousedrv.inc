;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


;   check mouse
;
;
;   FB00  ->   FB0F   mouse memory 00 chunk count - FB0A-B x - FB0C-D y
;   FB10  ->   FB17   mouse color mem
;   FB21              x move
;   FB22              y move
;   FB30              color temp
;   FB28              high bits temp

uglobal
  mousecount  dd  0x0
  mousedata   dd  0x0
endg

iglobal
mouse_delay         dd 10
mouse_speed_factor: dd 3
mouse_timer_ticks   dd 0
endg

align 4
draw_mouse_under:
	; return old picture

	cmp [set_hw_cursor], 0
	jz @F
	pushad
    mov  eax, [x_under]
    mov  ebx, [y_under]
	stdcall [hw_restore], eax, ebx
	popad
	ret
@@:
	pushad
	xor    ecx,ecx
	xor    edx,edx
	align  4
mres:
    mov eax, [x_under]
    mov ebx, [y_under]
	add    eax,ecx
	add    ebx,edx
	push   ecx
	push   edx
	push   eax
	push   ebx
	mov    eax,edx
	shl    eax,6
	shl    ecx,2
	add    eax,ecx
	add    eax,mouseunder
	mov    ecx,[eax]
	pop    ebx
	pop    eax
	mov    edi, 1 ;force
	call   [putpixel]
	pop    edx
	pop    ecx
	inc    ecx
	cmp    ecx, 16
	jnz    mres
	xor    ecx, ecx
	inc    edx
	cmp    edx, 24
	jnz    mres
	popad
	ret

save_draw_mouse:

	cmp [set_hw_cursor], 0
	je .no_hw_cursor
	pushad

    mov    [x_under], eax
    mov    [y_under], ebx
	movzx  eax,word [MOUSE_Y]
	movzx  ebx,word [MOUSE_X]
	push eax
	push ebx

	mov ecx, [Screen_Max_X]
	inc ecx
	mul ecx
        add eax, [_display_data]
        movzx edx, byte [ebx+eax]
	shl edx, 8
        mov esi, [edx+SLOT_BASE+APPDATA.cursor]

        cmp esi, [current_cursor]
	je .draw

       ; cmp [esi+CURSOR.magic], 'CURS'
       ; jne .fail

        push esi
	call [select_hw_cursor]
        mov [current_cursor], esi
.draw:
        stdcall [set_hw_cursor], esi
	popad
	ret
.fail:
	mov ecx, [def_cursor]
	mov [edx+SLOT_BASE+APPDATA.cursor], ecx
        stdcall [set_hw_cursor], ecx            ; stdcall: [esp]=ebx,eax
	popad
	ret

.no_hw_cursor:
	pushad
	; save & draw
    mov    [x_under], eax
    mov    [y_under], ebx
	push   eax
	push   ebx
	mov    ecx,0
	mov    edx,0
	align  4
drm:
	push   eax
	push   ebx
	push   ecx
	push   edx
	; helloworld
	push  ecx
	add    eax,ecx	; save picture under mouse
	add    ebx,edx
	push   ecx
	call   getpixel
    mov    [color_temp],ecx
	pop    ecx
	mov    eax,edx
	shl    eax,6
	shl    ecx,2
	add    eax,ecx
	add    eax,mouseunder
    mov    ebx,[color_temp]
	mov    [eax],ebx
	pop  ecx
    mov    edi,edx            ; y cycle
    shl    edi,4              ; *16 bytes per row
    add    edi,ecx            ; x cycle
    lea edi, [mousepointer+edi+edi*2]  ; we have our str address
	mov    esi, edi
	add    esi, 16*24*3
	push   ecx
    mov    ecx, [color_temp]
	call   combine_colors
    mov    [mouse_color_mem], ecx
	pop    ecx
	pop    edx
	pop    ecx
	pop    ebx
	pop    eax
	add    eax,ecx	     ; we have x coord+cycle
	add    ebx,edx	     ; and y coord+cycle
	push   ecx
    mov    ecx, [mouse_color_mem]
	mov    edi, 1
	call   [putpixel]
	pop    ecx
	mov    ebx,[esp+0]	; pure y coord again
	mov    eax,[esp+4]	; and x
    inc    ecx          ; +1 cycle
	cmp    ecx,16	    ; if more than 16
	jnz    drm
	xor    ecx, ecx
	inc    edx
	cmp    edx,24
	jnz    drm
	add   esp,8
	popad
	ret


combine_colors:
      ; in
      ; ecx - color ( 00 RR GG BB )
      ; edi - ref to new color byte
      ; esi - ref to alpha byte
      ;
      ; out
      ; ecx - new color ( roughly (ecx*[esi]>>8)+([edi]*[esi]>>8) )
      push eax
      push ebx
      push edx
      push ecx
      xor ecx, ecx
	 ; byte 2
      mov eax, 0xff
      sub al, [esi+0]
      mov ebx, [esp]
      shr ebx, 16
      and ebx, 0xff
      mul ebx
      shr eax, 8
      add ecx, eax
      xor eax, eax
      xor ebx, ebx
      mov al, [edi+0]
      mov bl, [esi+0]
      mul ebx
      shr eax, 8
      add ecx, eax
      shl ecx, 8
	 ; byte 1
      mov eax, 0xff
      sub al, [esi+1]
      mov ebx, [esp]
      shr ebx, 8
      and ebx, 0xff
      mul ebx
      shr eax, 8
      add ecx, eax
      xor eax, eax
      xor ebx, ebx
      mov al, [edi+1]
      mov bl, [esi+1]
      mul ebx
      shr eax, 8
      add ecx, eax
      shl ecx, 8
	 ; byte 2
      mov eax, 0xff
      sub al, [esi+2]
      mov ebx, [esp]
      and ebx, 0xff
      mul ebx
      shr eax, 8
      add ecx, eax
      xor eax, eax
      xor ebx, ebx
      mov al, [edi+2]
      mov bl, [esi+2]
      mul ebx
      shr eax, 8
      add ecx, eax
      pop eax
      pop edx
      pop ebx
      pop eax
      ret


__sys_disable_mouse:
      cmp  [MOUSE_VISIBLE], 0
      je    @f
      ret
@@:
      pushad
      cmp  [CURRENT_TASK],dword 1
      je   disable_m
      mov  edx,[CURRENT_TASK]
      shl  edx,5
      add  edx,window_data
      movzx  eax, word [MOUSE_X]
      movzx  ebx, word [MOUSE_Y]
      mov  ecx,[Screen_Max_X]
      inc  ecx
      imul  ecx,ebx
      add  ecx,eax
      add  ecx, [_display_data]
      mov   eax, [CURRENT_TASK]
      movzx ebx, byte [ecx]
      cmp   eax,ebx
      je    yes_mouse_disable
      movzx ebx, byte [ecx+16]
      cmp   eax,ebx
      je    yes_mouse_disable
      mov   ebx,[Screen_Max_X]
      inc   ebx
      imul  ebx,10
      add   ecx,ebx
      movzx ebx, byte [ecx]
      cmp   eax,ebx
      je    yes_mouse_disable
      movzx ebx, byte [ecx+16]
      cmp   eax,ebx
      je    yes_mouse_disable
      jmp   no_mouse_disable
yes_mouse_disable:
      mov  edx,[CURRENT_TASK]
      shl  edx,5
      add  edx,window_data
      movzx  eax, word [MOUSE_X]
      movzx  ebx, word [MOUSE_Y]
      mov  ecx,[edx+0]	 ; mouse inside the area ?
      add  eax,10
      cmp  eax,ecx
      jb   no_mouse_disable
      sub  eax,10
      add  ecx,[edx+8]
      cmp  eax,ecx
      jg   no_mouse_disable
      mov  ecx,[edx+4]
      add  ebx,14
      cmp  ebx,ecx
      jb   no_mouse_disable
      sub  ebx,14
      add  ecx,[edx+12]
      cmp  ebx,ecx
      jg   no_mouse_disable
disable_m:
      cmp  [MOUSE_VISIBLE], 0
      jne  no_mouse_disable
      pushf
      cli
      call draw_mouse_under
      popf
      mov  [MOUSE_VISIBLE], 1
no_mouse_disable:
      popad
      ret

__sys_draw_pointer:
	cmp   [mouse_pause],0
	je    @f
	ret
@@:
	push   eax
	mov	eax,[timer_ticks]
	sub	eax,[MouseTickCounter]
	cmp	eax,1
	ja	@f
	pop    eax
	ret
@@:
	mov	eax,[timer_ticks]
	mov	[MouseTickCounter],eax
	pop	eax
	pushad
    cmp    [MOUSE_VISIBLE], 0  ; mouse visible ?
	je     chms00
    mov [MOUSE_VISIBLE], 0
	movzx  ebx,word [MOUSE_Y]
	movzx  eax,word [MOUSE_X]
	pushfd
	cli
	call   save_draw_mouse
	popfd
nodmu2:
	popad
	ret
chms00:
    mov  ecx, [x_under]
    mov  edx, [y_under]
	movzx  ebx,word [MOUSE_Y]
	movzx  eax,word [MOUSE_X]
	cmp    eax,ecx
	jne    redrawmouse
	cmp    ebx,edx
	jne    redrawmouse
	jmp    nodmp
redrawmouse:
	pushfd
	cli
	call   draw_mouse_under
	call   save_draw_mouse
	popfd
nodmp:
	popad
	ret

proc set_mouse_data stdcall, BtnState:dword, XMoving:dword, YMoving:dword, VScroll:dword, HScroll:dword

    mov  eax,[BtnState]
    mov  [btn_down],eax

    mov  eax,[XMoving]
    call mouse_acceleration
    add  ax,[MOUSE_X]	 ;[XCoordinate]
    cmp  ax,0
    jge  @@M1
    mov  eax,0
    jmp  @@M2
@@M1:
    cmp  ax, word [Screen_Max_X]   ;ScreenLength
    jl	 @@M2
    mov  ax, word [Screen_Max_X]   ;ScreenLength-1

@@M2:
    mov  [MOUSE_X],ax	  ;[XCoordinate]

    mov  eax,[YMoving]
    neg  eax
    call mouse_acceleration

    add  ax,[MOUSE_Y]	;[YCoordinate]
    cmp  ax,0
    jge  @@M3
    mov  ax,0
    jmp  @@M4
@@M3:
    cmp  ax, word [Screen_Max_Y]  ;ScreenHeigth
    jl	 @@M4
    mov  ax, word [Screen_Max_Y] ;ScreenHeigth-1

@@M4:
    mov  [MOUSE_Y],ax	  ;[YCoordinate]

    mov  eax,[VScroll]
    add  [mouse_scroll_v], eax

    mov  eax,[HScroll]
    add  [mouse_scroll_h], eax

    mov  [mouse_active],1
    mov  eax,[timer_ticks]
    mov  [mouse_timer_ticks],eax
    ret
endp

mouse_acceleration:
	push  eax
	mov   eax,[timer_ticks]
	sub   eax,[mouse_timer_ticks]
	cmp   eax,[mouse_delay]
	pop   eax
	ja    @f
	;push  edx
	imul  eax,[mouse_speed_factor]
	;pop   edx
@@:
	ret

