key_codes db 27,182,109,115,114,122,98,118,108,120,99,13,8,32,0,    0, 0,0
   db 180,181,93,91
; bottom actions
   db  0, 0, 0, 0, 0,185,0, 0, 0, 0,0,0,92
btn_codes db  1,  0,  0,  9, 10,  3, 7,  6,  8,  4, 5, 0,0, 0,101,100,11,2
   db   0,  0, 0, 0
; bottom actions
   db 35,49,48,37,31,32,41,42,40,36,103,104,0
jmps dw  _close, kDel,kMute, bList.shuf, bList.repe, bList.prev, bList.next
   dw  bList.stop, kLoad, Xpress, Rewind.space, auto_load,Rewind, Rewind.space
   dw  bReverse, bShade, bList, bBar, bOctU, bOctD, bSelTrack, bSelChannel
; bottom actions
   dw kDel,bLsave,bLload,bRemAll,bAdd,bAddDir,bSelZero,bInvSel,bSelAll,bRemCrop
   dw  bMinimize,bAbout,bResetTrk

jmps_end:

key:
    mov  eax,2
    mcall
    mov  edi,key_codes
    cmp  ah,48
    jb   .jump
    cmp  ah,57
    ja   .jump
    sub  ah,48
    mov  byte[channel],ah
    jmp  midi_kill
  .jump:
    mov  ecx,(jmps_end-jmps)/2
    mov  ebx,edi
    shr  eax,8
    repne scasb
    jne  play_.noplsel
    sub  edi,ebx
    jmp  word[jmps+edi*2-2]
  kDel:
    call PL_del
    call PL_show
    jmp  still
  kMute:
    xor  [flag],FL_MUTE
    jmp  still
  kLoad:
;    dps  'Load'
    cmp  [list_count],LISTITEMS
    jbe  noparam
    jmp  still
  Xpress:
    mov  eax,IPC_PLAY
    call ipc_send
    jmp  still;Rewind.space
 auto_load:
    call PL_get1stsel
    mov  eax,ebx
  play_:
    mov  [play_num],eax
    call PL_getbyindex
    mov  edi,filename;fnbuf
    rep  movsb
    xor  eax,eax
    mov  byte[edi],al
    and  [flag],not FL_ADD
    or   [flag],FL_PLAY
    jmp  open_file
  .noplsel:
    call PL_get1stsel
    test ebx,ebx
    jz   .noupward
    cmp  al,178
    jne  .noupa
  .seldec:
    dec  ebx;[listsel]
  .listdraw:
    call PL_clearsel
    mov  eax,ebx
    call PL_getbyindex
    or   word[esi-2],FL_MULSEL
    call PL_show
    jmp  still
  .noupa:
    cmp  al,184
    jne  .noupward
    dec  [listsel]
  .swap:
    dec  ebx
    call PL_swap
    jmp  .listdraw
  .noupward:
    inc  ebx
    cmp  bl,[list_count]
    jae  still
    cmp  al,177
    je  .listdraw
  .nodowna:
    cmp  al,183
    jne  still
    inc  [listsel]
    jmp  .swap
  Rewind:
    push [midi_limit]
    pop  [cur_ptr]
    and  [cur_tick],0
    jmp  red
  .space:
    cmp  [tick_count],0
    jz   still
    mov  eax,IPC_TRIG
    call ipc_send
    jmp  red

  button:
    mov  eax,17
    mcall
    cmp  ah,20
    jb   .nobott
    cmp  ah,25
    ja   .nobott
    sub  ah,20
    mov  [popup],ah
  .sh:
    or   [flag],FL_BOTTRED
    call PL_show
    jmp  still
  .nobott:
    cmp  ah,31
    jb   .nopop
    cmp  ah,50
    ja   .nopop
    mov  [popup],-1
    or   [flag],FL_BOTTRED
    call PL_show

SH_POPUP equ 10
  if SH_POPUP eq 1
    mov  ebx,eax
    shr  ebx,8
    dps  'Popup#='
    dpd  ebx
    jmp  .sh
  end if
  .nopop:
    mov  edi,btn_codes
    jmp  key.jump
  bReverse:
    xor  [flag],FL_REVERSE
    call draw_bar
    jmp  still
  _close:
    call kill
    or   eax,-1
    mcall

  bShade:
    call Shade
    jmp  still
  Shade:
    xor  [flag],FL_HIDDEN
    test [flag],FL_HIDDEN
    jz  .open
    mov  esi,14
    mov  [coo],main_coo2
    jmp  .op
  .open:
    mov  esi,WND_HEIGHT
    mov  [coo],main_coo
  .op:
    mov  ebx,-1
    mov  ecx,ebx
    mov  edx,ebx
    mov  eax,67
    mcall
    ret
  bMinimize:
;    mcall 18,10
    jmp  still
  bList:
    mov  [popup],-1
    mov  edx,[listsel]
    call PL_getitemclick
    mov  [listsel],eax
    mcall 66,3
    mov  ebx,eax
;    dph  ebx
    test ebx,1100b
    jnz  .mul
    call PL_clearsel
    test ebx,11b
    jz   .skipor
    mov  ecx,[listsel]
    mov  [listsel],edx
    cmp  ecx,edx
    je   .skipor
    ja   .above
    xchg ecx,edx
  .above:
    sub  ecx,edx
    inc  ecx
    mov  eax,edx
    push ecx
    call PL_getbyindex
    sub  esi,2
    pop  ecx
    call PL_shiftsel
    jmp  .plsh
  .mul:
    bts  [flag],15
    jc   .skipor
    mov  eax,edx
    call PL_getbyindex
    or  word[esi-2],FL_MULSEL
  .skipor:
    mov  eax,[listsel]
    call PL_getbyindex
    xor  word[esi-2],FL_MULSEL
  .plsh:
    call PL_show
    mcall 40,111b
    mcall 23,30
    push eax
    mcall 40,1000111b
    pop  eax
    cmp  eax,3
    jne  still
    mcall 17
    cmp  ah,11
    jne  still
    call PL_getitemclick
    cmp  eax,[listsel]
    je  auto_load
    mov  [listsel],eax
    call PL_show
    jmp  still
  .repe:
    xor  [flag],FL_REPEAT
    jmp  red
  .stop:
    mov  eax,IPC_PAUS
    call ipc_send
    jmp  Rewind
  .shuf:
    xor  [flag],FL_SHUFFLE
    jmp  red
  .prev:
    mov  eax,[play_num]
    test eax,eax
    jz   still
    dec  eax
    jmp  play_
  .next:
    mov  eax,[play_num]
    inc  eax
    cmp  al,[list_count]
    jae  still
    jmp  play_
  bBar:
    cmp  [tick_count],0
    jz   still
    mov  eax,37
    mov  ebx,1
    mcall
    mov  ebp,[coo]
    sub  eax,[ebp-16]
    shr  eax,16
    mov  ebx,[tick_count]
    mul  ebx
    mov  ebx,[ebp-20]
    div  ebx
    mov  ebx,eax ; ebx - selected tick
    xor  ecx,ecx
    mov  esi,[midi_limit]
  .further:
    lodsw
    and  eax,0x7f
    add  ecx,eax
    cmp  ecx,ebx
    jb   .further
    sub  ecx,eax
    mov  [cur_tick],ecx
    sub  esi,2
    mov  [cur_ptr],esi
  drw:
    jmp  red

OCT_CH equ 3
  bOctU:
    add  [octave],OCT_CH
    jmp  midi_kill
  bOctD:
    sub  [octave],OCT_CH
    jmp  midi_kill
  bSelChannel:
    call get_num
    cmp  eax,-1
    je   still
    dps  <13,10,'Channel#'>
    dpd  eax
    mov  [channel],eax
    jmp  midi_kill
  bSelTrack:
    call get_num
    cmp  eax,-1
    je   still
    dps  <13,10,'Track#'>
    dpd  eax
    mov  [sel_track],al
    jmp  midi_kill
  bResetTrk:
    xor  eax,eax
    mov  [sel_track],al
    mov  [channel],eax
    dps  <13,10,'Both track & channel are 0',13,10>
    jmp  midi_kill
  bLsave:
    call PL_save
  bLload:
    call PL_load
    jmp  red
  bRemAll:
    and  [list_count],0
    mov  [play_limit],playlist-2
    jmp  red
  bAdd:
    movzx eax,[list_count]
    dpd  eax
    call fopen
    mov  edi,filename
    cmp  byte[edi],0
    jz   still
    call str_len
    xchg eax,ecx
    mov  esi,edi
    jmp  add_song
  bAddDir:
    call fopen
    mov  edi,filename
    cmp  byte[edi],0
    jz   still
    call find_slash
    lea  edx,[edi-filename+fnbuf] ; edx -> '/'
    and  byte[edi],0
    mov  esi,filename
    mov  edi,fnbuf
    mov  ecx,1024/4
    rep  movsd
        and     dword [dir_info+4], 0
  .nxt2:
        mcall   70, dir_info
        cmp     ebx, 1
        jnz     red ;still
        test    byte [dir_table+32], 11000b     ; exclude directories & vol label
        jnz     .no
        mov     edi, dir_table+32+40
        mov     esi, edi
        xor     eax, eax
        or      ecx, -1
        repnz   scasb
        mov     eax, [edi-5]
        or      eax, 0x20202000
        cmp     eax, '.mid'
        jz      .ok
        cmp     eax, '.kar'
        jnz     .no
.ok:
        lea     ecx, [edi-dir_table-32-40-1]
        mov     edi, edx
        rep     movsb
        mov     ecx, edi
        mov     esi, fnbuf
        sub     ecx, esi
        call    PL_add
.no:
        inc     dword [dir_info+4]
        jmp     .nxt2
  bSelZero:
    call PL_clearsel
    jmp  red
  bInvSel:
    call PL_invsel
    jmp  red
  bSelAll:
    call PL_clearsel
    call PL_invsel
    jmp  red
  bRemCrop:
    call PL_invsel
    call PL_del
    jmp  bSelAll
  bAbout:
    mov  edx,about1
    call debug_outstr
    jmp  still

get_num:  ; out: eax-number entered
    or   [flag],FL_LOCK
    mcall 10
    dpd  eax
    cmp  eax,1
    jne  .nored
    call draw_window
    jmp  get_num
  .nored:
    cmp  eax,2
    jne  .nokey
    mcall 2
    movzx eax,ah
    sub  eax,'0'
    jl   .none
    cmp  eax,9
    jbe  .ok
  .none:
    mov  eax,-1
  .ok:
    and  [flag],not FL_LOCK
    ret
  .nokey:
    cmp  eax,3
    jne  get_num
    mcall 17
    jmp  get_num

find_slash:
; in: edi-filename, out: edi-slash ptr-1
    push eax ecx
    call str_len
    mov  ecx,eax
    std
    add  edi,eax
    mov  al,'/'
    repne scasb
    cld
    add  edi,2
    pop  ecx eax
    ret

