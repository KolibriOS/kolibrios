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
           jz .exit
           pushad
           mov  eax, [x_under]
           mov  ebx, [y_under]
           stdcall [hw_restore], eax, ebx
           popad
.exit:
           ret

save_draw_mouse:

           cmp [set_hw_cursor], 0
           je .exit

           pushad

           mov [x_under], eax
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

           push esi
           call [select_hw_cursor]
           mov [current_cursor], esi
.draw:
           stdcall [set_hw_cursor], esi
           popad
.exit:
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
      movzx ebx, byte [ecx+32]
      cmp   eax,ebx
      je    yes_mouse_disable
      mov   ebx,[Screen_Max_X]
      inc   ebx
      imul  ebx,10
      add   ecx,ebx
      movzx ebx, byte [ecx]
      cmp   eax,ebx
      je    yes_mouse_disable
      movzx ebx, byte [ecx+32]
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


;[ecx]     x
;[ecx+4]   y
;[ecx+8]   w
;[ecx+12]  h

align 4
lock_cursor:

           pushfd
           cli

           xor eax, eax
           mov edx, [ecx]
           mov ebx, [ecx+4]
           cmp edx, [cur.right]
           jg .done

           cmp ebx, [cur.bottom]
           jg .done

           add edx, [ecx+8]
           add ebx, [ecx+12]
           cmp edx, [cur.left]
           jle .done

           cmp ebx, [cur.top]
           jle .done

           mov ecx, [CURRENT_TASK]

           mov ebx, [cur.left]
           mov eax, [cur.top]
           add ebx, [_display_data]
           mul [_screen_width]
           add ebx, eax

           cmp cl, [ebx]
           je .disable

           cmp cl, [ebx+31]
           je .disable

           mov eax, [_screen_width]
           shl eax, 5
           sub eax, [_screen_width]

           cmp cl, [ebx+eax]
           je .disable

           cmp cl, [ebx+eax+31]
           jne .done

.disable:
           call draw_mouse_under

           mov eax, 1
.done:
           inc eax
           mov [cur.lock], eax
           popfd
           ret

align 4
unlock_cursor:
           pushfd
           cli
           cmp [cur.lock], 2
           jne .done

           call save_draw_mouse
.done:
           mov [cur.lock], 1
           popfd
           ret
