; GIF LITE v2.0 by Willow
; Written in pure assembler by Ivushkin Andrey aka Willow
;
; This include file will contain functions to handle GIF image format
;
; Created: August 15, 2004
; Last changed: August 25, 2006

; Change COLOR_ORDER in your program
; if colors are displayed improperly

if ~ (COLOR_ORDER in <MENUETOS,OTHER>)
; This message may not appear under MenuetOS, so watch...
  display 'Please define COLOR_ORDER: MENUETOS or OTHER',13,10
end if

; virtual structure, used internally

struc GIF_list
{
    .NextImg rd 1
    .Left   rw 1
    .Top    rw 1
    .Width  rw 1
    .Height rw 1
}

struc GIF_info
{
    .Left   rw 1
    .Top    rw 1
    .Width  rw 1
    .Height rw 1
}

_null fix 0x1000

; ****************************************
;   FUNCTION GetGIFinfo - retrieve Nth image info
; ****************************************
; in:
;   esi - pointer to image list header
;   ecx - image_index (0...img_count-1)
;   edi - pointer to GIF_info structure to be filled

; out:
;   eax - pointer to RAW data, or 0, if error

GetGIFinfo:
    push  esi ecx edi
    xor   eax,eax
    jecxz .eloop
  .lp:
    mov   esi,[esi]
    test  esi,esi
    jz    .error
    loop  .lp
  .eloop:
    add   esi,4
    movsd
    movsd
    mov   eax,esi
  .error:
    pop   edi ecx esi
    ret

; ****************************************
;   FUNCTION ReadGIF - unpacks GIF image
; ****************************************
; in:
;   esi - pointer to GIF file in memory
;   edi - pointer to output image list
;   eax - pointer to work area (MIN 16 KB!)

; out:
;   eax - 0, all OK;
;   eax - 1, invalid signature;
;   eax >=8, unsupported image attributes
;
;   ecx - number of images

ReadGIF:
    push esi edi
    mov  [.table_ptr],eax
    mov  [.cur_info],edi
    xor  eax,eax
    mov  [.globalColor],eax
    mov  [.img_count],eax
    inc  eax
    cmp  dword[esi],'GIF8'
    jne  .ex            ; signature
    mov  ecx,[esi+0xa]
    inc  eax
    add  esi,0xd
    mov  edi,esi
    bt   ecx,7
    jnc  .nextblock
    mov  [.globalColor],esi
    call .Gif_skipmap
  .nextblock:
    cmp  byte[edi],0x21
    jne  .noextblock
    inc  edi
    cmp  byte[edi],0xf9 ; Graphic Control Ext
    jne  .no_gc
    add  edi,7
    jmp  .nextblock
  .no_gc:
    cmp  byte[edi],0xfe ; Comment Ext
    jne  .no_comm
    inc  edi
  .block_skip:
    movzx eax,byte[edi]
    lea  edi,[edi+eax+1]
    cmp  byte[edi],0
    jnz  .block_skip
    inc  edi
    jmp  .nextblock
  .no_comm:
    cmp  byte[edi],0xff ; Application Ext
    jne  .nextblock
    add  edi,13
    jmp  .block_skip
  .noextblock:
    cmp  byte[edi],0x2c    ; image beginning
    jne  .ex
    inc  [.img_count]
    inc  edi
    mov  esi,[.cur_info]
    add  esi,4
    xchg esi,edi
    movsd
    movsd
    push edi
    movzx ecx,word[esi]
    inc  esi
    bt   ecx,7
    jc   .uselocal
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
    mov  edi,[.table_ptr]
    xor  eax,eax
    cld
    lodsb               ; eax - block_count
    add  eax,esi
    mov  [.block_ofs],eax
    mov  [.bit_count],8
    mov  eax,1
    shl  eax,cl
    mov  [.CC],eax
    inc  eax
    mov  [.EOI],eax
    lea  ecx,[eax-1]
    mov  eax, _null shl 16
  .filltable:
    stosd
    inc  eax
    loop .filltable
    pop  edi
    mov  [.img_start],edi
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
    push eax
    mov  eax,[.table_ptr]
    mov  [eax+edx*4],ebx
    pop  eax
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
    mov  eax,[.cur_info]
    mov  [eax],edi
    mov  [.cur_info],edi
    add  esi,2
    xchg esi,edi
  .nxt:
    cmp  byte[edi],0
    jnz  .continue
    inc  edi
    jmp  .nxt
  .continue:
    cmp  byte[edi],0x3b
    jne  .nextblock
    xor  eax,eax
    stosd
    mov  ecx,[.img_count]
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
    mov  edx,[.table_ptr]
  .next:
    push word[edx+eax*4]
    mov  ax,word[edx+eax*4+2]
    inc  ecx
    cmp  ax,_null
    jnz  .next
    shl  ebx,16
    mov  bx,[esp]
  .loop2:
    pop  ax

    lea  esi,[eax+eax*2]
    add  esi,[.Palette]

    if COLOR_ORDER eq MENUETOS
        mov  esi,[esi]
        bswap esi
        shr  esi,8
        mov  [edi],esi
        add  edi,3
    else
        movsw
        movsb
    end if

    loop .loop2
    pop  edx eax esi
    ret

    .globalColor rd 1
    .img_count rd 1
    .cur_info rd 1        ; image table pointer
    .img_start rd 1
    .codesize rd 1
    .compsize rd 1
    .bit_count rd 1
    .CC rd 1
    .EOI rd 1
    .Palette rd 1
    .block_ofs rd 1
    .table_ptr rd 1
