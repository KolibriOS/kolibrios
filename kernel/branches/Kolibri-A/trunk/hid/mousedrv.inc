;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved. ;;
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
;   FB4A  ->   FB4D   FB4A-B x-under - FB4C-D y-under
;   FC00  ->   FCFE   com1/ps2 buffer
;   FCFF              com1/ps2 buffer count starting from FC00

uglobal
  mousecount  dd  0x0
  mousedata   dd  0x0
endg

iglobal
mouse_delay	   dd 10
mouse_speed_factor: dd 3
mouse_timer_ticks  dd 0
endg

;include 'm_com.inc'


draw_mouse_under:
	; return old picture

    cmp [_display.restore_cursor], 0
    je @F

	pushad
	movzx  eax,word [X_UNDER]
	movzx  ebx,word [Y_UNDER]
    stdcall [_display.restore_cursor], eax, ebx
	popad
	ret
@@:
	pushad
	xor    ecx,ecx
	xor    edx,edx
	align  4
mres:
	movzx  eax,word [X_UNDER]
	movzx  ebx,word [Y_UNDER]
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

    cmp [_display.move_cursor], 0
	je .no_hw_cursor
	pushad

	mov    [X_UNDER],ax
	mov    [Y_UNDER],bx
	movzx  eax,word [MOUSE_Y]
	movzx  ebx,word [MOUSE_X]
	push eax
	push ebx
	
	mov	ecx, ebx		; <<<<
	shr	ecx, 1
	imul 	ecx, [_WinMapWidth]
	shr	eax, 1
	shr	eax, 1
	add	eax, ecx
    	add 	eax, [_WinMapAddress]
    	movzx edx, byte [ebx+eax]
	shl edx, 8
        mov esi, [edx+SLOT_BASE+APPDATA.cursor]

        cmp esi, [current_cursor]
	je .draw

        push esi
    call [_display.select_cursor]
        mov [current_cursor], esi
.draw:
    stdcall [_display.move_cursor], esi
	popad
	ret
.fail:
	mov ecx, [def_cursor]
	mov [edx+SLOT_BASE+APPDATA.cursor], ecx
    stdcall [_display.move_cursor], ecx            ; stdcall: [esp]=ebx,eax
	popad
	ret

.no_hw_cursor:
	pushad
	; save & draw
	mov    [X_UNDER],ax
	mov    [Y_UNDER],bx
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
	call   get_pixel
	mov    [COLOR_TEMP],ecx
	pop    ecx
	mov    eax,edx
	shl    eax,6
	shl    ecx,2
	add    eax,ecx
	add    eax,mouseunder
	mov    ebx,[COLOR_TEMP]
	mov    [eax],ebx
	pop  ecx
	mov    edi,edx	     ; y cycle
	shl    edi,4	   ; *16 bytes per row
	add    edi,ecx	     ; x cycle
	mov    esi, edi
	add    edi, esi
	add    edi, esi       ; *3
	add    edi, mousepointer	;[MOUSE_PICTURE]	; we have our str address
	mov    esi, edi
	add    esi, 16*24*3
	push   ecx
	mov    ecx, [COLOR_TEMP]
	call   combine_colors
	mov    [MOUSE_COLOR_MEM], ecx
	pop    ecx
	pop    edx
	pop    ecx
	pop    ebx
	pop    eax
	add    eax,ecx	     ; we have x coord+cycle
	add    ebx,edx	     ; and y coord+cycle
	push   ecx
	mov    ecx, [MOUSE_COLOR_MEM]
	mov    edi, 1
	call   [putpixel]
	pop    ecx
	mov    ebx,[esp+0]	; pure y coord again
	mov    eax,[esp+4]	; and x
	inc    ecx	    ; +1 cycle
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
      cmp  dword [MOUSE_VISIBLE],dword 0
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
	mov	ecx, ebx		; <<<<
	shr	ecx, 1
	imul 	ecx, [_WinMapWidth]
	shr	eax, 1
	shr	eax, 1
	add	ecx, eax
    	add 	ecx, [_WinMapAddress]
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
      cmp  dword [MOUSE_VISIBLE],dword 0
      jne  no_mouse_disable
      pushf
      cli
      call draw_mouse_under
      popf
      mov  [MOUSE_VISIBLE],dword 1
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
	cmp    dword [MOUSE_VISIBLE],dword 0  ; mouse visible ?
	je     chms00
	mov	[MOUSE_VISIBLE], dword 0
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
	movzx  ecx,word [X_UNDER]
	movzx  edx,word [Y_UNDER]
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
    mov  [BTN_DOWN],eax

    mov  eax,[XMoving]
    call mouse_acceleration
    add  ax,[MOUSE_X]	 ;[XCoordinate]
    cmp  ax,0
    jge  @@M1
    mov  eax,0
    jmp  @@M2
@@M1:
    cmp  ax,[Screen_Max_X]   ;ScreenLength
    jl	 @@M2
    mov  ax,[Screen_Max_X]   ;ScreenLength-1

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
    cmp  ax,[Screen_Max_Y]  ;ScreenHeigth
    jl	 @@M4
    mov  ax,[Screen_Max_Y] ;ScreenHeigth-1

@@M4:
    mov  [MOUSE_Y],ax	  ;[YCoordinate]

    mov  eax,[VScroll]
    add  [MOUSE_SCROLL_V],ax

    mov  eax,[HScroll]
    add  [MOUSE_SCROLL_H],ax

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

diff16 "window code end",0,$
diff10 "mousedrv  size ",draw_mouse_under,$
