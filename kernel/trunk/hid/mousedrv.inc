$Revision$
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

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

mouse_delay     dd  10
mouse_speed_factor dw 3

include 'm_ps2.inc'
include 'm_com1.inc'
include 'm_com2.inc'


;test_mario79:
;        push   esi
;        push   eax
;        mov    [write_error_to],process_test_m79+43
;        movzx  eax,al  ;[DevErrorCode]
;        call   writehex
;        mov    esi,process_test_m79
;        call   sys_msg_board_str
;        pop    eax
;        pop    esi
;        ret
;process_test_m79 db 'K : Process - test Mario79 error    00000000',13,10,0

draw_mouse_under:
        ; return old picture

        cmp [set_hw_cursor], 0
        jz @F
        pushad
        movzx  eax,word [X_UNDER]
        movzx  ebx,word [Y_UNDER]
        stdcall [hw_restore], eax, ebx
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

        cmp [set_hw_cursor], 0
        jz @F
        pushad

        mov    [X_UNDER],ax
        mov    [Y_UNDER],bx
        movzx  eax,word [MOUSE_Y]
        movzx  ebx,word [MOUSE_X]
        push eax
        push ebx

        mov ecx, [ScreenWidth]
        inc ecx
        mul ecx

        movzx edx, byte [display_data+ebx+eax]
        shl edx, 8
        mov ecx, [edx+SLOT_BASE+APPDATA.cursor]

        cmp [ecx+CURSOR.magic], 'CURS'
        jne .fail
;        cmp [ecx+CURSOR.size], CURSOR_SIZE
;        jne .fail
        push ecx
        call [set_hw_cursor]
        popad
        ret
.fail:
        mov ecx, [def_cursor]
        mov [edx+SLOT_BASE+APPDATA.cursor], ecx
        push ecx
        call [set_hw_cursor]
        popad
        ret

@@:
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
        add    eax,ecx  ; save picture under mouse
        add    ebx,edx
        push   ecx
        call   getpixel
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
        mov    edi,edx       ; y cycle
        shl    edi,4       ; *16 bytes per row
        add    edi,ecx       ; x cycle
        mov    esi, edi
        add    edi, esi
        add    edi, esi       ; *3
        add    edi,[MOUSE_PICTURE]      ; we have our str address
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
        add    eax,ecx       ; we have x coord+cycle
        add    ebx,edx       ; and y coord+cycle
        push   ecx
        mov    ecx, [MOUSE_COLOR_MEM]
        mov    edi, 1
        call   [putpixel]
        pop    ecx
        mov    ebx,[esp+0]      ; pure y coord again
        mov    eax,[esp+4]      ; and x
        inc    ecx          ; +1 cycle
        cmp    ecx,16       ; if more than 16
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
      mov  ecx,[ScreenWidth]
      inc  ecx
      imul  ecx,ebx
      add  ecx,eax
      add  ecx, display_data
      mov   eax, [CURRENT_TASK]
      movzx ebx, byte [ecx]
      cmp   eax,ebx
      je    yes_mouse_disable
      movzx ebx, byte [ecx+16]
      cmp   eax,ebx
      je    yes_mouse_disable
      mov   ebx,[ScreenWidth]
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
      mov  ecx,[edx+0]   ; mouse inside the area ?
      add  eax,14
      cmp  eax,ecx
      jb   no_mouse_disable
      sub  eax,14
      add  ecx,[edx+8]
      cmp  eax,ecx
      jg   no_mouse_disable
      mov  ecx,[edx+4]
      add  ebx,20
      cmp  ebx,ecx
      jb   no_mouse_disable
      sub  ebx,20
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
        mov     eax,[timer_ticks]
        sub     eax,[MouseTickCounter]
        cmp     eax,1
        ja      @f
        pop    eax
        ret
@@:
        mov     eax,[timer_ticks]
        mov     [MouseTickCounter],eax
        pop     eax
        pushad
        cmp    dword [MOUSE_VISIBLE],dword 0  ; mouse visible ?
        je     chms00
        mov     [MOUSE_VISIBLE], dword 0
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

