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
        
        pushad
      
        xor    ecx,ecx
        xor    edx,edx

        ;cli    ; !!!****
        align  4
      mres:

        movzx  eax,word [0xfb4a]
        movzx  ebx,word [0xfb4c]

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

        ;;;push   edi
        mov    edi, 1 ;force
        call   [putpixel]
        ;;;pop    edi

        pop    edx
        pop    ecx

        inc    ecx
        cmp    ecx, 16
        jnz    mres
        xor    ecx, ecx
        inc    edx
        cmp    edx, 24
        jnz    mres
        ;sti    ; !!!****

        popad
        
        ret

save_draw_mouse:
        pushad
        ; save & draw

        mov    [0xfb4a],ax
        mov    [0xfb4c],bx
        push   eax
        push   ebx
        mov    ecx,0
        mov    edx,0

        ;cli ; !!!****
        align  4
      drm:

        push   eax
        push   ebx
        push   ecx
        push   edx

        ; helloworld
        push  ecx
;        push   eax ebx ecx
        add    eax,ecx  ; save picture under mouse
        add    ebx,edx
        push   ecx
        call   getpixel
        mov    [0xfb30],ecx
        pop    ecx
        mov    eax,edx
        shl    eax,6
        shl    ecx,2
        add    eax,ecx
        add    eax,mouseunder
        mov    ebx,[0xfb30]
        mov    [eax],ebx
;        pop    ecx ebx eax
        pop  ecx

        mov    edi,edx       ; y cycle
        shl    edi,4       ; *16 bytes per row
        add    edi,ecx       ; x cycle
        mov    esi, edi
        add    edi, esi
        add    edi, esi       ; *3
        add    edi,[0xf200]      ; we have our str address
        mov    esi, edi
        add    esi, 16*24*3
        push   ecx
        mov    ecx, [0xfb30]
        call   combine_colors
        mov    [0xfb10], ecx
        pop    ecx


        pop    edx
        pop    ecx
        pop    ebx
        pop    eax

        add    eax,ecx       ; we have x coord+cycle
        add    ebx,edx       ; and y coord+cycle

;        push   ecx edi
        push   ecx
        mov    ecx, [0xfb10]
        mov    edi, 1
        call   [putpixel]
        pop    ecx
;        pop    edi ecx

;      mnext:

        mov    ebx,[esp+0]      ; pure y coord again
        mov    eax,[esp+4]      ; and x

        inc    ecx          ; +1 cycle
        cmp    ecx,16       ; if more than 16
        jnz    drm
        xor    ecx, ecx
        inc    edx
        cmp    edx,24
        jnz    drm

        pop    ebx
        pop    eax

        ;sti ; !!!****
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
;      xor eax, eax
;      xor ebx, ebx
;      mov al, [edi+0]
;      mov bl, [esi+0]
    movzx eax, byte [edi+0]
    movzx ebx, byte [esi+0]
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
;      xor eax, eax
;      xor ebx, ebx
;      mov al, [edi+1]
;      mov bl, [esi+1]
    movzx eax, byte [edi+1]
    movzx ebx, byte [esi+1]
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
;      xor eax, eax
;      xor ebx, ebx
;      mov al, [edi+2]
;      mov bl, [esi+2]
    movzx eax, byte [edi+2]
    movzx ebx, byte [esi+2]
      mul ebx
      shr eax, 8
      add ecx, eax

      pop eax
      pop edx
      pop ebx
      pop eax
      ret


__sys_disable_mouse:
      

      cmp  dword [0xf204],dword 0
      je    @f
      ret
    @@:  
;      cli
      pushad

      cmp  [0x3000],dword 1
      je   disable_m

      mov  edx,[0x3000]
      shl  edx,5
      add  edx,window_data

      movzx  eax, word [0xfb0a]
      movzx  ebx, word [0xfb0c]

      mov  ecx,[0xfe00]
      inc  ecx
      imul  ecx,ebx
      add  ecx,eax
      add  ecx, display_data

      movzx eax, byte [edx+twdw+0xe]

      movzx ebx, byte [ecx]
      cmp   eax,ebx
      je    yes_mouse_disable
      movzx ebx, byte [ecx+16]
      cmp   eax,ebx
      je    yes_mouse_disable

;      mov   ebx,[0xfe00]
;      inc   ebx
;      imul  ebx,10
;      add   ecx,ebx
;      movzx ebx, byte [ecx]
;      cmp   eax,ebx
;      je    yes_mouse_disable

      mov   ebx,[0xfe00]
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

      mov  edx,[0x3000]
      shl  edx,5
      add  edx,window_data

      movzx  eax, word [0xfb0a]
      movzx  ebx, word [0xfb0c]

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
      cmp  dword [0xf204],dword 0
      jne  @f
      cli
       call draw_mouse_under
      sti
;    @@:
      mov  [0xf204],dword 1
;       inc  dword [0xf204]
    @@:
    
    no_mouse_disable:

      popad
;      sti
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
;        cli

        pushad
        cmp    dword [0xf204],dword 0  ; mouse visible ?
        je     chms00

        mov     [0xf204], dword 0
        movzx  ebx,word [0xfb0c]
        movzx  eax,word [0xfb0a]
        cli
        call   save_draw_mouse
        sti
      nodmu2:
        popad
        
        ret

      chms00:

        movzx  ecx,word [0xfb4a]
        movzx  edx,word [0xfb4c]

        movzx  ebx,word [0xfb0c]
        movzx  eax,word [0xfb0a]

        cmp    eax,ecx
        jne    redrawmouse

        cmp    ebx,edx
        jne    redrawmouse

        jmp    nodmp

      redrawmouse:
        cli
        call   draw_mouse_under
        call   save_draw_mouse
        sti
    nodmp:
        popad
;        sti
        ret
