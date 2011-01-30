; SOKOBAN FOR MENUET v0.1
; Written in pure assembler by Ivushkin Andrey aka Willow
;
; Last changed: July 2, 2004
;
; Main idea, art & graphics
;   Sokofun for Windows 95 by Games 4 Brains
;   and Sokoban 2.3 by BjЎrn Kфllmark
;
; Level designers:
;
;   Alberto Garcia, Aymeric du Peloux, Brian Kent, David Holland,
;   David W Skinner, Erim Sever, Evgeniy Grigoriev, Franчois Marques,
;   Frantisek Pokorny, Howard Abed,J franklin Mentzer, Jaques Duthen,
;   John C Davis, John Polhemus, Kobus Theron, Lee Haywood, Mario Bonenfant,
;   Martin P Holland, Mic (Jan Reineke), Phil Shapiro, Richard Weston,
;   Sven Egevad, Ken'ichiro Takahashi (takaken), Thinking Rabbit,
;   Yoshio Murase, ZICO (Zbigniew Kornas)
;
; Special thanks to Hirohiko Nakamiya
;
; More credits:
;   Masato Hiramatsu, Kazuo Fukushima, Klaus Clemens
;
; Game uses its own format of levelset files *.LEV
;   with simple run-length compression

; COMPILE WITH FASM

CUR_DIR equ '/HD/1/MENUETOS/' ; change it to appropriate path

SKIN_SIZE equ 11520         ; size of skin file (16x240)

; field dimensions
FLD_LEFT equ 43
FLD_LEFT2 equ FLD_LEFT shl 16
FLD_TOP equ 40
FLD_TOP2 equ FLD_TOP shl 16
IMG_SIZE equ 16 shl 16+16
SHIFT equ (16 shl 16)
WND_COLOR equ 0x03aabbcc

; level list dimensions
LEVLIST_XY equ FLD_TOP shl 16+45
LEVLIST_SPACING equ 10
LEVWND_X equ 320
LEVWND_Y equ 200

; input line dimensions
INP_X equ 10 shl 16+300
INP_Y equ 160 shl 16+16
INP_XY equ 15 shl 16+164

; load button dimensions
LOAD_X equ 130 shl 16+65
LOAD_Y equ 180 shl 16+14
LOAD_XY equ 135 shl 16+184
CHOOSE_XY equ 40 shl 16+148


WIN_XY equ 135 shl 16+25

; workmode constants, more defs in CELLTYPE.INC
WM_WINNER  equ 0x10
WM_READSET equ 0
WM_LOSE    equ 0x20

use32
  org    0x0
  db     'MENUET01'
  dd     0x01
  dd     START
  dd     I_END
  dd     0x100000
  dd     0x7fff0
  dd     0x0
  dd     0x0

include 'macros.inc'    ; decrease code size (optional)
include 'celltype.inc'  ; object identifiers
;include 'debug.inc'
;lang equ ru             ; russian interface; english if undefined

START:
    mov  eax,58         ; load skin image-it is in RAW 16x240 BGR
    mov  ebx,file_info  ; IrfanView recommended
    int  0x40
    test ebx,ebx
;    jmp load_level
;    jz   close
  load_fail:            ; clear input line, also if levelset load failed
    mov  [inp_pos],0
  load_std:
    mov  esi,stdlev
    mov  edi,path_end
    mov  ecx,stdlev_len-stdlev
    rep  movsb
    mov  ecx,10
  reset_fi:
    mov  dword[cnf_level],level_start
    xor  eax,eax
    mov  [levpage],eax
    mov  word[ll_num],'00'   ; reset some counters
  read_cnf:
    mov  eax,58
    mov  ebx,file_info
    int  0x40
    test ebx,ebx        ; load standard levels SOKO-?.LEV instead of custom
    jz   nxt_cnf
    add  dword[cnf_level],ebx
  nxt_cnf:
    test ecx,ecx        ; this check is for loading a custom levelset
    jz   no_increase
    inc  byte[file_num] ; next standard levelset
    loop read_cnf
  no_increase:
    cmp  dword[cnf_level],level_start
    jne  go_on
    test ecx,ecx
    jz   load_fail
    jmp  close          ; missing standard levels & exiting
  go_on:
    mov  eax,[cnf_level]
    mov  byte[eax],0xf0 ; end-of-levels mark

    call read_levelset
  backto_set:
    mov  byte[workmode],WM_READSET
    mov  byte[winmode],0
    jmp  red
restart_level:
    call decode_field   ; uncompress level

red:
    call draw_window

still:

    mov  eax,10
    int  0x40
    cmp  byte[winmode],WM_WINNER
    je   backto_set
    cmp  byte[winmode],WM_LOSE
    je   backto_set
    cmp  eax,1
    je   red
    cmp  eax,2
    je   key
    cmp  eax,3
    je   button

    jmp  still

  key:
    mov  eax,2
    int  0x40

    cmp  byte[workmode],WM_READSET
    jne  key_move
    cmp  ah,32          ; Space moves focus to input line
    je   is_input
    cmp  ah,184
    jne  no_prev
    cmp  [levpage],0    ; PgUp
    jz   still
    sub  [levpage],10
    cmp  byte[ll_num+1],'0'
    jnz  _pu
    dec  byte[ll_num]
    mov  byte[ll_num+1],'9'+1
  _pu:
    dec  byte[ll_num+1]
    jmp  red
  no_prev:
    cmp  ah,183         ; PgDn
    jne  no_next
    mov  eax,[levpage]
    add  eax,10
    cmp  eax,[levelcount]
    jae  still
    mov  [levpage],eax
    cmp  byte[ll_num+1],'9'
    jnz  _pd
    inc  byte[ll_num]
    mov  byte[ll_num+1],'0'-1
  _pd:
    inc  byte[ll_num+1]
    jmp  red
  no_next:
    sub  ah,48
    cmp  ah,9
    ja   still
    movzx eax,ah        ; user selects a level
    add  eax,[levpage]
    cmp  eax,[levelcount]
    jae  still
    mov  eax,[levelmap+eax*4]
    mov  [levptr],eax   ; current level pointer
    mov  al,byte[eax]
    mov  byte[workmode],al
    jmp  restart_level

    ; we're already in game
  key_move:
    cmp  ah,180        ; Home
    je   backto_set
    cmp  ah,176
    jb   no_arrows
    sub  ah,176
    cmp  ah,3
    ja   no_arrows
    movzx ecx,ah
    movzx edx,[player]
    inc  ch
    call valid_move
    cmp  byte[winmode],WM_WINNER
    jne  no_winner
    mov  ecx,0x20ac0000
    mov  edx,win_msg
    mov  esi,win_msg_end-win_msg   ; print victory congratulations
  print_msg:
    mov  ebx,WIN_XY
    mov  eax,4
    int  0x40
    jmp  d_f
  no_winner:
    cmp  byte[winmode],WM_LOSE
    jne  d_f
  no_loser:
    test  al,al         ; no move accepted
    jnz   still
  d_f:
    call draw_field     ; move performed-redraw
    jmp  still
  no_arrows:
    cmp  ah,27
    je   restart_level

    jmp  still

  button:
    mov  eax,17
    int  0x40

    cmp  ah,1
    jne  noclose
  close:
    xor  eax,eax
    dec  eax
    int  0x40           ; shutdown.

  noclose:
    cmp  ah,2
    jne  no_input
  is_input:             ; simple input line with backspace feature
    mov  ebx,[entered]  ; sorry - no cursor
    test ebx,ebx
    jnz  wait_input
    mov  [inp_pos],ebx
    inc  [entered]
  wait_input:
    call draw_input
    mov  eax,10
    int  0x40
    cmp  eax,2
    jne  still
    mov  edi,[inp_pos]
    mov  eax,2
    int  0x40
    shr  eax,8
    cmp  eax,27
    je   still
    cmp  eax,13
    je   load_level
    cmp  eax,8
    je   backsp
    mov  [fn_input+edi],al
    inc  [inp_pos]
    jmp  wait_input
  backsp:
    test edi,edi
    jz   wait_input
    dec  [inp_pos]
    jmp  wait_input
  no_input:
    cmp  ah,3
    jne  no_load
  load_level:
    mov  ecx,[inp_pos]
    test ecx,ecx
    je   load_std
    mov  esi,fn_input
    mov  byte[esi+ecx],0
    inc  ecx
    mov  edi,path_end
    rep  movsb
    jmp  reset_fi
  no_load:
    jmp  still


;   *********************************************
;   ** FILLS LEVEL POINTER MAP ******************
;   *********************************************
read_levelset:

    mov  dword[wnd_width],LEVWND_X
    mov  dword[wnd_height],LEVWND_Y
    mov  [levelcount],0
    mov  edi,level_start
    mov  esi,levelmap
    mov  al,0xff
  rls_cycle:
    cmp  byte[edi],EOF
    je   end_of_levelset
    mov  [esi],edi
    add  esi,4
    mov  ecx,1024
    inc  [levelcount]
    repne scasb
    jecxz eol  ;end_of_levelset
    jmp  rls_cycle
  end_of_levelset:
    mov  eax,[levelcount]
;    debug_print_dec eax
    ret
  eol:
;   debug_print '*** '
    jmp  end_of_levelset

;   *********************************************
;   *******  DEFINE & DRAW WINDOW & OTHER STUFF *
;   *********************************************

draw_window:

    mov  eax,12
    mov  ebx,1
    int  0x40

    mov  eax,0
    mov  ebx,150*65536
    add  ebx,[wnd_width]
    mov  ecx,50*65536
    add  ecx,[wnd_height]
    mov  edx,WND_COLOR
    mov  esi,0x805080d0
    mov  edi,0x005080d0
    int  0x40

    mov  eax,4
    mov  ebx,8*65536+8
    mov  ecx,0x10ddeeff
    mov  edx,zagolovok
    mov  esi,zag_konets-zagolovok
    int  0x40

    cmp  byte[workmode],WM_READSET
    je   list_levels

    mov  edi,[levptr]   ; print custom level filename
    add  ebx,170*65536
    lea  edx,[edi+4]
    movzx  esi,byte[edi+3]
    int  0x40

    call draw_field
    cmp  [entered],0
    jz   end_of_draw
    mov  edx,fn_input   ; print input line text
    mov  esi,[inp_pos]
    mov  ebx,FLD_LEFT2+FLD_TOP-15
    jmp  draw_level_file

  list_levels:

    call draw_input

    mov  eax,8          ; draw load button
    mov  ebx,LOAD_X
    mov  ecx,LOAD_Y
    mov  edx,3
    mov  esi,WND_COLOR
    int  0x40

    mov  eax,4
    mov  ecx,0x20107a30
    mov  ebx,LOAD_XY
    mov  edx,load_char
    mov  esi,loadlen-load_char
    int  0x40

    mov  ebx,LEVLIST_XY
    mov  edi,0x204e00e7
    xor  esi,esi
    mov  ecx,10
  ll_cycle:
    push ecx esi ebx esi
    lea  ecx,[esi+'0']
    mov  [ll_num+2],cl
    mov  ecx,edi
    mov  edx,ll_num
    mov  esi,4
    int  0x40
    add  ebx,25 shl 16
    pop  esi
    add  esi,[levpage]
    mov  edx,[levelmap+esi*4]
    add  edx,4
    movzx esi,byte[edx-1]
    int  0x40
    pop  ebx esi ecx
    inc  esi
    mov  edx,[levelcount]
    sub  edx,[levpage]
    cmp  edx,esi
    jbe  choose_print
    add  ebx,LEVLIST_SPACING
    loop ll_cycle
  choose_print:
    mov  edx,ll_msg
    mov  esi,ll_msg_end-ll_msg
    mov  ebx,CHOOSE_XY
  draw_level_file:
    mov  eax,4
    int  0x40

  end_of_draw:
    mov  eax,12
    mov  ebx,2
    int  0x40

    ret

;   *********************************************
;   ******* DRAW CELL IMAGES WITHIN FIELD *******
;   *********************************************

draw_field:
    cmp  byte[workmode],sSokonex
    jne  no_chl
    call check_lasers   ; Sokonex game
  no_chl:
    mov  eax,13         ; clear field area
    mov  edx,WND_COLOR
    mov  edi,[levptr]
    movzx ebx,byte[edi+1]
    shl  ebx,4
    lea  ebx, [FLD_LEFT2+ebx]
    movzx ecx,byte[edi+2]
    shl  ecx,4
    lea  ecx, [FLD_TOP shl 16+ecx]
    int  0x40

    mov  edx, FLD_LEFT2+FLD_TOP
    movzx edi,byte[edi+1]
    shl  edi,20
    add  edi, FLD_LEFT2

    xor  eax,eax
    mov  ecx,[fld_size]
    mov  esi,field
  fld_cycle:
    lodsb
    call draw_img
    add  edx,SHIFT
    cmp  edx,edi
    jl  no_nl
    add  edx,16
    and  edx,0xffff
    add  edx,FLD_LEFT2
  no_nl:
    loop fld_cycle
    cmp  byte[workmode],sSokonex
    jne  end_of_df
    call draw_lasers
  end_of_df:
    ret

;   *********************************************
;   *********** DRAW CELL IMAGE *****************
;   *********************************************

draw_img:               ; in: eax-object index, edx-coordinates
    pusha
    cmp  eax,tWall
    jbe  no_adjust
    cmp  [workmode],sSokolor
    jne  no_di_color
    add  eax,pm_col-pm_nex
    jmp  no_adjust
  no_di_color:
    cmp  [workmode],sSokonex
    jne  no_adjust
    inc  eax
  no_adjust:
    movzx  ebx,byte [pic_map+eax]
    cmp  ebx,0xf
    je   no_img
  bl_place:
    mov  ecx, IMG_SIZE
    imul ebx, 256*3
    add  ebx,strip
    mov  eax,7          ; draw_image sysfunc
    int  0x40
  no_img:
    popa
    ret

;****************************************
;******* DRAW CONTENTS OF INPUT LINE ****
;****************************************
draw_input:
    push edi
    cmp  eax,4
    jne  highlight
    mov  esi,WND_COLOR
    jmp  di_draw
  highlight:
    mov  esi,0xe0e0e0
  di_draw:
    mov  eax,8
    mov  ebx,INP_X
    mov  ecx,INP_Y
    mov  edx,2
    int  0x40
    mov  eax,4
    mov  ecx,0x00107a30            ; шрифт 1 и цвет ( 0xF0RRGGBB )
    mov  ebx,INP_XY
    mov  edx,fn_input
    mov  esi,[inp_pos]
    int  0x40
    pop  edi
    ret

;   ********************************************************
;   * DECOMPRESS LEVEL & FILL SOME TABLES TO CHECK VICTORY *
;   ********************************************************

decode_field:
;    debug_print <13,10>
    xor  eax,eax
    mov  dword[checkpoint],eax
    mov  dword[checkpoint+4],eax
    mov  byte[checkcount],al
    mov  edi,[levptr]
    mov  dl,[edi]
    mov  [workmode],dl
    movzx  edx,byte[edi+1]
    mov  esi,edx
    shl  esi,4
    add  esi,FLD_LEFT*2-25
    mov  [wnd_width],esi
    neg  edx
    mov  [move_map+8],edx
    neg  edx
    mov  [move_map+4],edx
    movzx eax,byte[edi+2]
    mov  esi,eax
    shl  esi,4
    add  esi,FLD_TOP*2-18
    mov  [wnd_height],esi
    imul edx,eax
    mov  [fld_size],edx
    lea  esi,[edi+4]
    movzx ecx,byte[edi+3]
    add  esi,ecx
    cmp  byte[esi],0xff
    je   backto_set
    xor  edi,edi
    cld
 dec_cycle:
    lodsb
    movzx ecx,al
    and  ecx,0xf                   ; ecx-count of objects
    shr  al,4                      ; eax-index of object
    inc  ecx
    sub  edx,ecx
  dc_cycle:
    mov  [field+edi],al
    call chk_win_obj
    jne  no_register
    push eax ecx esi
    movzx ecx,al
    shl  eax,12
    or   eax,edi
    inc  byte[checkcount]
    cmp  [workmode],sSokolor
    jne  chk_sokoban
;    debug_print ':'
;    debug_print_dec ecx
    sub  ecx,tRedB
    shl  ecx,1
    cmp  word[checkpoint+ecx],0
    jnz  no_new_check
    mov  [checkpoint+ecx],ax
    and  eax,0xfff
;    debug_print_dec eax
    jmp  no_new_check
  chk_sokoban:
    cmp  [workmode],sSokonex
    jne  no_nex
    cmp  byte[checkcount],1
    ja   no_new_check
  no_nex:
    movzx ecx,byte[checkcount]
    mov  word[checkpoint-2+ecx*2],ax
  no_new_check:
    pop  esi ecx eax
  no_register:
    inc  edi
    loop dc_cycle
    cmp  edx,0
    jg   dec_cycle
    mov  ecx,[fld_size]
    xor  edx,edx
  fp_cycle:
    mov  al,[field+edx]
    and  al,0xfe
    cmp  al,tPlayer
    je   pl_found
    inc  edx
    loop fp_cycle
  pl_found:
    mov  [player],dx
    movzx eax,byte[checkcount]
;    debug_print_dec eax
    ret

;   *********************************************
;   * WHETHER OBJECT IS VICTORY DEPENDENT *******
;   *********************************************

chk_win_obj:             ; al-object in a cell
    push ecx eax
    and  al,0xf
    mov  cl,[workmode]
    cmp  cl,sSokoban
    jne  nota_sokoban
    cmp  al,tBlock
    jmp  cwo_exit
  nota_sokoban:
    cmp  cl,sSokonex
    jne  nota_sokonex
    cmp  al,tConnect
    je   cwo_exit
    cmp  al,tStConnect
    jmp  cwo_exit
  nota_sokonex:
    push eax
    and  eax,tRedB
    cmp  eax,tRedB
    pop  eax
  cwo_exit:
    pop  eax ecx
    ret

;   *********************************************
;   ***** GET CELL AT CERTAIN DIRECTION *********
;   *********************************************

get_cell_at:            ; in:  dx - current cell, cl - direction
    mov  ebx,edx        ; out: al - object at direction, bx - new position
    movzx eax,cl
    and  eax,11b
    mov  eax, [move_map+eax*4]
    add  ebx,eax
    mov  al,[field+ebx]
    ret

;   *********************************************
;   *** WHETHER A MOVE CAN BE DONE, & DO IT *****
;   *********************************************

valid_move:                         ; in:  dx - current cell, cl - direction
    push edx esi
    call get_cell_at                ; if ch>0 perform all moves
    cmp  al,tWall
    jb   result_ok
    je   vm_exit
    cmp  [workmode],sSokonex
    jne  n_vm_nex
    cmp  al,tStConnect
    je   vm_exit
    cmp  al,tHole
    je   vm_exit
  n_vm_nex:
    push edx ebx
    mov  edx,ebx
    movzx esi,al
    call get_cell_at
    cmp  al,tPlace
    jbe  push_it
    cmp  [workmode],sSokonex
    jne  no_plate
    cmp  al,tHole
    jne  no_plate
    cmp  esi,tBroken
    jae  vm_sink
    cmp  esi,tPlate
    jne  no_plate
    and  byte[field+ebx],0
  vm_sink:
    and  byte[field+edx],0
    jmp  vm_hole
  no_plate:
    pop  ebx edx esi edx
    ret
  push_it:
    call do_move
  vm_hole:
    pop  ebx edx
  result_ok:
    call do_move
    xor  al,al
  vm_exit:
    pop  esi edx
    ret

;   *********************************************
;   ******* ACTUALLY PERFORM MOVES **************
;   *********************************************

do_move:                            ; in: dx - source cell
    test ch,ch                      ;     bx - target cell
    jz   dm_exit                    ;     ch = 0 don't perform moves
    mov  al,byte[field+edx]
    cmp  byte[workmode],sSokoban
    jne  no_dm_ban
    and  al,0xfe
  no_dm_ban:
    xor  byte[field+edx],al
    or   byte[field+ebx],al
    call chk_win_obj
    jne  no_check_win
    pusha
    movzx ecx,byte[checkcount]
    xor  edi,edi
  dm_cycle:
    movzx esi,word[checkpoint+edi*2]
    and  esi,0xfff
    and  edx,0xfff
    cmp  esi,edx
    jnz  not_an_obj
    movzx eax,dl
    movzx eax,byte[field+ebx]
    shl  eax,12
    or   eax,ebx
    mov  word[checkpoint+edi*2],ax
    jmp  dm_ex
  not_an_obj:
    inc  edi
    loop dm_cycle
  dm_ex:
    popa
    call check_win
    jne  no_check_win
    mov  byte[winmode],WM_WINNER
  no_check_win:
    cmp  al,tPlayer
    jne  dm_exit
    mov  [player],bx
  dm_exit:
    ret

;   *********************************************
;   ******* CHECK VICTORY CONDITIONS ************
;   *********************************************

check_win:
;    debug_print <13,10>
    push eax ebx ecx esi
    xor  eax,eax
    movzx ecx,byte[checkcount]
    mov  esi,checkpoint
    mov  bl,byte[workmode]
    xor  bh,bh
    mov  [colcount],bh
    cld
  cw_cycle:
    lodsw
    cmp  bl,sSokoban
    jne  nocw_sokoban
    test ax,1 shl 12
    jz   cw_not_inplace
    inc  bh
  cw_not_inplace:
    loop cw_cycle
;    movzx eax,bh
    cmp  [checkcount],bh
    jmp  cw_exit
  nocw_sokoban:
    cmp  bl,sSokonex
    jne  nocw_sokonex
    mov  dx,ax
    call scan_root
    cmp  al,[checkcount]
    jmp  cw_exit

  nocw_sokonex:
    cmp  esi,checkpoint+8
    ja   cwlor_exit
;    debug_print '*'
    test ax,ax
    jz   cw_cycle
    mov  dx,ax
    call scan_root
    add  [colcount],al
;    debug_print '*->'
;    debug_print_dec eax
    jmp  cw_cycle
  cwlor_exit:
    mov  al,[colcount]
    cmp  al,[checkcount]
  cw_exit:
;    debug_print <13,10>
    pop esi ecx ebx eax
    ret

;   *********************************************
;   **** WHETHER LASERS DESTROY SOMETHING *******
;   *********************************************

check_lasers:
    pusha
    xor  edx,edx
    mov  ecx,[fld_size]
  cl_loop:
    push ecx edx
    mov  cl,[field+edx]
    sub  cl,tLaserW
    jl   cl_exit
  cl_begin:
    call get_cell_at
    cmp  al,tLaserW
    jae  cl_destroy
    cmp  al,tBroken
    je   cl_destroy
    cmp  al,tEmpty
    je   no_cl_destroy
    cmp  al,tHole
    je   no_cl_destroy
    cmp  al,tPlayer
    jne  cl_exit
    mov  ecx,0x20ac0000
    mov  edx,lose_msg
    mov  esi,lose_msg_end-lose_msg  ; print loose message
    mov  byte[winmode],WM_LOSE
    mov  ebx,WIN_XY
    mov  eax,4
    int  0x40
    jmp  cl_exit
  cl_destroy:
    mov  byte[field+ebx],0
  no_cl_destroy:
    mov  edx,ebx
    jmp  cl_begin
  cl_exit:
    pop  edx ecx
    inc  edx
    loop cl_loop
    popa
    ret

;   *********************************************
;   *** USED BY CHECK_WIN IN SOKONEX & SOKOLOR **
;   *********************************************

scan_root:                ; input:   dx-beginning cell, ebx-what to search
    push esi
    mov  edi,srch         ; output:  eax-number of win_obj found
    mov  eax,0xfff
    movzx ecx,[checkcount]
    inc  ecx
    cld
    rep  stosw            ; clearing area for scan
    movzx ebx,dx
    and  edx,eax          ; dx-cell number to compare with
    shr  ebx,12           ; bl-obj id
    mov  [color],bl
    mov  esi,srch
    mov  edi,eax          ; mask to extract cell
    mov  word[srch],dx
  sr_loop:
    lodsw
    push esi              ; saving scan pointer
       movzx edx,ax       ; edx-[dirs*4][cell*12]
       and  edx,edi
;       debug_print ' >'
       mov  ecx,4
  sr_dir_loop1:
;       debug_print '.'
       push ecx           ; saving dir counter
         lea  ebx,[ecx+11]
         bts  word[esi-2],bx
         jc   sr_endloop      ; this entry is already processed
;         debug_print '^'
         dec  ecx             ; cl-direction
         call get_cell_at     ; bx-new position, al-object
;         cmp  [workmode],sSokonex
;         jne  no_sr_nex
         call chk_win_obj
         jne  sr_endloop      ; not a win_obj there
;         debug_print '@'
         cmp  [workmode],sSokolor
         jne  no_sr_lor
         cmp  al,[color]
         jne  sr_endloop
       no_sr_lor:
         push esi
         mov  esi,srch        ; let us search for existing entries
       sr_loop1:
         lodsw
         and  eax,edi         ; eax-cell w/o dirs
         cmp  eax,ebx
         je   sr_foundentry   ; this is the entry we're seeking for
         cmp  word[esi],0xfff
         jnz  sr_loop1        ; next entry
    ; we reached empty area
         mov  [esi],bx
         add  esi,2
       sr_foundentry:
         mov  eax,15
         sub  eax,ecx
         bts  [esi-2],ax      ; mark entry as used
         pop  esi
;         inc  [e_fnd]         ; one more obj found
     sr_endloop:
       pop  ecx
       loop sr_dir_loop1
;       jmp  tttt
;     sr_dir_loop:
;       jmp  sr_dir_loop1
;     tttt:
    pop esi
    cmp  word[esi],0xfff
    jne  sr_loop
    mov  eax,esi
    sub  eax,srch
    shr  eax,1
    pop  esi
;    debug_print_dec eax
    ret

;   *********************************************
;   *** SPECIAL ROUTINE TO DRAW LASERS **********
;   *********************************************

draw_lasers:
    xor  edx,edx
    mov  ecx,[fld_size]
  dl_loop:
    push ecx edx
    mov  cl,[field+edx]
    sub  cl,tLaserW
    jl   dl_eloop
    inc  ch
  dl_gca:
    call get_cell_at
    cmp  al,tEmpty
    je   dl_draw
    cmp  al,tHole
    jne  dl_eloop
  dl_draw:
    call draw_beams
    mov  edx,ebx
    jmp  dl_gca
  dl_eloop:
    pop  edx
    inc  edx
    pop  ecx
    loop dl_loop
    ret

;   *********************************************
;   *** DRAWS LASER BEAMS IN CERTAIN DIRECTION **
;   *********************************************

draw_beams:
    pusha
    mov esi,[levptr]
    movzx esi,byte[esi+1]
    mov  eax,ebx
    xor  edx,edx
    div  esi
    movzx esi,cl
    dec  esi
    shr  esi,1
    and  esi,1
    shl  edx,20
    mov  ebx,edx
    shl  eax,20
    mov  ecx,eax
    add  ebx,dword[beam_xy+esi*8]
    add  ecx,dword[beam_xy+esi*8+4]
    mov  edx,0xe9e25c
    mov  eax,13
    int  0x40
    popa
    ret

ud:
   ud2  ; debugging purposes only


;   *********************************************
;   *** COMPRESS LEVEL - NOT READY YET **********
;   *********************************************

;    push    esi ebx ;ecx
;    xchg    ebx,edi
;    mov     esi,edi                 ; esi,edi - beginning
;; ebx - end of unpacked field
;  first_enc:
;    lodsb                           ; al - first byte
;    shl     ax,8                    ; ah - this byte, al=0
;  next_enc:
;    cmp     esi,ebx
;    jae     exit_enc
;;    movzx   ecx,byte[esi]
;;    debug_print_dec ecx
;    cmp     ah,byte[esi]
;    jne     newchar
;    inc     esi
;    inc     al
;    cmp     al,15
;    jb      next_enc
;  newchar:
;    shl     al,4
;    shr     ax,4
;    stosb
;    jmp     first_enc
;  exit_enc:
;    shl     al,4
;    shr     ax,4
;    stosb
;    mov     al,0xff
;    stosb
;    pop     ebx esi ecx
;
;    dec     ecx
;    jcxz    outcycle
;    jmp     next_lev
;  outcycle:


; Здесь находятся данные программы:

; интерфейс программы двуязычный - задайте язык в macros.inc
load_char:
if lang eq ru
     db 'Загрузить'
else
     db 'Open file'
end if
loadlen:

ll_msg:
if lang eq ru
     db 'Выберите уровень'
else
     db 'Choose a level'
end if
     db ' (0-9, PgUp, PgDn)'
ll_msg_end:

fn_input:
;   db 'cnf'
;    db 'soko-4.lev'
if lang eq ru
     db 'или введите имя файла'
else
     db 'or enter a filename'
end if
inp_end:
     rb 256-(inp_end-fn_input)

win_msg:
if lang eq ru
     db 'Ура!!! Вы прошли уровень!'
else
     db "You've completed the level!"
end if
win_msg_end:

lose_msg:
if lang eq ru
     db 'Вы парализованы! Проигрыш...'
else
     db "You're paralized! Game over..."
end if
lose_msg_end:

zagolovok:               ; строка заголовка
if lang eq ru
     db   'СОКОБАН ДЛЯ'
else
     db   'SOKOBAN FOR'
end if
     db   ' MENUET'
zag_konets:              ; и её конец

pic_map:
    db 0xf,9,0,0,1,1,5,6
pm_nex:
    db 2,7,8,3,4,0xa,0xa,0xa,0xa
pm_col:
    db 0xb,0xc,0xd,0xe

beam_xy:
    dd (FLD_LEFT+7) shl 16+2, FLD_TOP2+16
    dd FLD_LEFT2+16, (FLD_TOP+7) shl 16+2

ll_num db '00x.'

move_map dd -1,+0,-0,1      ; 0 - W, 1 - S, 2 - N, 3 - E

stdlev     db 'SOKO-0.LEV',0
stdlev_len:

inp_pos    dd inp_end-fn_input
entered    dd 0

file_info:
           dd 0
           dd 0
           dd 0x100
cnf_level  dd strip
           dd workarea
file_name  db CUR_DIR
path_end   db 'SKIN.'
file_num   db 'RAW',0

           rb 256-($-file_name)

I_END:  ; конец программы

winmode db ?
scanptr dd ?
levpage dd ?
workmode db ?
player dw ?
fld_size dd ?
levptr dd ?
wnd_height dd ?
wnd_width dd ?
color  db ?
colcount db ?
levelcount dd ?
checkcount db ?
checkpoint:
    times 256 dw ?
levelmap:
    times 1024 dd ?
strip rb SKIN_SIZE

workarea:
  srch  rb 0x10000-($-workarea)

level_start rb 0x20000
field:
