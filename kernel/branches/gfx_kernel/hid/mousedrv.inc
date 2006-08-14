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

__sys_draw_mouse_under:
        ; return old picture

        pushad

        xor    ecx,ecx
        xor    edx,edx
        mov    esi,mouseunder-4

        align  4
      mres:

        add    esi,4

        movsx  eax,word[0xfb4a]
        add    eax,ecx
        js     .skip
        movsx  ebx,word[0xfb4c]
        add    ebx,edx
        js     .skip

        push   ecx
        push   edx
        mov    ecx,[esi]

        mov    edi,1 ;force
        push   esi
        call   [putpixel]
        pop    esi

        pop    edx
        pop    ecx
    .skip:
        inc    ecx
        cmp    ecx,32

        jnz    mres
        xor    ecx, ecx
        inc    edx
        cmp    edx,32
        jnz    mres

        popad
        ret


save_draw_mouse:
        pushad
        ; save & draw
        mov    [0xfb4a],ax
        mov    [0xfb4c],bx
        push   eax
        push   ebx
        xor    ecx,ecx
        mov    edx,ecx
        mov    esi,mouseunder-4;3
        mov    edi,mousepointer+62-4

        mov    dword[0x6900],mouseunder+mousecomb

      drm:

        add    esi,4;3
        add    edi,4

        push   eax ebx ecx edx

        push   eax ebx
        add    eax,ecx  ; save picture under mouse
        js     @f
        add    ebx,edx
        js     @f
        push   esi edi
        call   [getpixel]
        pop    edi esi
        mov    [esi],ecx
        pop    ebx eax

        push   esi edi
        call   combine_colors
        pop    edi esi
        mov    [0xfb10],ecx

        pop    edx ecx ebx eax

        xchg   esi,[0x6900]
        push   dword[0xFB10]
        pop    dword[esi]
        add    esi,3
        xchg   esi,[0x6900]

        jc     mnext

        add    eax,ecx       ; we have x coord+cycle
        js     mnext
        add    ebx,edx       ; and y coord+cycle
        js     mnext

        push   ecx edi esi
        mov    ecx, [0xfb10]
        mov    edi, 1
        call   [putpixel]
        pop    esi edi ecx
        jmp    mnext
    @@: add    esp,8
        pop    edx ecx ebx eax
      mnext:

        mov    ebx,[esp+0]      ; pure y coord again
        mov    eax,[esp+4]      ; and x

        inc    ecx          ; +1 cycle
        cmp    ecx,32
        jnz    drm
        xor    ecx,ecx
        inc    edx
        cmp    edx,32
        jnz    drm
        pop    ebx
        pop    eax

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
    ; colors:
    ;   [esp] = background:
    ;   [edi] = cursor
    ;   <ecx> = combined

    cmp byte[edi+3],0
    jne @f
    stc
    ret
  @@:
    cmp byte[edi+3],255
    jne @f
    mov ecx,[edi]
    and ecx,0x00FFFFFF
    clc
    ret
  @@:
      push ecx

    xor  ecx,ecx

    movzx eax,byte[edi+2]
    movzx ebx,byte[esp+2]
    sub   eax,ebx
    movzx ebx,byte[edi+3]
    imul  ebx
    xor   edx,edx
    mov   ebx,255
    div   ebx
    add   al,[esp+2]
    mov   cl,al
    shl   ecx,8

    movzx eax,byte[edi+1]
    movzx ebx,byte[esp+1]
    sub   eax,ebx
    movzx ebx,byte[edi+3]
    imul  ebx
    xor   edx,edx
    mov   ebx,255
    div   ebx
    add   al,[esp+1]
    mov   cl,al
    shl   ecx,8

    movzx eax,byte[edi+0]
    movzx ebx,byte[esp+0]
    sub   eax,ebx
    movzx ebx,byte[edi+3]
    imul  ebx
    xor   edx,edx
    mov   ebx,255
    div   ebx
    add   al,[esp+0]
    mov   cl,al

    add   esp,4

    clc
      ret


__sys_disable_mouse:
      cmp  dword [0xf204],dword 0
      je    @f
      ret
@@:  
      pushad
      cmp  [0x3000],dword 1
      je   disable_m
      mov  edx,[0x3000]
      shl  edx,5
      add  edx,window_data
      movzx  eax,word[0xfb0a]
      movzx  ecx,word[mousepointer+10]
      sub    eax,ecx
      movzx  ebx,word[0xfb0c]
      movzx  ecx,word[mousepointer+12]
      sub    ebx,ecx
      mov  ecx,[0xfe00]
      imul  ecx,ebx
      add  ecx,eax
      add  ecx, display_data
;     mov   eax, [0x3000]
      movzx eax, byte [edx+twdw+0xe]
      movzx ebx, byte [ecx]
      cmp   eax,ebx
      je    yes_mouse_disable
      movzx ebx, byte [ecx+32]
      cmp   eax,ebx
      je    yes_mouse_disable
      mov   ebx,[0xfe00]
      inc   ebx
      imul  ebx,32
      add   ecx,ebx
      movzx ebx, byte [ecx]
      cmp   eax,ebx
      je    yes_mouse_disable
      movzx ebx, byte [ecx+32]
      cmp   eax,ebx
      je    yes_mouse_disable
      jmp   no_mouse_disable
yes_mouse_disable:
      mov  edx,[0x3000]
      shl  edx,5
      add  edx,window_data
      movzx  eax, word [0xfb0a]
      movzx  ebx, word [0xfb0c]
      movzx  ecx,word[mousepointer+10]
      sub    eax,ecx
      movzx  ecx,word[mousepointer+12]
      sub    ebx,ecx
      mov  ecx,[edx+0]   ; mouse inside the area ?
      add  eax,32
      cmp  eax,ecx
      jl   no_mouse_disable
      sub  eax,32
      add  ecx,[edx+8]
      cmp  eax,ecx
      jg   no_mouse_disable
      mov  ecx,[edx+4]
      add  ebx,32
      cmp  ebx,ecx
      jl   no_mouse_disable
      sub  ebx,32
      add  ecx,[edx+12]
      cmp  ebx,ecx
      jg   no_mouse_disable
disable_m:
      cmp  dword [0xf204],dword 0
      jne  no_mouse_disable
      cli
       call [draw_mouse_under]
      sti
      mov  [0xf204],dword 1
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
        cmp    dword [0xf204],dword 0  ; mouse visible ?
        je     chms00
        mov     [0xf204], dword 0
        movzx  ebx,word [0xfb0c]
        movzx  eax,word [0xfb0a]
        movzx  esi,word[mousepointer+10]
        sub    eax,esi
        movzx  esi,word[mousepointer+12]
        sub    ebx,esi
        cli
        call   save_draw_mouse
        sti
nodmu2:
        popad
        ret
chms00:
        movsx  ecx,word[0xfb4a]
        movsx  edx,word[0xfb4c]
        movzx  ebx,word [0xfb0c]
        movzx  eax,word [0xfb0a]
        movzx  esi,word[mousepointer+10]
        sub    eax,esi
        movzx  esi,word[mousepointer+12]
        sub    ebx,esi
        cmp    eax,ecx
        jne    redrawmouse
        cmp    ebx,edx
        jne    redrawmouse
        jmp    nodmp
redrawmouse:
        cli
        call   [draw_mouse_under]
        call   save_draw_mouse
        sti
nodmp:
        popad
        ret

