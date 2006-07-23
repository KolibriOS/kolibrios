max_buttons=4095
dececx:
    push edx
    push ecx

    mov  edx,2
  .loop:

    cmp   byte [esp+edx],0x20
    jae   @f
    mov   [esp+edx],byte 0x20
   @@:
    sub   [esp+edx],byte 0x20

    dec  edx
    jns  .loop

    pop  ecx
    pop  edx
    ret
    
incecx:
    push edx
    push ecx
    
    mov  edx,2
  .loop:
    
    cmp  byte [esp+edx],0xdf
    jbe  @f
    mov  [esp+edx],byte 0xdf
   @@:
    add  [esp+edx],byte 0x20
    
    dec  edx
    jns  .loop
    pop  ecx
    pop  edx
    ret
    
incecx2:
    push edx
    push ecx
    
    mov  edx,2
  .loop:
    
    cmp  byte [esp+edx],0xeb
    jbe  @f
    mov  [esp+edx],byte 0xeb
   @@:
    add  [esp+edx],byte 0x14
    
    dec  edx
    jns  .loop
    pop  ecx
    pop  edx
    ret

drawbuttonframes:

        push  esi
        push  edi
        push  eax
        push  ebx
        push  ecx
        push  edx

        shr   eax,16
        shr   ebx,16
        mov   edx,[0x3010]

        add   eax,[edx-twdw]
        add   ebx,[edx-twdw+4]
        mov   cx,ax
        mov   dx,bx
        shl   eax,16
        shl   ebx,16
        mov   ax,cx
        mov   bx,dx
        add   ax,word [esp+12]
        mov   esi,ebx
        mov   edi,0
        mov   ecx,[esp+0]
        call  incecx
        call  [draw_line]

        movzx edx,word [esp+8]
        add   ebx,edx
        shl   edx,16
        add   ebx,edx
        mov   ecx,[esp+0]
        call  dececx
        call  [draw_line]

        mov   ebx,esi
        push  edx
        mov   edx,eax
        shr   edx,16
        mov   ax,dx
        mov   edx,ebx
        shr   edx,16
        mov   bx,dx
        mov   dx,[esp+8+4]
        add   bx,dx
        pop   edx
        mov   edi,0
        mov   ecx,[esp+0]
        call  incecx
        call  [draw_line]

        mov   esi,edx
        mov   dx,[esp+12]
        add   ax,dx
        shl   edx,16
        add   eax,edx
        add   ebx,1*65536
        mov   edx,esi
        mov   ecx,[esp+0]
        call  dececx
        call  [draw_line]

        pop   edx
        pop   ecx
        pop   ebx
        pop   eax
        pop   edi
        pop   esi

        ret

button_dececx:

        cmp   [buttontype],dword 1
        jne   .finish
;        je    bdece
;        ret
;      bdece:
        push  eax
        mov   eax,0x01
        cmp   edi,20
        jg    @f
        mov   eax,0x02
      @@:
        test  ecx,0xff
        jz    @f
        sub   ecx,eax
      @@:
        shl   eax,8
        test  ecx,0xff00
        jz    @f
        sub   ecx,eax
      @@:
        shl   eax,8
        test  ecx,0xff0000
        jz    @f
        sub   ecx,eax
      @@:
        pop    eax
      .finish:
        ret


sys_button:

        test  ecx,0x80000000
        jnz   remove_button

        push  esi
        push  edi
        push  eax
        push  ebx
        push  ecx
        push  edx

        or    ax,ax
        jle   noaddbutt
        or    bx,bx
        jle   noaddbutt

        test  ecx,0x40000000
        jnz   button_no_draw

        pushad                       ; button body
        push  ebx
        shr   eax,16
        shr   ebx,16
        mov   edx,[0x3010]
        mov   esi,[edx-twdw]
        mov   edi,[edx-twdw+4]
        add   eax,esi
        add   ebx,edi
        mov   cx,ax
        mov   dx,bx
        shl   eax,16
        shl   ebx,16
        mov   ax,cx
        mov   bx,dx
        movzx ecx,word [4+32+esp+12]
        add   eax,ecx
        mov   ecx,[4+32+esp+0]
        cmp   [buttontype],dword 0
        je    @f
        call  incecx2
       @@:
        movzx edi,word [esp]

        pop   edx
        and   edx, 0xFFFF

     .newline:
        call  button_dececx
        push  edi
        xor   edi, edi
        call  [draw_line]
        pop   edi
        add   ebx,1*65536+1   ; [ y start | y end ]
        dec   edx
        jnz   .newline
        popad

        call  drawbuttonframes

      button_no_draw:

        and   ecx,0xffff

        mov   edi,[0x3010]
        sub   edi,twdw

        mov   edi,[0xfe88]
        movzx eax,word [edi]
        cmp   eax,max_buttons
        jge   noaddbutt
        inc   eax
        mov   [edi],ax

        shl   eax,4
        add   eax,edi

        mov   bx,[0x3000]
        mov   [eax],bx

        add   eax,2         ; save button id number
        mov   ebx,[esp+4]
        mov   [eax],bx      ; bits 0-15
        shr   ebx,16
        mov   [eax-2+0xc],bx; bits 16-31
        add   eax,2         ; x start
        mov   bx,[esp+12+2]
        mov   [eax],bx
        add   eax,2         ; x size
        mov   bx,[esp+12+0]
        mov   [eax],bx
        add   eax,2         ; y start
        mov   bx,[esp+8+2]
        mov   [eax],bx
        add   eax,2         ; y size
        mov   bx,[esp+8+0]
        mov   [eax],bx

     noaddbutt:

        pop   edx
        pop   ecx
        pop   ebx
        pop   eax
        pop   edi
        pop   esi

        ret


remove_button:

    and  ecx,0x7fffffff

  rnewba2:

    mov   edi,[0xfe88]
    mov   eax,edi
    movzx ebx,word [edi]
    inc   bx

  rnewba:

    dec   bx
    jz    rnmba

    add   eax,0x10

    mov   dx,[0x3000]
    cmp   dx,[eax]
    jnz   rnewba

    cmp   cx,[eax+2]
    jnz   rnewba

    pushad
    mov   ecx,ebx
    inc   ecx
    shl   ecx,4
    mov   ebx,eax
    add   eax,0x10
    call  memmove
    dec   dword [edi]
    popad

    jmp   rnewba2

  rnmba:

    ret

find_pressed_button_frames:

        pushad

        movzx ebx,word [eax+0]
        shl   ebx,5
        add   ebx,window_data
        movzx ecx,word [ebx+0]     ; window x start
        movzx edx,word [eax+4]     ; button x start
        add   ecx,edx
        push  ecx

        mov   dx,[eax+6]     ; button x size
        add   cx,dx
        mov   esi,ecx
        inc   esi
        mov   cx,[ebx+4]     ; window y start
        mov   dx,[eax+8]     ; button y start
        add   ecx,edx
        mov   ebx,ecx
        mov   dx,[eax+10]    ; button y size
        add   dx,cx
        inc   dx

        pop   eax

        ; eax x beginning
        ; ebx y beginning
        ; esi x end
        ; edx y end
        ; ecx color

        mov   [pressed_button_eax],eax
        mov   [pressed_button_ebx],ebx
        mov   [pressed_button_ecx],ecx
        mov   [pressed_button_edx],edx
        mov   [pressed_button_esi],esi

        popad
        ret

uglobal
  pressed_button_eax  dd  0
  pressed_button_ebx  dd  0
  pressed_button_ecx  dd  0
  pressed_button_edx  dd  0
  pressed_button_esi  dd  0
endg

; negative button image

negativebutton:
        ; If requested, do not display button
        ; boarder on press.
        test  ebx,0x20000000
        jz    draw_negative_button
        ret
      draw_negative_button:

        pushad

        mov   eax,[pressed_button_eax]
        mov   ebx,[pressed_button_ebx]
        mov   ecx,[pressed_button_ecx]
        mov   edx,[pressed_button_edx]
        mov   esi,[pressed_button_esi]
        mov   ecx,0x01000000

        dec   edx
        push  edx
        inc   edx
        dec   esi
        push  esi
        inc   esi

        push  eax
        push  ebx
        push  ecx
        push  edx
        push  edi

        call  [disable_mouse]

      bdbnewline:
        mov   edi,1    ; force
        cmp   eax,[esp+16]
        jz    bneg
        cmp   eax,[esp+20]
        jz    bneg
        cmp   ebx,[esp+12]
        jz    bneg
        cmp   ebx,[esp+24]
        jnz   nbneg
;        jz    bneg
;        jmp   nbneg

      bneg:

        ;;;call  [disable_mouse]
        call  [putpixel]

      nbneg:

        inc   eax
        cmp   eax,esi
        jnz   bdbnewline
        mov   eax,[esp+16]
        inc   ebx
        cmp   ebx,edx
        jnz   bdbnewline

        add   esp,28

        popad

        ret

; check buttons


; 0000 word process number
; 0002 word button id number : bits 0-15
; 0004 word x start
; 0006 word x size
; 0008 word y start
; 000A word y size
; 000C word button id number : bits 16-31
;
; button table in 0x10 increments
;
; first at 0x10


checkbuttons:

    cmp   [0xfb40],byte 0    ; mouse buttons pressed
    jnz   @f
    ret
  @@:

    pushad

    xor    esi, esi
    mov    edi, [0xfe88]
    movzx  edx, word [edi]
    test   edx, edx
    jne    @f
    popad
    ret

  @@:

    push  esi
    inc   edx
    push  edx

  buttonnewcheck:

    pop   edx
    pop   esi
    inc   esi
    cmp   edx,esi
    jge   bch

    popad                 ; no button pressed
    ret

  bch:

    push  esi
    push  edx
    mov   eax,esi
    shl   eax,4
    add   eax,edi

;......................start 1/2 : modified by vhanla .............................
    mov   [buttonid],eax
;......................end   1/2 : modified by vhanla .............................

    ; check that button is at top of windowing stack

    movzx ebx,word [eax]
    movzx ecx,word [0xC000 + ebx * 2]
    cmp   ecx,[0x3004]
    jne   buttonnewcheck

    ; check that button start is inside window x/y end

    movzx ebx,word [eax+0]
    shl   ebx,5

        test    [ebx+window_data+WDATA.fl_wstate],WSTATE_MINIMIZED
        jnz     buttonnewcheck

;    add   ebx,window_data
;    mov   ecx,[window_data+ebx+8]          ; window end X
    movzx edx,word [eax+4]     ; button start X
    cmp   edx, [window_data+ebx+8] ;ecx
    jge   buttonnewcheck

;    mov   ecx,[window_data+ebx+12]         ; window end Y
    movzx edx, word [eax+8]    ; button start Y
    cmp   edx, [window_data+ebx+12] ;ecx
    jge   buttonnewcheck

    ; check coordinates
                               ; mouse x >= button x ?
    movzx ebx,word [eax+0]
    shl   ebx,5
    add   ebx,window_data
    movzx ecx,word [ebx+0]     ; window x start
    movzx edx,word [eax+4]     ; button x start
    add   edx,ecx
    mov   cx,[0xfb0a]
    cmp   edx,ecx
    jg    buttonnewcheck

    movzx ebx,word [eax+6]     ; button x size
    add   edx,ebx
    cmp   ecx,edx
    jg    buttonnewcheck

                               ; mouse y >= button y ?
    movzx ebx,word [eax+0]
    shl   ebx,5
    add   ebx,window_data
    movzx ecx,word [ebx+4]     ; window y start
    movzx edx,word [eax+8]     ; button y start
    add   edx,ecx
    mov   cx,[0xfb0c]
    cmp   edx,ecx
    jg    buttonnewcheck

    movzx ebx,word [eax+10]    ; button y size
    add   edx,ebx
    cmp   ecx,edx
    jg    buttonnewcheck

    ; mouse on button

    pop   edx
    pop   esi

    mov   bx,[eax+0xc]     ; button id : bits 16-31
    shl   ebx,16
    mov   bx,[eax+2]       ; button id : bits 00-16
    push  ebx

    mov   [0xfb44],byte 1  ; no mouse down checks
    call find_pressed_button_frames
    call negativebutton

    pushad
  cbwaitmouseup:

    call  checkidle

    call  [draw_pointer]

    pushad
    call  stack_handler
    popad

    cmp   [0xfb40],byte 0  ; mouse buttons pressed ?
    jnz   cbwaitmouseup
    popad

    call  negativebutton
    mov   [0xfff4],byte 0  ; no mouse background
    mov   [0xfff5],byte 0  ; draw mouse
;..................................... start 2/2 : modified by vhanla .............................
    ; check coordinates
    jmp afterbuttonid
    buttonid dd 0x0 ;here a will backup the eax value
    afterbuttonid:

   pusha
  ; mouse x >= button x ?
   movzx ebx,word [eax+0]
   shl   ebx,5
   add   ebx,window_data
   movzx ecx,word [ebx+0]    ; window x start
   movzx edx,word [eax+4]    ; button x start
   add   edx,ecx
   mov   cx,[0xfb0a]
   cmp   edx,ecx
   jg   no_on_button ;if we release the pointer out of the button area

   movzx ebx,word [eax+6]    ; button x size
   add   edx,ebx
   cmp   ecx,edx
   jg   no_on_button

        ; mouse y >= button y ?
   movzx ebx,word [eax+0]
   shl   ebx,5
   add   ebx,window_data
   movzx ecx,word [ebx+4]    ; window y start
   movzx edx,word [eax+8]    ; button y start
   add   edx,ecx
   mov   cx,[0xfb0c]
   cmp   edx,ecx
   jg   no_on_button

   movzx ebx,word [eax+10]   ; button y size
   add   edx,ebx
   cmp   ecx,edx
   jg   no_on_button
   popa
   mov   [0xf500],byte 1 ; no of buttons in buffer
   pop   ebx
   mov   [0xf501],ebx   ; lets put the button id in buffer
   push  ebx
   pusha
   jmp yes_on_button
no_on_button:
   mov   [0xf500],byte 0 ; no of buttons in buffer
yes_on_button:
   mov   [0xfb44],byte 0 ; mouse down -> do not draw
   popa
   pop ebx
   popa
   ret

;..................................... end 2/2 : modified by vhanla ................................
