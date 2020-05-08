
uglobal
align 4
ReadGIF.globalColor dd ?
ReadGIF.cur_info    dd ?    ; image table pointer
ReadGIF.codesize    dd ?
ReadGIF.compsize    dd ?
ReadGIF.bit_count   dd ?
ReadGIF.CC          dd ?
ReadGIF.EOI         dd ?
ReadGIF.Palette     dd ?
ReadGIF.block_ofs   dd ?
ReadGIF.gif_workarea    rb 16*1024
endg

; unpacks GIF image
ReadGIF:
;   in:
; esi - pointer to GIF file in memory
; edi - pointer to output image list
;   out:
; eax=0 -> ok, eax=1 -> invalid signature
; eax>=8 -> unsupported image attributes
    push esi edi
    mov  [.cur_info],edi
    xor  eax,eax
    mov  [.globalColor],eax
    inc  eax
    cmp  dword[esi],'GIF8'
    jne  .ex            ; signature
    mov  ecx,[esi+0xa]
    add  esi,0xd
    mov  edi,esi
    test cl,cl
    jns  .nextblock
    mov  [.globalColor],esi
    call .Gif_skipmap
  .nextblock:
    cmp  byte[edi],0x21
    jne  .noextblock
    inc  edi
    inc  edi
  .block_skip:
    movzx eax,byte[edi]
    lea  edi,[edi+eax+1]
    test eax,eax
    jnz  .block_skip
    jmp  .nextblock
  .noextblock:
    mov  al,8
    cmp  byte[edi],0x2c    ; image beginning
    jne  .ex
    inc  edi
    mov  esi,[.cur_info]
    xchg esi,edi
    movzx eax,word[esi+4]
    stosd
    movzx eax,word[esi+6]
    stosd
    add   esi,8
    push edi
    mov  ecx,[esi]
    inc  esi
    test cl,cl
    js   .uselocal
    push [.globalColor]
    mov  edi,esi
    jmp  .setPal
  .uselocal:
    call .Gif_skipmap
    push esi
  .setPal:
    movzx ecx,byte[edi]
    inc  ecx
    mov  [.codesize],ecx
    dec  ecx
    pop  [.Palette]
    lea  esi,[edi+1]
    mov  edi,.gif_workarea
    xor  eax,eax
    lodsb               ; eax - block_count
    add  eax,esi
    mov  [.block_ofs],eax
    mov  [.bit_count],8
    mov  eax,1
    shl  eax,cl
    mov  [.CC],eax
    mov  ecx,eax
    inc  eax
    mov  [.EOI],eax
    mov  eax, 1000h shl 16
  .filltable:
    stosd
    inc  eax
    loop .filltable
    pop  edi
  .reinit:
    mov  edx,[.EOI]
    inc  edx
    push [.codesize]
    pop  [.compsize]
    call .Gif_get_sym
    cmp  eax,[.CC]
    je   .reinit
    call .Gif_output
  .cycle:
    movzx ebx,ax
    call .Gif_get_sym
    cmp  eax,edx
    jae  .notintable
    cmp  eax,[.CC]
    je   .reinit
    cmp  eax,[.EOI]
    je   .end
    call .Gif_output
  .add:
    mov  dword [.gif_workarea+edx*4],ebx
    cmp  edx,0xFFF
    jae  .cycle
    inc  edx
    bsr  ebx,edx
    cmp  ebx,[.compsize]
    jne  .noinc
    inc  [.compsize]
  .noinc:
    jmp  .cycle
  .notintable:
    push eax
    mov  eax,ebx
    call .Gif_output
    push ebx
    movzx eax,bx
    call .Gif_output
    pop  ebx eax
    jmp  .add
  .end:
    xor  eax,eax
  .ex:
    pop  edi esi
    ret

.Gif_skipmap:
; in: ecx - image descriptor, esi - pointer to colormap
; out: edi - pointer to area after colormap
    and  ecx,111b
    inc  ecx            ; color map size
    mov  ebx,1
    shl  ebx,cl
    lea  ebx,[ebx*2+ebx]
    lea  edi,[esi+ebx]
    ret

.Gif_get_sym:
    mov  ecx,[.compsize]
    push ecx
    xor  eax,eax
  .shift:
    ror  byte[esi],1
    rcr  eax,1
    dec  [.bit_count]
    jnz  .loop1
    inc  esi
    cmp  esi,[.block_ofs]
    jb   .noblock
    push eax
    xor  eax,eax
    lodsb
    test eax,eax
    jnz  .nextbl
    mov  eax,[.EOI]
    sub  esi,2
    add  esp,8
    jmp  .exx
  .nextbl:
    add  eax,esi
    mov  [.block_ofs],eax
    pop  eax
  .noblock:
    mov  [.bit_count],8
  .loop1:
    loop .shift
    pop  ecx
    rol  eax,cl
  .exx:
    xor  ecx,ecx
    ret

.Gif_output:
    push esi eax edx
    mov  edx,.gif_workarea
  .next:
    push word[edx+eax*4]
    mov  ax,word[edx+eax*4+2]
    inc  ecx
    cmp  ax,1000h
    jnz  .next
    shl  ebx,16
    mov  bx,[esp]
  .loop2:
    pop  ax
    lea  esi,[eax+eax*2]
    add  esi,[.Palette]
    mov  esi,[esi]
    bswap esi
    shr  esi,8
    mov  [edi],esi
    add  edi,3
    loop .loop2
    pop  edx eax esi
    ret
