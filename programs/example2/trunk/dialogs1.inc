;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                       ;
;    DIALOGS1.INC                                       ;
;                                                       ;
;    COMPILE WITH FASM for MENUET                       ;
;                                                       ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


menus     dd   3        ; number of menus
m_x       dd   0x5      ; x start
m_y       dd   20       ; y start
m_xs      dd   290      ; x size
m_ys      dd   14       ; y size
g_stack   dd   0xf000   ; thread stack - required


menu:; AB C           D            E            F            G

  db  '*D FILE        +Save File   +Load File   +-           +Quit        '
  db  '*B EDIT        +Copy        +Paste       '
  db  '*B HELP        +Setup       +About..     '
  db    '@' ; end mark

;  A   : Data type '*' -> New menu , '+' -> menu selection
;  B   : Number of selections in menu (A+)
;  C   : Menu header text
;  D-G : Menu selection text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;  DATA BELOW IS FOR DIALOGS1.INC INTERNALS

menu_action dd '----'

window_on   db 0

g_n         dd -1
g_x         dd 0x0
g_t         dd 0x0
g_1         dd 0x0
g_l         dd 0x0
closet      db 0

table: times 1024 db 0

last_mouse  dd  0x0

mo_x       dd 0x0
mo_y       dd 0x0


check_mouse:

     pusha

     cmp  [window_on],1
     je   no_open

     mov  eax,37
     mov  ebx,2
     int  0x40

     cmp  [window_on],0
     jne  openw2

     cmp  eax,0
     je   no_open

  openw2:

   waitformouse:

     mov  eax,23
     mov  ebx,2
     int  0x40

     cmp  eax,0
     jne  no_open

     mov  eax,37
     mov  ebx,2
     int  0x40

     cmp  eax,0
     jne  waitformouse


     mov  eax,37
     mov  ebx,1
     int  0x40

     mov  esi,eax

     shr  eax,16
     xor  edx,edx
     mov  ebx,50
     div  ebx
     mov  edx,eax
     cmp  edx,[g_n]
     je   no_open
     cmp  edx,[menus]
     jge  no_open
     mov  eax,esi

     and  eax,0xffff

     mov  ebx,[m_y]
     cmp  eax,ebx
     jbe  no_open
     add  ebx,[m_ys]
     cmp  eax,ebx
     jge  no_open

     cmp  [window_on],0
     je   noww

     mov  [closet],1
     mov  ecx,100
   waitm:
     mov  eax,5
     mov  ebx,1
     int  0x40
     dec  ecx
     jz   no_open
     cmp  [window_on],0
     jne  waitm
   noww:

     mov  eax,edx
     jmp  cll

   no_open:

     mov  [last_mouse],esi

     popa

     ret

   cll:

     mov  [window_on],2

     mov  [g_n],eax
     mov  [g_x],96
     mov  [g_t],0
     mov  [g_1],1

     mov  eax,9
     mov  ebx,table
     mov  ecx,-1
     int  0x40

     mov  eax,[table+34]
     mov  [mo_x],eax
     mov  eax,[table+38]
     mov  [mo_y],eax

     mov  eax,51
     mov  ebx,1
     mov  ecx,alert_entry
     mov  edx,[g_stack]
     int  0x40

     mov  [esp+28],dword 0  ; clear button entry

     mov  [menu_action],'MD  '

   check_gr:

     popa

     ret


draw_menu:

     mov  eax,9
     mov  ebx,table
     mov  ecx,-1
     int  0x40

     cmp  [table+46],dword 30
     jb   drmr

     mov  eax,13     ; white background
     mov  ebx,[m_x]
     shl  ebx,16
     add  ebx,[m_xs]
     inc  ebx
     mov  ecx,[m_y]
     shl  ecx,16
     add  ecx,[m_ys]
     mov  edx,0xf0f8ff
     int  0x40

     mov  eax,38     ; egde lines
     mov  ebx,[m_x]
     shl  ebx,16
     add  ebx,[m_x]
     add  ebx,[m_xs]
     mov  ecx,[m_y]
     shl  ecx,16
     add  ecx,[m_y]
     mov  edx,0x000000
     int  0x40
     mov  eax,38
     mov  ecx,[m_y]
     add  ecx,[m_ys]
     shl  ecx,16
     add  ecx,[m_y]
     add  ecx,[m_ys]
     int  0x40

     mov  esi,menu-1
     mov  edi,[m_x]
     mov  ebp,1
    new_menu:
     inc  esi

     cmp  [esi],byte '*'
     jne  drmnl1
     push esi
     mov  eax,4
     mov  ebx,edi
     shl  ebx,16
     add  ebx,[m_y]
     add  ebx,0x00050004
     mov  ecx,0x000000
     mov  edx,esi
     add  edx,3
     mov  esi,12
     int  0x40    ; draw text
     pop  esi
     add  esi,2
     add  edi,50
     inc  ebp

    drmnl1:
     cmp  [esi],byte '@'
     jne  new_menu

    drmr:

     ret

alert_box:

   ; eax : x size - min 200
   ; ebx : pointer to ASCIIZ - max 128 character text
   ; ecx : button 1 id          ( OK or YES )
   ; edx : button 2 id or zero  ( NO )


   cmp  [window_on],0
   jne  alert_box_return

   mov  [window_on],1

   cmp  eax,100
   jg   size_ok
   mov  eax,100
 size_ok:

   mov  [g_x],eax
   mov  [g_t],ebx
   mov  [g_1],ecx

   mov  ecx,0
 new_search:
   cmp  [ebx],byte 0
   je   found_len
   inc  ebx
   inc  ecx
   cmp  ecx,128
   jbe  new_search
 found_len:
   mov  [g_l],ecx

   mov  eax,51
   mov  ebx,1
   mov  ecx,alert_entry
   mov  edx,[g_stack]
   int  0x40

   mov  [menu_action],'MA  '

 alert_box_return:

   ret

alert_entry:

     call alert_draw_window

alert_still:

    mov  eax,23                 ; wait here for event
    mov  ebx,1
    int  0x40

    cmp  eax,1                  ; redraw request ?
    je   alert_red
    cmp  eax,2                  ; key in buffer ?
    je   alert_key
    cmp  eax,3                  ; button in buffer ?
    je   alert_button

    cmp  [closet],0
    jne  ccc

    mov  eax,9
    mov  ebx,table
    mov  ecx,-1
    int  0x40

    cmp  ax,[table+4]
    je   no_close
  ccc:
    mov  [closet],0
    mov  [g_n],-1
    mov  [menu_action],'----'
    mov  [window_on],0
    mov  eax,-1
    int  0x40
  no_close:

    jmp  alert_still

  alert_red:                          ; redraw
    call alert_draw_window
    jmp  alert_still

  alert_key:                    ; key
    mov  eax,2                  ; just read it and ignore
    int  0x40
    jmp  alert_still

  alert_button:                 ; button
    mov  eax,17                 ; get id
    int  0x40

    shr  eax,8
    cmp  eax,3
    jg   no_action1
    dec  eax
    shl  eax,2
    mov  eax,dword [eax+rtext]
    mov  [menu_action],eax
    jmp  action_done
  no_action1:
    sub  eax,16
    add  eax,65
    shl  eax,8
    mov  ebx,[g_n]
    add  ebx,65
    add  eax,ebx
    mov  [menu_action],eax

  action_done:

    mov  [closet],0
    mov  [g_n],-1
    mov  [window_on],0
    mov  eax,-1                 ; close this program
    int  0x40

  rtext db 'NO  YES OK  '

    jmp  alert_still


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


alert_draw_window:


    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40

    cmp  [window_on],2
    jne  no_win_type_2

    mov  edx,menu-1
    mov  ecx,[g_n]
    add  ecx,1
   find_menu:
    inc  edx
    cmp  [edx],byte '*'
    je   menu_loop
    jmp  find_menu
   menu_loop:
    loop find_menu
    movzx ebp,byte [edx+1]
    sub  ebp,64
    push edx
                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,[g_n]
    imul ebx,50
    add  ebx,[mo_x]
    add  ebx,[m_x]
    shl  ebx,16
    add  ebx,[g_x]
    mov  ecx,[mo_y]
    add  ecx,[m_y]
    add  ecx,[m_ys]
    shl  ecx,16
    mov  edx,14
    imul edx,ebp
    add  edx,7
    add  ecx,edx
    mov  edx,0x00ffffff            ; color of work area RRGGBB,8->color gl
    mov  esi,0x00ffffff            ; color of grab bar  RRGGBB,8->color gl
    mov  edi,0x000000cc            ; color of frames    RRGGBB
    int  0x40

    pop  edx

    mov  ebx,5*65536+7            ; draw info text with function 4
    mov  ecx,0x10000000
    mov  esi,12
    mov  ebp,16
  no_d_found:
    inc  edx
    cmp  [edx],byte '*'
    je   d_drawed
    cmp  [edx],byte '@'
    je   d_drawed
    cmp  [edx],byte '+'
    jne  no_d_found
    inc  edx
    pusha          ; draw button
    mov  eax,8
    mov  ecx,ebx
    mov  ebx,[g_x]
    add  ebx,0x0000fffe
    shl  ecx,16
    add  ecx,0xfffc0000+14
    mov  edx,0x40000000
    add  edx,ebp
    mov  esi,0
    int  0x40
    popa
    mov  eax,4     ; draw text
    int  0x40
    inc  ebp
    add  ebx,14
    jmp  no_d_found
  d_drawed:

  no_win_type_2:


    cmp  [window_on],1
    jne  no_win_1

    mov  eax,14                    ; to middle of screen
    int  0x40
    mov  ecx,eax
    and  ecx,0xffff
    shr  ecx,1
    shr  eax,1
    mov  ebx,[g_x]
    shr  ebx,1
    shl  ebx,16
    sub  eax,ebx
    mov  ebx,eax

    mov  eax,0                     ; function 0 : define and draw window
    mov  bx,word [g_x]
    sub  ecx,80
    shl  ecx,16
    mov  cx,110                    ; [y start] *65536 + [y size]
    mov  edx,0x02ffffff            ; color of work area RRGGBB,8->color gl
    mov  esi,0x80d05050            ; color of grab bar  RRGGBB,8->color gl
    mov  edi,0x00d05050            ; color of frames    RRGGBB
    int  0x40


    mov  eax,4                       ; label
    mov  ebx,8*65536+8
    mov  ecx,0x10ddeeff
    mov  edx,alert_labelt1
    mov  esi,alert_label1len-alert_labelt1
    int  0x40

    mov  eax,4
    mov  ebx,10*65536+43
    mov  ecx,0x10000000
    mov  edx,[g_t]
    mov  esi,[g_l]
    int  0x40

    cmp  [g_1],1
    jne  gadgets_no_1

    mov  eax,8
    mov  ebx,[g_x]
    sub  ebx,100
    shr  ebx,1
    shl  ebx,16
    add  ebx,30*65536+40
    mov  ecx,75*65536+16
    mov  edx,3
    mov  esi,0x446688
    int  0x40

    mov  eax,4
    mov  ebx,[g_x]
    sub  ebx,100
    shr  ebx,1
    shl  ebx,16
    add  ebx,31*65536+80
    mov  ecx,0x10ffffff
    mov  edx,alert_t2
    mov  esi,alert_t2len-alert_t2
    int  0x40

  gadgets_no_1:

    cmp  [g_1],2
    jne  gadgets_no_2

    mov  eax,8
    mov  ebx,[g_x]
    sub  ebx,100
    shr  ebx,1
    shl  ebx,16
    add  ebx,0*65536+40
    mov  ecx,75*65536+16
    mov  edx,1
    mov  esi,0x446688
    int  0x40

    mov  eax,8
    mov  ebx,[g_x]
    sub  ebx,100
    shr  ebx,1
    shl  ebx,16
    add  ebx,57*65536+40
    mov  ecx,75*65536+16
    mov  edx,2
    mov  esi,0x446688
    int  0x40

    mov  eax,4
    mov  ebx,[g_x]
    sub  ebx,100
    shr  ebx,1
    shl  ebx,16
    add  ebx,1*65536+80
    mov  ecx,0x10ffffff
    mov  edx,alert_t1
    mov  esi,alert_t1len-alert_t1
    int  0x40

  gadgets_no_2:

  no_win_1:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret


; DATA AREA


alert_t1:
    db '  No      Yes'
alert_t1len:


alert_t2:
    db '  OK'
alert_t2len:


alert_labelt1:
     db   'ALERT'
alert_label1len:








