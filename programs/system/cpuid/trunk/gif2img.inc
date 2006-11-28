COLOR_ORDER equ MENUETOS
gif_hash_offset = gif_hash_area

macro gif2img gifsrc,imgsrc
{
local hasharea, ReadGIF, nextblock,_null
local globalColor, img_count, cur_info, img_start
local codesize, compsize, bit_count, CC, EOI, Palette
local block_ofs, table_ptr, gifmacend
local no_gc, block_skip, no_comm, noextblock, uselocal
local setPal, filltable, reinit, cycle, zadd, noinc
local notintable, er, zend, nxt, continue, ex, Gif_skipmap
local Gif_get_sym, shift, nextbl, noblock, loop1, exx
local Gif_output, next, loop2

_null fix 0x1000 ; 0x1000

if ~gifsrc eq esi
    mov  esi,gifsrc
end if
if ~imgsrc eq edi
    mov  edi,imgsrc
end if

    if defined gif_hash_offset
    mov  eax,gif_hash_offset
    else
    mov  eax,hasharea
    end if

    call ReadGIF
    jmp  gifmacend

    if defined gif_hash_offset
    else
    hasharea:
    times 4096 dd 0 ;4096
    end if

ReadGIF:
    push esi edi
    mov  [table_ptr],eax
    mov  [cur_info],edi
    xor  eax,eax
    mov  [globalColor],eax
    mov  [img_count],eax
    inc  eax
    cmp  dword[esi],'GIF8'
    jne  er	       ; signature
    mov  ecx,[esi+0xa]
    inc  eax
    add  esi,0xd
    mov  edi,esi
    bt	 ecx,7
    jnc  nextblock
    mov  [globalColor],esi
    call Gif_skipmap
nextblock:
    cmp  byte[edi],0x21
    jne  noextblock
    inc  edi
    cmp  byte[edi],0xf9 ; Graphic Control Ext
    jne  no_gc
    add  edi,7
    jmp  nextblock
no_gc:
    cmp  byte[edi],0xfe ; Comment Ext
    jne  no_comm
    inc  edi
block_skip:
    movzx eax,byte[edi]
    lea  edi,[edi+eax+1]
    cmp  byte[edi],0
    jnz  block_skip
    inc  edi
    jmp  nextblock
no_comm:
    cmp  byte[edi],0xff ; Application Ext
    jne  nextblock
    add  edi,13
    jmp  block_skip
noextblock:
    cmp  byte[edi],0x2c    ; image beginning
    jne  er
    inc  [img_count]
    inc  edi
    mov  esi,[cur_info]
    xchg esi,edi
;    movsd
;    movsd

    mov   bp,word[esi+4]
    movzx ebx,bp
    mov   [edi],ebx

    mov   bp,word[esi+6]
    movzx ebx,bp
    mov   [edi+4],ebx

    add edi,8
    add esi,8

    push edi
    movzx ecx,word[esi]
    inc  esi
    bt	 ecx,7
    jc	 uselocal
    push [globalColor]
    mov  edi,esi
    jmp  setPal
uselocal:
    call Gif_skipmap
    push esi
setPal:
    movzx ecx,byte[edi]
    inc  ecx
    mov  [codesize],ecx
    dec  ecx
    pop  [Palette]
    lea  esi,[edi+1]
    mov  edi,[table_ptr]
    xor  eax,eax
    cld
    lodsb		; eax - block_count
    add  eax,esi
    mov  [block_ofs],eax
    mov  [bit_count],8
    mov  eax,1
    shl  eax,cl
    mov  [CC],eax
    inc  eax
    mov  [EOI],eax
    lea  ecx,[eax-1]
    mov  eax, _null shl 16
filltable:
    stosd
    inc  eax
    loop filltable
    pop  edi
    mov  [img_start],edi
reinit:
    mov  edx,[EOI]
    inc  edx
    push [codesize]
    pop  [compsize]
    call Gif_get_sym
    cmp  eax,[CC]
    je	 reinit
    call Gif_output
cycle:
    movzx ebx,ax
    call Gif_get_sym
    cmp  eax,edx
    jae  notintable
    cmp  eax,[CC]
    je	 reinit
    cmp  eax,[EOI]
    je	 zend
    call Gif_output
zadd:
    push eax
    mov  eax,[table_ptr]
    mov  [eax+edx*4],ebx
    pop  eax
    cmp  edx,0xFFF
    jae  cycle
    inc  edx
    bsr  ebx,edx
    cmp  ebx,[compsize]
    jne  noinc
    inc  [compsize]
noinc:
    jmp  cycle
notintable:
    push eax
    mov  eax,ebx
    call Gif_output
    push ebx
    movzx eax,bx
    call Gif_output
    pop  ebx eax
    jmp  zadd
er:
    pop  edi
    jmp  ex
zend:
;    mov  eax,[.cur_info]    ; skip offset to next frame
;    mov  [eax],edi
    mov  [cur_info],edi
    add  esi,2
    xchg esi,edi
nxt:
    cmp  byte[edi],0
    jnz  continue
    inc  edi
    jmp  nxt
continue:
;    cmp  byte[edi],0x3b    ;read next frame
;    jne  nextblock
    xor  eax,eax
    stosd
    mov  ecx,[img_count]
ex:
    pop  edi esi
    ret

Gif_skipmap:
; in: ecx - image descriptor, esi - pointer to colormap
; out: edi - pointer to area after colormap

    and  ecx,111b
    inc  ecx		; color map size
    mov  ebx,1
    shl  ebx,cl
    lea  ebx,[ebx*2+ebx]
    lea  edi,[esi+ebx]
    ret

Gif_get_sym:
    mov  ecx,[compsize]
    push ecx
    xor  eax,eax
shift:
    ror  byte[esi],1
    rcr  eax,1
    dec  [bit_count]
    jnz  loop1
    inc  esi
    cmp  esi,[block_ofs]
    jb	 noblock
    push eax
    xor  eax,eax
    lodsb
    test eax,eax
    jnz  nextbl
    mov  eax,[EOI]
    sub  esi,2
    add  esp,8
    jmp  exx
nextbl:
    add  eax,esi
    mov  [block_ofs],eax
    pop  eax
noblock:
    mov  [bit_count],8
loop1:
    loop shift
    pop  ecx
    rol  eax,cl
exx:
    xor  ecx,ecx
    ret

Gif_output:
    push esi eax edx
    mov  edx,[table_ptr]
next:
    push word[edx+eax*4]
    mov  ax,word[edx+eax*4+2]
    inc  ecx
    cmp  ax,_null
    jnz  next
    shl  ebx,16
    mov  bx,[esp]
loop2:
    pop  ax

    lea  esi,[eax+eax*2]
    add  esi,[Palette]

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

    loop loop2
    pop  edx eax esi
    ret

    globalColor dd 1
    img_count dd 1
    cur_info dd 1	 ; image table pointer
    img_start dd 1
    codesize dd 1
    compsize dd 1
    bit_count dd 1
    CC dd 1
    EOI dd 1
    Palette dd 1
    block_ofs dd 1
    table_ptr dd 1

gifmacend:
}

