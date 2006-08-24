; Playlist support for MIDPLAY
LCOLOR  equ 0x00ff00
PCOLOR  equ 0xffffff

PL_X equ 10 shl 16
PL_Y equ PLY shl 16
PL_XY equ 10 shl 16+PLY

PL_show:
   test [flag],FL_HIDDEN
   jnz   .ex
   pusha
   and  [counter],0
   mov  ebx,PL_X+255
   mov  eax,13
   mov  ecx,PL_Y-1 shl 16+4+9*LISTITEMS
   xor  edx,edx
   int  0x40
   movzx  ecx,byte[list_count]
   test ecx,ecx
   jz   .ex2
   push ecx
   imul ecx,9
   add  ecx,PL_Y-1 shl 16-2
   mov  edx,11+1 shl 29+1 shl 30
   mov  eax,8
   int  0x40
   pop  ecx
   mov  edi,[pl_ptr]
   mov  eax,4
   mov  ebx,PL_XY+2 shl 16
 .sh_loop:
   movzx esi,word[edi]
   and  esi,not FL_MULSEL
   add  edi,2
   pusha
   mov  edx,edi
   mov  ecx,[counter]
   test word[edi-2],FL_MULSEL
   jz   .nosel
   pusha
   mov  ebx,PL_X+255+1 shl 16-2
   imul ecx,9
   shl  ecx,16
   add  ecx,PL_Y-1 shl 16+8
   mov  edx,0xc6
   mov  eax,13
   int  0x40
   popa
 .nosel:
   pusha
   mov  edx,ebx
   mov  esi,0xa0a0a0;0x508cec
   mcall 47,0x30000
   popa
   cmp  ecx,[play_num]
   je   .high
   mov  ecx,LCOLOR
   jmp  .int
 .high:
   mov  ecx,PCOLOR
 .int:
   add  ebx,25 shl 16
   int  0x40
   popa
   add  edi,esi
   add  ebx,9
   inc  [counter]
   loop .sh_loop2
   jmp  .ex2
 .sh_loop2:
   jmp  .sh_loop
 .ex2:
   popa
   test [flag],FL_BOTTRED
   jz   .nobott
   pusha
   mcall 7,hdr_raw+12+275*16*3,<275,12>,<10,(WND_HEIGHT-20)>
   mov  ebp,main_coo2
   mov  esi,10 shl 16
   mov  edi,(WND_HEIGHT-22)shl 16
   mov  ecx,1
   pushd 155 shl 16+5
   pushd (WND_HEIGHT-22) shl 16+5
   call draw_navigation
   add  esp,8
   popa
   call draw_bottom
   and  [flag],not FL_BOTTRED
 .nobott:
 .ex:
   ret

PL_add:
; in: esi->filename,ecx->fname_len
   jecxz .ex
   cmp  [list_count],LISTITEMS
   jae   .ex
   pusha
   mov  edi,[play_limit]
   mov  [edi],cx
   add  edi,2
   rep  movsb
   mov  [play_limit],edi
   inc  [list_count]
   popa
 .ex:
   ret

PL_del:
    movzx ecx,byte[list_count]
    jecxz .ex
    call PL_get1stsel
    mov  edx,ebx
  .lp:
    mov  edi,esi
    xor  eax,eax
    lodsw
    btr  eax,15
    jc  .elp2
    add  esi,eax
    jmp  .elp
  .elp2:
    push esi ecx
    add  esi,eax
    mov  ecx,[play_limit]
    sub  ecx,esi
    rep  movsb
    mov  [play_limit],edi
    dec  [list_count]
    cmp  edx,[play_num]
    ja   .no
    dec  [play_num]
  .no:
    pop  ecx esi
    sub  esi,2
  .elp:
    inc  edx
    loop .lp
  .ex:
    ret

PL_getitemclick:
;  out: eax- item # (0..n)
   mov  ebx,1
   mov  eax,37
   int  0x40
   sub  eax,PLY-1
   mov  ebx,9
   div  bl
   movzx eax,al
   ret

PL_getbyindex:
;  in:eax-index, out: esi-filename, ecx-length
   mov  esi,[pl_ptr]
   mov  ecx,eax
 .loop:
   lodsw
   and  eax,not FL_MULSEL
   jecxz .gbi
   add  esi,eax
   dec  ecx
   jmp  .loop
 .gbi:
   movzx ecx,ax
   ret

PL_get1stsel:
; out: esi- 1st selected, ebx - index
   mov  esi,[pl_ptr]
   xor  ebx,ebx
PL_getnextsel:
   push eax ecx
   movzx ecx,[list_count]
 .lp:
   movzx eax,word[esi]
   btr  eax,15
   jc   .ex2
   lea  esi,[esi+eax+2]
   inc  ebx
   loop .lp
   xor  ebx,ebx
 .ex2:
   pop  ecx
 .ex:
   pop  eax
   ret

PL_clearsel:
   pusha
   mov  ebx,not FL_MULSEL
   xor  eax,eax
   mov  esi,[pl_ptr]
   movzx ecx,[list_count]
   jecxz .flg
 .loop:
   and  word[esi],bx
   lodsw
   add  esi,eax
   loop .loop
 .flg:
   and  [flag],not FL_MULSEL
   popa
   ret

PL_shiftsel:
   pusha
   xor  eax,eax
;   mov  esi,[pl_ptr]
;   movzx ecx,[list_count]
 .loop:
   lodsw
   or   word[esi-2],FL_MULSEL
   add  esi,eax
   loop .loop
   jmp  PL_clearsel.flg

PL_invsel:
   pusha
   mov  ebx,not FL_MULSEL
   xor  eax,eax
   mov  esi,[pl_ptr]
   movzx ecx,[list_count]
   jecxz .ex
 .loop:
   xor  word[esi],FL_MULSEL
   lodsw
   and  eax,ebx
   add  esi,eax
   loop .loop
 .ex:
   jmp  PL_clearsel.flg

PL_load:
   and  [list_count],0
   mov  [pl_ptr],playlist
   mov  ebx,PL_info
   mov  dword[ebx+12],playlist
   mov  eax,58
   int  0x40
   test eax,eax
   jz   .ok1     ; ebx- filesize
   cmp  eax,6
   jne  .ex
 .ok1:
   mov  eax,0x0a0d
   cmp  word[playlist],ax
   je   .ok
   sub  [pl_ptr],2
 .ok:
   mov  edi,[pl_ptr]
   add  ebx,edi
   mov  word[ebx],ax
   add  edi,2
 .loo:
   mov  edx,edi
 .loo2:
   mov  ecx,256
   repne scasb
   cmp  edi,ebx
   jb   .ok2
   lea  edi,[ebx+1]
 .ok2:
   mov  ecx,edi
   sub  ecx,edx
   dec  ecx
   inc  edi
   jecxz .shift
   mov  [edx-2],cx
   inc  [list_count]
   cmp  edi,ebx
   jb   .loo
 .ex1:
   sub  ebx,2
   mov  [play_limit],ebx
 .ex:
   ret

 .shift:
   mov  ecx,ebx
   sub  ecx,edi
   jecxz .ex1
   sub  ebx,2
   mov  esi,edi
   sub  edi,2
   push edi
   rep  movsb
   pop  edi
   jmp  .loo

PL_save:
   movzx ecx,byte[list_count]
   jecxz .ex
   mov  eax,0x0a0d
   mov  edi,[pl_ptr]
   lea  edx,[edi+2]
   mov  [PL_info+12],edx
 .savl:
   movzx ebx,word[edi]
   and  bx,not FL_MULSEL
   stosw
   add  edi,ebx
   loop .savl
   stosw
   sub  edi,[pl_ptr]
   mov  ebx,PL_info
   mov  [ebx+8],edi
   mov  dword[ebx],1
   mov  eax,58
   int  0x40
 .ex:
   ret

PL_swap:  ; swap [ebx] with [ebx+1]
    mov  eax,ebx
    call PL_getbyindex
    add  ecx,2
    sub  esi,2
    push ecx esi ; save begin & length of 1st fname
    mov  edi,fnbuf
    rep  movsb
    movzx ecx,word[esi]
    and  cx,not FL_MULSEL
    add  ecx,2
    pop  edi
    rep  movsb
    pop  ecx
    mov  esi,fnbuf
    rep  movsb
    cmp  ebx,[play_num]
    jne  .nosel1
    inc  [play_num]
    ret
  .nosel1:
    inc  ebx
    cmp  ebx,[play_num]
    jne  .nosel2
    dec  [play_num]
  .nosel2:
    ret

PL_info:
 .mode  dd 0
        dd 0
 .bytes dd 20
        dd 0
        dd hash_table
        db PLAYLIST_PATH,0
