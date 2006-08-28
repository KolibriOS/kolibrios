;   *********************************************
;   *******  DRAW PLAY BAR **********************
;   *********************************************

draw_bar:
    push eax ebp
    mov  eax,[cur_tick]
    test eax,eax
    jz   .ex
    mov  ebp,[coo]
    mul  dword [ebp-20]
    mov  ebx,[tick_count]
    test ebx,ebx
    jz   .ex
    div  ebx
    mov  ebx,[ebp-16]
    mov  bx,ax
    mov  ecx,[ebp-12]
    mov  edx,0x84706a
    mov  eax,13
    int  0x40
    mov  esi,TOTALTIME_XY
    mov  eax,[tick_count]
    call draw_total_time
  .nobar:
    mov  esi,[ebp+20]
    mov  ebx,[ebp+12]
    mov  ecx,[ebp+16]
    call draw_cur_time
    mov  esi,140 shl 16+(WND_HEIGHT-18)
    sub  ebx,95 shl 16
    add  ecx,(WND_HEIGHT-34)shl 16
    call draw_cur_time
  .ex:
    pop  ebp eax
    ret



;   *********************************************
;   *******  éèêÖÑÖãÖçàÖ à éíêàëéÇäÄ éäçÄ *******
;   *********************************************

draw_window:
    mov  eax,12
    mov  ebx,1
    int  0x40
    or   [flag],FL_BOTTRED
    xor  eax,eax
    mov  ebx,100*65536+275
    mov  ecx,200*65536+WND_HEIGHT
    mov  edi,hdr_raw+12
    mov  ebp,[coo]
    test [flag],FL_HIDDEN
    jz   .nohide1
    and  ebx,0xffff
    and  ecx,0xffff
    add  edi,275*14*3
  .nohide1:
    push edi
    mov  edx,WND_BACK
    mov  esi,edx
    mov  edi,edx
    int  0x40
    mov  ecx,[channel]
    shl  ecx,4
    add  cl,[sel_track]
    mcall 47,0x20100,,<191,15>,0xf0f000
    movsx ecx,[octave]
    add   ecx,100
    mcall ,0x30000,,<132,15>
    pop  ebx
    mcall 7,,(275 shl 16+14),0
    mov  esi,0x80ecce7a

    mov  eax,8
    mov  ebx,265 shl 16+7
    mov  ecx,3 shl 16+7
    mov  edx,1+1 shl 30
    int  0x40        ; close button
    sub  ebx,10 shl 16
    mov  edx,100+1 shl 30
    int  0x40        ; shade button-101
    sub  ebx,10 shl 16
    add  edx,3
    int  0x40        ; minimize button-103
    sub  ebx,239 shl 16
    inc  edx         ; about button-104
    int  0x40
    xor  esi,esi
    xor  edi,edi
    mov  ecx,3
    pushd [ebp+12]
    pushd [ebp+16]
    call draw_navigation
    add  esp,8

    mov  ebx,[ebp-16]
    add  ebx,[ebp-20]
    mov  ecx,[ebp-12]
    add  ecx,[ebp-24]
    mov  esi,0x2c2b46
    mov  edx,2+1 shl 29
    int  0x40

    shr  eax,1

    mov  ecx,12
    mov  ebx,FN_XY
    mov  edx,[fn_ptr];I_PARAM;filename
    mov  esi,1;[fname_len]
    mov  edi,0x00e200
  .fnlp:
    push ecx
    mcall ,,edi
    add  ebx,1 shl 16
    mcall
    add  ebx,6 shl 16
    inc  edx
    pop  ecx
    loop .fnlp
    add  ebx,5 shl 16
    mcall ,,0xa0a0a0,text,text_end-text
    call draw_bar
    call PL_show
    test [flag],FL_HIDDEN
    jnz  .nohide2
    mov  eax,7
    mov  ebx,btn_raw+12
    mov  ecx,BTNS_SIZE
    mov  edx,BTNS_XY
    int  0x40
  .nohide2:
    mov  esi,FL_SHUFFLE
    mov  edi,btncoords+8
    mov  eax,13
    mov  edx,0xd600
    mov  ecx,2
  .loo3:
    test [flag],esi
    je   .el
    mov  ebx,[edi]
    add  ebx,5 shl 16
    add  ebx,[ebp]
    mov  bx,3
    push ecx
    mov  ecx,[edi+4]
    add  ecx,4 shl 16
    add  ecx,[ebp+4]
    mov  cx,2
    int  0x40
    pop  ecx
  .el:
    add  edi,8
    inc  esi
    loop .loo3

  .enddraw:
    mov  eax,12
    mov  ebx,2
    int  0x40
    ret

draw_navigation:
; ebp - coordinates
    push esi edi ecx
    mov  eax,8

    mov  ecx,5
    mov  ebx,[ebp]
    add  ebx,[ebp-8]
    add  ebx,esi
    mov  edx,3+1 shl 30
  .btnloo:
    push ecx
    mov  ecx,[ebp+4]
    add  ecx,[ebp-4]
    add  ecx,edi
    int  0x40
    pop  ecx
    add  ebx,[ebp+8]
    inc  edx
    loop .btnloo
    pop  ecx

    lea  edi,[ebp+24]
  .btnloo2:
    mov  ebx,[edi]
    add  ebx,[ebp]
    add  ebx,[esp+4]
    push ecx
    mov  ecx,[edi+4]
    add  ecx,[ebp+4]
    add  ecx,[esp+4]
    int  0x40
    pop  ecx
    add  edi,8
    inc  edx
    loop .btnloo2
    pop  edi esi
    ret

draw_cur_time:
    pusha
    mcall 13,,,WND_BACK
    mcall 8,,,101+1 shl 29+1 shl 30
    mov  ebx,[tick_count]
    mov  eax,[cur_tick]
    test [flag],FL_REVERSE
    jz   .norev
    xchg eax,ebx
    sub  eax,ebx
    lea  ebx,[esi-6 shl 16]
    push eax
    mcall 4,,0xf000,dots+1,1
    pop  eax
  .norev:
    mov  [esp+28],eax
    popa
draw_total_time:      ; eax -time to show
    pusha
    xor  edx,edx
    mov  ebx,100
    div  ebx
    xor  edx,edx
    mov  ebx,60
    div  ebx
    push edx
    mov  ecx,eax
    mcall 47,0x20000,,esi,0xf000
    pop  ecx
    add  edx,17 shl 16
    int  0x40
    sub  edx,5 shl 16
    mcall 4,edx,esi,dots,1
    popa
    ret

BOTT_BTN equ 25 shl 16+18
BOTT_SIZEX equ 25
BOTT_SIZEY equ 18
BOTT_S     equ BOTT_SIZEX*BOTT_SIZEY*3
BOTT_X  equ PL_X
BOTT_Y  equ (WND_HEIGHT-27)
BOTT_SPACE equ 30 shl 16

popup db -1

draw_bottom:
    pusha
    mcall 13,175,(BOTT_Y-5)shl 16+24,WND_BACK
    add  ebx,(266-30)shl 16-135
    mcall
    mov  ecx,5
    mov  esi,bot_btn
    push esi
    mov  ebx,bottom_raw+12
    xor  eax,eax
  .nxt:
    push ecx
    lodsw
    shl  eax,16
    lea  edx,[eax+BOTT_Y]
    mcall 7,,BOTT_BTN
    add  ebx,BOTT_S*4
    pop  ecx
    cmp  ecx,4
    jne  .noadj
    add  ebx,BOTT_S
  .noadj:
    loop .nxt
    mov  esi,[esp]
    mov  ecx,5
    mov  ebp,BOTT_Y shl 16+BOTT_SIZEY
    mov  edx,20+1 shl 30
  .nxt2:
    push ecx
    lodsw
    shl  eax,16
    lea  ebx,[eax+BOTT_SIZEX+2 shl 16-3]
    mcall 8,,ebp
    inc  edx
    pop  ecx
    loop .nxt2
    pop  esi
    call draw_popup
    popa
    ret

draw_popup:
    movsx eax,[popup]
    cmp  eax,0
    jge  .ok
    mov  edx,30+1 shl 31
    mov  ecx,21
    mov  eax,8
  .clr:
    mcall
    inc  edx
    loop .clr
    jmp  .ex
  .ok:
    push eax
    lea  edx,[eax+1 shl 31+20]
    mcall 8
    pop  eax
    mov  ebx,eax
    shl  eax,2
    mov  ecx,3
    cmp  eax,4
    jne  .noadj1
    inc  ecx
  .noadj1:
    cmp  eax,8
    jb   .noadj2
    inc  eax
  .noadj2:
    inc  eax
    push eax
    movzx edx,word[bot_btn+ebx*2]
    shl  edx,16
    add  edx,BOTT_Y
    push edx
  .noadj3:
    imul ebx,eax,BOTT_S
    add  ebx,bottom_raw+12
    push ecx
  .nxt:
    push ecx
    mcall 7,,BOTT_BTN
    add  ebx,BOTT_S
    sub  edx,BOTT_SIZEY
    pop  ecx
    loop .nxt
    pop  ecx ebx edx
    mov  ebp,BOTT_Y shl 16+BOTT_SIZEY
    mov  eax,8
    add  edx,1 shl 30+30
    add  ebx,2 shl 16
    mov  bx,BOTT_SIZEX-3
  .nxt2:
    push ecx
    mcall ,,ebp
    inc  edx
    sub  ebp,BOTT_SIZEY shl 16
    pop  ecx
    loop .nxt2
  .ex:
    ret

bot_btn:
    dw 10,40,70,100,240

about1:
  db 13,10,'* MIDAMP for KolibriOS v1.1 by Willow, July 2005 *',13,10,0