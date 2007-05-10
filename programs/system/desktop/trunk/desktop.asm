;
;    UNIFORM WINDOW COLOURS & SKIN
;
;    Compile with FASM for Menuet
;
;    < russian edition by Ivan Poddubny >
;    < skin selection by Mike Semenyako >
;

;******************************************************************************
   use32
   org     0
   db      'MENUET01'  ; identifier
   dd      1           ; header version
   dd      START       ; start address
   dd      I_END       ; file size
   dd      28000h      ; memory
   dd      10000h      ; stack pointer
   dd      0,0         ; parameters, reserved

   include 'lang.inc'
   include '..\..\..\macros.inc'
   include 'kglobals.inc'
   include 'unpacker.inc'
;******************************************************************************


struct SKIN_HEADER
  ident   dd ?
  version dd ?
  params  dd ?
  buttons dd ?
  bitmaps dd ?
ends

struct SKIN_PARAMS
  skin_height    dd ?
  margin.right   dw ?
  margin.left    dw ?
  margin.bottom  dw ?
  margin.top     dw ?
  colors.inner   dd ?
  colors.outer   dd ?
  colors.frame   dd ?
  colors_1.inner dd ?
  colors_1.outer dd ?
  colors_1.frame dd ?
  dtp.size       dd ?
  dtp.data       db 40 dup (?)
ends

struct SKIN_BUTTONS
  type     dd ?
  pos:
    left   dw ?
    top    dw ?
  size:
    width  dw ?
    height dw ?
ends

struct SKIN_BITMAPS
  kind  dw ?
  type  dw ?
  _data  dd ?
ends


START:                          ; start of execution

    mov  eax,48                 ; get current colors
    mov  ebx,3
    mov  ecx,color_table
    mov  edx,4*10
    mcall

    cld
    mov  esi,default_skn
    mov  edi,fname
    mov  ecx,default_skn.size
    rep  movsb
    mov  [skin_info.fname],0
    mov  [skin_info.workarea],0x10000
    call load_skin_file

;    mov  esi, default_dtp
;    mov  edi, fname
;    mov  ecx, default_dtp.size
;    rep  movsb

red:
    call draw_window            ; at first, draw the window

still:

    mov  eax,23                 ; wait here for event
    mov  ebx,5
    mcall

    dec  eax                    ; redraw request ?
    jz   red
    dec  eax                    ; key in buffer ?
    jz   key
    dec  eax                    ; button in buffer ?
    jz   button

    call draw_cursor

    jmp  still


  key:                          ; key
    mov  al,2                   ; just read it and ignore
    mcall
    jmp  still

  button:                       ; button
    mov  al,17                  ; get id
    mcall

    cmp  ah,11                  ; read string
    jne  no_string
    call read_string
    jmp  still
  no_string:

    cmp  ah,12                  ; load file
    jne  no_load
    call load_file
    call draw_window
    jmp  still
  no_load:

    cmp  ah,13                  ; save file
    jne  no_save
    call save_file
    jmp  still
  no_save:

    cmp  ah,14                  ; set 3d buttons
    jne  no_3d
    mov  eax,48
    mov  ebx,1
    mov  ecx,1
    mcall
    jmp  doapply
   no_3d:

    cmp  ah,15                  ; set flat buttons
    jne  no_flat
    mcall 48, 1, 0
doapply:
    mcall 48, 0, 0
    jmp  still
  no_flat:

    cmp  ah,16                  ; apply
    jne  no_apply
  apply_direct:
    mov  eax,48
    mov  ebx,2
    mov  ecx,color_table
    mov  edx,10*4
    mcall
    jmp  doapply
  no_apply:

    cmp  ah,17                  ; load skin file
    jne  no_load_skin
    call load_skin_file
    call draw_window
    jmp  still
  no_load_skin:

    cmp   ah,18                 ; apply skin
    jne   no_apply_skin
    cmp   [skin_info.fname],0
    je    no_apply_skin
    mcall 48,8,skin_info
    call  draw_window
    jmp   still
  no_apply_skin:

    cmp  ah,31
    jb   no_new_colour
    cmp  ah,41
    jg   no_new_colour
    shr  eax,8
    sub  eax,31
    shl  eax,2
    mov  ebx,[color]
    mov  [eax+color_table],ebx
    cmp  dword[0x18000+SKIN_HEADER.ident],'SKIN'
    jne  @f
    mov  edi,[0x18000+SKIN_HEADER.params]
    mov  dword[edi+0x18000+SKIN_PARAMS.dtp.data+eax],ebx
    call draw_skin
@@: call draw_colours
    jmp  still
  no_new_colour:

    cmp  ah,1                   ; terminate
    jnz  noid1
    or   eax,-1
    mcall
  noid1:

    jmp  still


draw_cursor:

    pusha
    mov  eax,37
    mov  ebx,2
    mcall

    cmp  eax,0
    jne  dc1
    popa
    ret

 dc1:

    mov  eax,37
    mov  ebx,1
    mcall

    mov  ebx,eax
    shr  ebx,16
    mov  ecx,eax
    and  ecx,0xffff

    cmp  ecx,32
    jbe  no_color
    cmp  ebx,32
    jbe  no_color

    cmp  ebx,266           ; CHANGE COLOR
    jb   no_color
    cmp  ebx,266+20*3
    jg   no_color

    cmp  ecx,30+128
    jge  no_color
    cmp  ecx,30
    jb   no_color

    sub  ebx,266
    mov  eax,ebx
    cdq
    mov  ebx,20
    div  ebx
    mov  ebx,2
    sub  ebx,eax

    add  ecx,-30
    not  ecx
    shl  ecx,1

    mov  byte [ebx+color],cl
    call draw_color

    popa
    ret

  no_color:

    popa
    ret


load_file:
        xor     eax, eax
        mov     ebx, read_info
        mov     dword [ebx], eax       ; subfunction: read
        mov     dword [ebx+4], eax     ; offset (low dword)
        mov     dword [ebx+8], eax     ; offset (high dword)
        mov     dword [ebx+12], 40     ; read colors file: 4*10 bytes
        mov     dword [ebx+16], color_table ; address
        mcall   70
        ret

load_skin_file:
        xor     eax, eax
        mov     ebx, read_info
        mov     dword [ebx], eax       ; subfunction: read
        mov     dword [ebx+4], eax     ; offset (low dword)
        mov     dword [ebx+8], eax     ; offset (high dword)
        mov     dword [ebx+12], 32*1024 ; read: max 32 KBytes
        mov     dword [ebx+16], 0x10000 ; address
        mcall   70

        mov     esi, 0x10000

        cmp     dword [esi], 'KPCK'
        jnz     notpacked
        cmp     dword [esi+4], 32*1024 ; max 32 KBytes
        ja      doret
        push    0x20000
        push    esi
        call    unpack
        mov     esi, 0x20000
notpacked:

    cmp   dword[esi+SKIN_HEADER.ident],'SKIN'
    jne   doret

    mov   edi,0x18000
    mov   ecx,0x8000/4
    rep   movsd

    mov   esi,fname
    mov   edi,skin_info.fname
    mov   ecx,257
    rep   movsb

    mov   ebp,0x18000
    mov   esi,[ebp+SKIN_HEADER.params]
    add   esi,ebp
    lea   esi,[esi+SKIN_PARAMS.dtp.data]
    mov   edi,color_table
    mov   ecx,10
    rep   movsd
  doret:

ret


save_file:
        mov     ebx, write_info
        mov     dword [ebx], 2         ; subfunction: write
        and     dword [ebx+4], 0       ; (reserved)
        and     dword [ebx+8], 0       ; (reserved)
        mov     dword [ebx+12], 10*4   ; bytes to write
        mov     dword [ebx+16], color_table ; address
        mcall   70
        ret

read_string:

    pusha

    mov  edi,fname
    mov  al,'_'
    mov  ecx,87
    cld
    rep  stosb

    call print_text

    mov  edi,fname

  f11:
    mov  eax,10
    mcall
    cmp  eax,2
    jne  read_done
;    mov  eax,2
    mcall
    shr  eax,8
    cmp  eax,13
    je   read_done
    cmp  eax,8
    jne  nobsl
    cmp  edi,fname
    je   f11
    dec  edi
    mov  [edi],byte '_'
    call print_text
    jmp  f11
   nobsl:
    mov  [edi],al

    call print_text

    inc  edi
    cmp  edi, fname+87
    jne  f11

  read_done:

    mov  ecx, fname+88
    sub  ecx, edi
    mov  eax, 0
    cld
    rep  stosb

    call print_text

    popa

    ret


print_text:
    pushad

    mpack ebx,15,6*87+4
    mpack ecx,(30+18*10+2),11
    mcall 13,,,[w_work]

    mpack ebx,17,(30+18*10+4)
    mcall 4,,[w_work_text],fname,87

    popad
ret


draw_color:

    pusha

    mov  eax,13
    mov  ebx,266*65536+60
    mov  ecx,170*65536+30
    mov  edx,[color]
    mcall

;   mov  eax,13
    mov  ebx,266*65536+60
    mov  ecx,200*65536+10
    mov  edx,[w_work]
    mcall

    mov  eax,47
    mov  ebx,0+1*256+8*65536
    mov  ecx,[color]
    mov  edx,272*65536+201
    mov  esi,[w_work_text]
    mcall

    popa

    ret


draw_colours:

    pusha

    mov  esi,color_table

    mov  ebx,225*65536+32
    mov  ecx,32*65536+12
  newcol:
    mov  eax,13
    mov  edx,[esi]
    mcall
    add  ecx,18*65536
    add  esi,4
    cmp  esi,color_table+4*9
    jbe  newcol

    popa

    ret


draw_framerect: ; ebx,ecx
        push    ebx ecx
        add     bx,[esp+6]
        mov     cx,[esp+2]
        dec     ebx
        mcall   38
        add     cx,[esp]
        rol     ecx,16
        add     cx,[esp]
        sub     ecx,0x00010001
        mcall
        mov     ebx,[esp+4]
        mov     ecx,[esp]
        mov     bx,[esp+6]
        add     cx,[esp+2]
        dec     ecx
        mcall
        add     bx,[esp+4]
        rol     ebx,16
        add     bx,[esp+4]
        sub     ebx,0x00010001
        mcall
        add     esp,8
        ret

find_bitmap:
        mov     edi,[ebp+SKIN_HEADER.bitmaps]
        add     edi,ebp
        xor     ebx,ebx
  .lp1: cmp     dword[edi],0
        je      .lp2
        cmp     dword[edi+0],eax
        jne     @f
        mov     ebx,[edi+SKIN_BITMAPS._data]
        add     ebx,ebp
        mov     ecx,[ebx-2]
        mov     cx,[ebx+4]
        add     ebx,8
  .lp2: ret
    @@: add     edi,8
        jmp     .lp1

dec_edx:
        sub     dl,4
        jnc     @f
        xor     dl,dl
    @@: sub     dh,4
        jnc     @f
        xor     dh,dh
    @@: rol     edx,16
        sub     dl,4
        jnc     @f
        xor     dl,dl
    @@: rol     edx,16
        ret

area:
  .x      = 345
  .y      = 20
  .width  = 206
  .height = 191

wnd1:
  .x      = area.x+49
  .y      = area.y+5
  .width  = 150
  .height = 90
wnd2:
  .x      = area.x+35
  .y      = area.y+35
  .width  = 150
  .height = 90
wnd3:
  .x      = area.x+21
  .y      = area.y+65
  .width  = 150
  .height = 90
wnd4:
  .x      = area.x+7
  .y      = area.y+95
  .width  = 150
  .height = 90

virtual at edi+SKIN_PARAMS.dtp.data
  dtp system_colors
end virtual

draw_skin:
        mcall   13,<area.x,area.width>,<area.y+2,area.height-2>,0x00FFFFFF

        mov     ebp,0x18000
        mov     edi,[ebp+SKIN_HEADER.params]
        add     edi,ebp
        mpack   ebx,wnd1.x,wnd1.width
        mpack   ecx,wnd1.y,wnd1.height
        mov     edx,[dtp.frame]
        call    draw_framerect
        mcall   13,<wnd1.x+1,wnd1.width-2>,<wnd1.y+1,wnd1.height-2>,dword[dtp.work]

        mov     eax,38
        mpack   ebx,wnd1.x+1,wnd1.x+wnd1.width-2
        mpack   ecx,wnd1.y+1,wnd1.y+1
        mov     edx,[dtp.grab]
        mov     esi,20
    @@: mcall
        call    dec_edx
        add     ecx,0x00010001
        dec     esi
        jnz     @b

        mov     edi,[ebp+SKIN_HEADER.params]
        add     edi,ebp
        mcall   4,<wnd1.x+6,wnd1.y+7>,dword[dtp.grab_text],caption_text,caption_text.size

        mcall   8,<wnd1.x+wnd1.width-18,12>,<wnd1.y+4,12>,0,[dtp.grab_button]
        mcall   4,<wnd1.x+wnd1.width-18+4,wnd1.y+4+2>,dword[dtp.grab_button_text],close_text,close_text.size

;----------------------------------------------------------------------

        mov     edi,[ebp+SKIN_HEADER.params]
        add     edi,ebp
        mpack   ebx,wnd2.x,wnd2.width
        mpack   ecx,wnd2.y,wnd2.height
        mov     edx,[dtp.frame]
        shr     edx,1
        and     edx,0x007F7F7F
        call    draw_framerect
        mpack   ebx,wnd2.x+4,wnd2.width-8
        mpack   ecx,wnd2.y+4,wnd2.height-8
        call    draw_framerect
        mcall   13,<wnd2.x+1,wnd2.width-2>,<wnd2.y+1,3>,[dtp.frame]
        add     ecx,(wnd2.height-5)*65536
        mcall
        mcall   ,<wnd2.x+1,3>,<wnd2.y+1,wnd2.height-2>
        add     ebx,(wnd2.width-5)*65536
        mcall
        mcall   ,<wnd2.x+5,wnd2.width-10>,<wnd2.y+5,wnd2.height-10>,dword[dtp.work]

        mov     eax,38
        mpack   ebx,wnd2.x+4,wnd2.x+wnd2.width-5
        mpack   ecx,wnd2.y+4,wnd2.y+4
        mov     edx,[dtp.grab]
        mov     esi,16
    @@: mcall
        call    dec_edx
        add     ecx,0x00010001
        dec     esi
        jnz     @b

        mov     edi,[ebp+SKIN_HEADER.params]
        add     edi,ebp
        mcall   4,<wnd2.x+8,wnd2.y+7>,dword[dtp.grab_text],caption_text,caption_text.size

        mcall   8,<wnd2.x+wnd2.width-20,12>,<wnd2.y+4,12>,0,[dtp.grab_button]
        mcall   4,<wnd2.x+wnd2.width-20+4,wnd2.y+4+2>,dword[dtp.grab_button_text],close_text,close_text.size

;----------------------------------------------------------------------

        mov     edi,[ebp+SKIN_HEADER.params]
        add     edi,ebp
        mpack   ebx,wnd3.x,wnd3.width
        mpack   ecx,wnd3.y,wnd3.height
        mov     edx,[edi+SKIN_PARAMS.colors_1.outer]
        call    draw_framerect
        mpack   ebx,wnd3.x+4,wnd3.width-8
        mpack   ecx,wnd3.y+4,wnd3.height-8
        mov     edx,[edi+SKIN_PARAMS.colors_1.inner]
        call    draw_framerect
        mcall   13,<wnd3.x+1,wnd3.width-2>,<wnd3.y+1,3>,[edi+SKIN_PARAMS.colors_1.frame]
        add     ecx,(wnd3.height-5)*65536
        mcall
        mcall   ,<wnd3.x+1,3>,<wnd3.y+1,wnd3.height-2>
        add     ebx,(wnd3.width-5)*65536
        mcall
        mcall   ,<wnd3.x+5,wnd3.width-10>,<wnd3.y+5,wnd3.height-10>,dword[dtp.work]

        mov     eax,0x00000001 ; left, inactive
        call    find_bitmap
        mcall   7,,,<wnd3.x,wnd3.y>

        pushd   [ebx-8]
        mov     eax,0x00000003 ; base, inactive
        call    find_bitmap
        pop     edx
        mov     esi,wnd3.x+wnd3.width-1
        sub     esi,edx
        shl     edx,16
        add     edx,wnd3.x*65536+wnd3.y
        mcall   7
    @@: rol     edx,16
        add     dx,[ebx-8]
        cmp     dx,si
        ja      @f
        rol     edx,16
        mcall   7
        jmp     @b
    @@:

        mov     eax,0x00000002 ; oper, inactive
        call    find_bitmap
        mov     edx,ecx
        shr     edx,16
        neg     edx
        shl     edx,16
        add     edx,(wnd3.x+wnd3.width)*65536+wnd3.y
        mcall   7

        mov     ebp,0x18000
        mov     edi,[ebp+SKIN_HEADER.params]
        add     edi,ebp
        mov     eax,dword[edi+SKIN_PARAMS.margin.left-2]
        mov     ax,word[edi+SKIN_PARAMS.skin_height]
        sub     ax,[edi+SKIN_PARAMS.margin.bottom]
        shr     ax,1
        add     ax,[edi+SKIN_PARAMS.margin.top]
        add     ax,-4
        push    eax
        lea     ebx,[eax+wnd3.x*65536+wnd3.y]
        mcall   4,,dword[dtp.grab_text],caption_text,caption_text.size

;---------------------------------------------------------

        mov     edi,[ebp+SKIN_HEADER.params]
        add     edi,ebp
        mpack   ebx,wnd4.x,wnd4.width
        mpack   ecx,wnd4.y,wnd4.height
        mov     edx,[edi+SKIN_PARAMS.colors.outer]
        call    draw_framerect
        mpack   ebx,wnd4.x+4,wnd4.width-8
        mpack   ecx,wnd4.y+4,wnd4.height-8
        mov     edx,[edi+SKIN_PARAMS.colors.inner]
        call    draw_framerect
        mcall   13,<wnd4.x+1,wnd4.width-2>,<wnd4.y+1,3>,[edi+SKIN_PARAMS.colors.frame]
        add     ecx,(wnd4.height-5)*65536
        mcall
        mcall   ,<wnd4.x+1,3>,<wnd4.y+1,wnd4.height-2>
        add     ebx,(wnd4.width-5)*65536
        mcall
        mcall   ,<wnd4.x+5,wnd4.width-10>,<wnd4.y+5,wnd4.height-10>,dword[dtp.work]

        mov     eax,0x00010001 ; left, inactive
        call    find_bitmap
        mcall   7,,,<wnd4.x,wnd4.y>

        pushd   [ebx-8]
        mov     eax,0x00010003 ; base, inactive
        call    find_bitmap
        pop     edx
        mov     esi,wnd4.x+wnd4.width-1
        sub     esi,edx
        shl     edx,16
        add     edx,wnd4.x*65536+wnd4.y
        mcall   7
    @@: rol     edx,16
        add     dx,[ebx-8]
        cmp     dx,si
        ja      @f
        rol     edx,16
        mcall   7
        jmp     @b
    @@:

        mov     eax,0x00010002 ; oper, inactive
        call    find_bitmap
        mov     edx,ecx
        shr     edx,16
        neg     edx
        shl     edx,16
        add     edx,(wnd4.x+wnd4.width)*65536+wnd4.y
        mcall   7

        mov     ebp,0x18000
        mov     edi,[ebp+SKIN_HEADER.params]
        add     edi,ebp
        pop     eax
        lea     ebx,[eax+wnd4.x*65536+wnd4.y]
        mcall   4,,dword[dtp.grab_text],caption_text,caption_text.size

;----------------------------------------------------------------------

        mov     edi,[ebp+SKIN_HEADER.buttons]
        add     edi,ebp
  .lp1: cmp     dword[edi],0
        je      .lp2
        mov     ebx,dword[edi+SKIN_BUTTONS.left-2]
        mov     bx,[edi+SKIN_BUTTONS.width]
        mov     ecx,dword[edi+SKIN_BUTTONS.top-2]
        mov     cx,[edi+SKIN_BUTTONS.height]
        add     ebx,(wnd4.x+wnd4.width)*65536
        add     ecx,wnd4.y*65536
        dec     ebx
        dec     ecx
        mcall   8,,,0x40000000
        add     edi,12
        jmp     .lp1
  .lp2:

        mov     edi,[ebp+SKIN_HEADER.params]
        add     edi,ebp
        mpack   ebx,wnd4.x+10,wnd4.y+10
        add     bx,word[edi+SKIN_PARAMS.skin_height]
        mcall   4,,[dtp.work_text],window_text,window_text.size

        mov     ecx,[edi+SKIN_PARAMS.skin_height]
        shl     ecx,16
        add     ecx,(wnd4.y+8)*65536+10
        mcall   13,<wnd4.x+window_text.size*6+20,wnd4.x+wnd4.width-10-\
                   (wnd4.x+window_text.size*6+20)>,,[dtp.work_graph]

        add     ecx,25*65536+8
        mcall   8,<wnd4.x+wnd4.width/2-button_text.size*3-6,\
                  button_text.size*6+11>,,0,[dtp.work_button]

        shr     ecx,16
        mov     bx,cx
        add     ebx,0x00060006
        mcall   4,,[dtp.work_button_text],button_text,button_text.size

        ret

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall

    mov  eax,48
    mov  ebx,3
    mov  ecx,app_colours
    mov  edx,10*4
    mcall

    mov  eax,14
    mcall

                                      ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,110*65536+555         ; [x start] *65536 + [x size]
    mov  ecx,50*65536+255          ; [y start] *65536 + [y size]
    mov  edx,[w_work]              ; color of work area RRGGBB,8->color
    or   edx,0x13000000
    mov  edi,title                ; WINDOW LABEL
    mcall

if lang eq ru
  load_w  = (5*2+6*9)
  save_w  = (5*2+6*9)
  flat_w  = (5*2+6*7)
  apply_w = (5*2+6*9)
else
  load_w  = (5*2+6*6)
  save_w  = (5*2+6*6)
  flat_w  = (5*2+6*4)
  apply_w = (5*2+6*7)
end if

    mov  eax,8                    ; FILENAME BUTTON
    mov  ebx,5*65536+545
    mov  ecx,212*65536+10
    mov  edx,0x4000000B
    mov  esi,[w_grab_button]       ; button color RRGGBB
    mcall

;   mov  eax,8                    ; LOAD BUTTON
    mov  ebx,15*65536+load_w
    mov  ecx,(30+18*11)*65536+14
    mov  edx,12
    mov  esi,[w_work_button]
    mcall

;   mov  eax,8                    ; SAVE BUTTON
    add  ebx,(load_w+2)*65536-load_w+save_w
    inc  edx
    mcall

;   mov  eax,8                    ; 3D
;   mov  ebx,15*65536+35
;   mov  ecx,(30+18*12)*65536+14
    mov  ebx,(340-t1.size*6-13)*65536+(5*2+6*4)
    inc  edx
    mcall

;   mov  eax,8                    ; FLAT
    add  ebx,(5*2+6*4+2)*65536-(5*2+6*4)+flat_w
    inc  edx
    mcall

;   mov  eax,8                    ; APPLY BUTTON
    add  ebx,(flat_w+6+2)*65536-flat_w+apply_w
    inc  edx
    mcall

;   mov  eax,8                    ; LOAD SKIN BUTTON
    mov  ebx,(336+(555-335)/2-t2.size*6/2)*65536+load_w
    inc  edx
    mcall

;   mov  eax,8                    ; APPLY SKIN BUTTON
    add  ebx,(load_w+6+2)*65536-load_w+apply_w
    inc  edx
    mcall

    mov  eax, 4
    mov  ebx, (339-t1.size*6-12)*65536+(30+18*11+4)
    mov  ecx, [w_work_button_text]
    mov  edx, t1
    mov  esi, t1.size
    mcall

    mov  ebx,(336+(555-335)/2-t2.size*6/2)*65536+(30+18*11+4)
    mov  edx,t2
    mov  esi,t2.size
    mcall

;   mov  eax, 4
;    mov  ebx, 277*65536+(30+18*12+4)
;    mov  edx, t2
;    mov  esi, t2.size
;    mcall

    mov  eax,38                    ; R G B COLOR GLIDES
    mov  ebx,266*65536+285
    mov  ecx,30*65536+30
    mov  edx,0xff0000
  .newl:
    mcall
    pusha
    add  ebx,20*65536+20
    shr  edx,8
    mcall
    add  ebx,20*65536+20
    shr  edx,8
    mcall
    popa
    sub  edx,0x020000
    add  ecx,0x00010001
    cmp  ecx,158*65536+158
    jnz  .newl

    call draw_color

    mov  edx,31                    ; BUTTON ROW
    mov  ebx,15*65536+200
    mov  ecx,30*65536+14
    mov  esi,[w_work_button]
  newb:
    mov  eax,8
    mcall
    add  ecx,18*65536
    inc  edx
    cmp  edx,40
    jbe  newb

    mov  ebx,15*65536+34           ; ROW OF TEXTS
    mov  ecx,[w_work_button_text]
    mov  edx,text
    mov  esi,32
  newline:
    mov  eax,4
    mcall
    add  ebx,18
    add  edx,32
    cmp  [edx],byte 'x'
    jne  newline

    call draw_colours

    mcall 13,<5,546>,<212,11>,[w_work]
    mcall 13,<337,7>,<2,250>,[w_frame]
    shr   edx,1
    and   edx,0x007F7F7F
    mcall 38,<336,336>,<20,250>
    add   ebx,0x00080008
    mcall
    sub   ebx,0x00040004
    mcall ,,<0,255>
    mcall ,<5,550>,<211,211>
    add   ecx,0x000C000C
    mcall

    call print_text

    cmp  dword[0x18000+SKIN_HEADER.ident],'SKIN'
    jne  @f
    call draw_skin
  @@:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall

    ret


; DATA AREA

lsz text,\
    ru,  ' êÄåäÄ éäçÄ                     ',\
    ru,  ' èéãéëÄ áÄÉéãéÇäÄ               ',\
    ru,  ' äçéèäÄ çÄ èéãéëÖ áÄÉéãéÇäÄ     ',\
    ru,  ' íÖäëí çÄ äçéèäÖ çÄ áÄÉéãéÇäÖ   ',\
    ru,  ' íÖäëí áÄÉéãéÇéäÄ               ',\
    ru,  ' êÄÅéóÄü éÅãÄëíú                ',\
    ru,  ' äçéèäÄ Ç êÄÅéóÖâ éÅãÄëíà       ',\
    ru,  ' íÖäëí çÄ äçéèäÖ                ',\
    ru,  ' íÖäëí Ç êÄÅéóÖâ éÅãÄëíà        ',\
    ru,  ' ÉêÄîàäÄ Ç êÄÅéóÖâ éÅãÄëíà      ',\
    ru,  '                                ',\
    ru,  ' áÄÉêìáàíú  ëéïêÄçàíú           ',\
    ru,  'x',\
    en,  ' WINDOW FRAME                   ',\
    en,  ' WINDOW GRAB BAR                ',\
    en,  ' WINDOW GRAB BUTTON             ',\
    en,  ' WINDOW GRAB BUTTON TEXT        ',\
    en,  ' WINDOW GRAB TITLE              ',\
    en,  ' WINDOW WORK AREA               ',\
    en,  ' WINDOW WORK AREA BUTTON        ',\
    en,  ' WINDOW WORK AREA BUTTON TEXT   ',\
    en,  ' WINDOW WORK AREA TEXT          ',\
    en,  ' WINDOW WORK AREA GRAPH         ',\
    en,  '                                ',\
    en,  '  LOAD    SAVE                  ',\
    en,  'x',\
    et,  ' AKNA RAAM                      ',\
    et,  ' AKNA HAARAMISE RIBA            ',\
    et,  ' AKNA HAARAMISE NUPP            ',\
    et,  ' AKNA HAARAMISE NUPU TEKST      ',\
    et,  ' AKNA HAARAMISE PEALKIRI        ',\
    et,  ' AKNA T÷÷PIIRKOND               ',\
    et,  ' AKNA T÷÷PIIRKONNA NUPP         ',\
    et,  ' AKNA T÷÷PIIRKONNA NUPPU TEKST  ',\
    et,  ' AKNA T÷÷PIIRKONNA TEKST        ',\
    et,  ' AKNA T÷÷PIIRKONNA GRAAFIKA     ',\
    et,  '                                ',\
    et,  '  LAADI SALVESTA                ',\
    et,  'x'

lsz t1,\
    ru, '  3D   èãéëäàÖ   èêàåÖçàíú ',\
    en, '  3D   FLAT    APPLY  ',\
    et, '  3D   LAME   KINNITA '

lsz t2,\
    ru,  ' áÄÉêìáàíú   èêàåÖçàíú ',\
    en,  '  LOAD     APPLY  ',\
    et,  '  LAADI   KINNITA '

lsz caption_text,\
    ru, 'á†£Æ´Æ¢Æ™',\
    en, 'Caption',\
    et, 'Pealkiri'

sz  close_text,'x'

lsz window_text,\
    ru, 'í•™·‚ ¢ Æ™≠•',\
    en, 'Window text',\
    et, 'Akna tekst'

lsz button_text,\
    ru, 'í•™·‚ ≠† ™≠ÆØ™•',\
    en, 'Button text',\
    et, 'Nupu tekst'

sz  default_skn, '/RD/1/DEFAULT.SKN',0

if lang eq ru
  title db 'çÄëíêéâäÄ éäéç',0
else if lang eq et
  title db 'AKNA SEADED - VALI VƒRV JA VAJUTA OBJEKTILE',0
else
  title db 'WINDOWS SETTINGS - DEFINE COLOR AND CLICK ON TARGET',0
end if


color dd  0

IncludeIGlobals

I_END:

IncludeUGlobals

read_info:
  .mode         dd ?            ; read
  .start_block  dd ?            ; first block
  .blocks       dd ?            ; 512 bytes
  .address      dd ?
  .workarea     dd ?
fname rb 256+1            ; filename (+1 - for zero at the end)

virtual at read_info
 write_info:
  .mode         dd ?
  rd 1
  .bytes2write  dd ?
  .address      dd ?
  .workarea     dd ?
end virtual

skin_info:
  .mode         dd ?
  .start_block  dd ?
  .blocks       dd ?
  .address      dd ?
  .workarea     dd ?
  .fname rb 256+1

app_colours:

w_frame              dd ?
w_grab               dd ?
w_grab_button        dd ?
w_grab_button_text   dd ?
w_grab_text          dd ?
w_work               dd ?
w_work_button        dd ?
w_work_button_text   dd ?
w_work_text          dd ?
w_work_graph         dd ?

color_table:
  times 10 dd ?
