; GIF LITE v3.0 by Willow
; Written in pure assembler by Ivushkin Andrey aka Willow
; Modified by Diamond
;
; This include file will contain functions to handle GIF image format
;
; Created: August 15, 2004
; Last changed: June 24, 2007

; Requires kglobals.inc (iglobal/uglobal macro)
; (program must 'include "kglobals.inc"' and say 'IncludeUGlobal'
; somewhere in uninitialized data area).

; Configuration: [changed from program which includes this file]
; 1. The constant COLOR_ORDER: must be one of
;    PALETTE - for 8-bit image with palette (sysfunction 65)
;    MENUETOS - for MenuetOS and KolibriOS color order (sysfunction 7)
;    OTHER - for standard color order
; 2. Define constant GIF_SUPPORT_INTERLACED if you want to support interlaced
;    GIFs.
; 3. Single image mode vs multiple image mode:
;    if the program defines the variable 'gif_img_count' of type dword
;    somewhere, ReadGIF will enter multiple image mode: gif_img_count
;    will be initialized with image count, output format is GIF_list,
;    the function GetGIFinfo retrieves Nth image info. Otherwise, ReadGIF
;    uses single image mode: exit after end of first image, output is
;    <dd width,height, times width*height[*3] db image>

if ~ (COLOR_ORDER in <PALETTE,MENUETOS,OTHER>)
; This message may not appear under MenuetOS, so watch...
  display 'Please define COLOR_ORDER: PALETTE, MENUETOS or OTHER',13,10
end if

if defined gif_img_count
; virtual structure, used internally

struct GIF_list
    NextImg rd 1
    Left   rw 1
    Top    rw 1
    Width  rw 1
    Height rw 1
    Delay  rd 1
    Displacement rd 1 ; 0 = not specified
                       ; 1 = do not dispose
                       ; 2 = restore to background color
                       ; 3 = restore to previous
if COLOR_ORDER eq PALETTE
    Image rd 1
end if
ends

struct GIF_info
    Left   rw 1
    Top    rw 1
    Width  rw 1
    Height rw 1
    Delay  rd 1
    Displacement rd 1
if COLOR_ORDER eq PALETTE
    Palette rd 1
end if
ends

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
    lodsd
    movsd
    movsd
    movsd
    movsd
if COLOR_ORDER eq PALETTE
    lodsd
    mov   [edi],esi
else
    mov   eax,esi
end if
  .error:
    pop   edi ecx esi
    ret

end if

_null fix 0x1000

; ****************************************
;   FUNCTION ReadGIF - unpacks GIF image
; ****************************************
; in:
;   esi - pointer to GIF file in memory
;   edi - pointer to output image list

; out:
;   eax - 0, all OK;
;   eax - 1, invalid signature;
;   eax >=8, unsupported image attributes
;

ReadGIF:
    push esi edi
    mov  [.cur_info],edi
    xor  eax,eax
    mov  [.globalColor],eax
if defined gif_img_count
    mov  [gif_img_count],eax
    mov  [.anim_delay],eax
    mov  [.anim_disp],eax
end if
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
if defined gif_img_count
    cmp  byte[edi],0xf9 ; Graphic Control Ext
    jne  .no_gc
    movzx eax,word [edi+3]
    mov  [.anim_delay],eax
    mov  al,[edi+2]
    shr  al,2
    and  eax,7
    mov  [.anim_disp],eax
    add  edi,7
    jmp  .nextblock
  .no_gc:
end if
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
if defined gif_img_count
    inc  [gif_img_count]
end if
    inc  edi
    mov  esi,[.cur_info]
if defined gif_img_count
    add  esi,4
end if
    xchg esi,edi
if defined GIF_SUPPORT_INTERLACED
    movzx ecx,word[esi+4]
    mov  [.width],ecx
    movzx eax,word[esi+6]
    imul eax,ecx
if ~(COLOR_ORDER eq PALETTE)
    lea  eax,[eax*3]
end if
    mov  [.img_end],eax
    inc  eax
    mov  [.row_end],eax
    and  [.pass],0
    test byte[esi+8],40h
    jz   @f
if ~(COLOR_ORDER eq PALETTE)
    lea  ecx,[ecx*3]
end if
    mov  [.row_end],ecx
@@:
end if
if defined gif_img_count
    movsd
    movsd
    mov  eax,[.anim_delay]
    stosd
    mov  eax,[.anim_disp]
    stosd
else
    movzx eax,word[esi+4]
    stosd
    movzx eax,word[esi+6]
    stosd
    add   esi,8
end if
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
if ~(COLOR_ORDER eq PALETTE)
    pop  [.Palette]
end if
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
    mov  eax, _null shl 16
  .filltable:
    stosd
    inc  eax
    loop .filltable
if COLOR_ORDER eq PALETTE
    pop  eax
    pop  edi
    push edi
    scasd
    push esi
    mov  esi,eax
    mov  ecx,[.CC]
@@:
    lodsd
    dec  esi
    bswap eax
    shr  eax,8
    stosd
    loop @b
    pop  esi
    pop  eax
    mov  [eax],edi
else
    pop  edi
end if
if defined GIF_SUPPORT_INTERLACED
    mov  [.img_start],edi
    add  [.img_end],edi
    add  [.row_end],edi
end if
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
if defined GIF_SUPPORT_INTERLACED
    mov  edi,[.img_end]
end if
if defined gif_img_count
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
    xchg esi,edi
    and  dword [eax],0
end if
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
    cmp  ax,_null
    jnz  .next
    shl  ebx,16
    mov  bx,[esp]
  .loop2:
    pop  ax

    if COLOR_ORDER eq PALETTE
        stosb
    else
        lea  esi,[eax+eax*2]
        add  esi,[.Palette]

    if COLOR_ORDER eq MENUETOS
        mov  esi,[esi]
        bswap esi
        shr  esi,8
        mov  [edi],esi
        add  edi,3
    else
        movsb
        movsb
        movsb
    end if
    end if

if defined GIF_SUPPORT_INTERLACED
    cmp  edi,[.row_end]
    jb   .norowend
    mov  eax,[.width]
if ~(COLOR_ORDER eq PALETTE)
    lea  eax,[eax*3]
end if
    push eax
    sub  edi,eax
    add  eax,eax
    cmp  [.pass],3
    jz   @f
    add  eax,eax
    cmp  [.pass],2
    jz   @f
    add  eax,eax
@@:
    add  edi,eax
    pop  eax
    cmp  edi,[.img_end]
    jb   .nextrow
    mov  edi,[.img_start]
    inc  [.pass]
    add  edi,eax
    cmp  [.pass],3
    jz   @f
    add  edi,eax
    cmp  [.pass],2
    jz   @f
    add  edi,eax
    add  edi,eax
@@:
.nextrow:
    add  eax,edi
    mov  [.row_end],eax
    xor  eax,eax
.norowend:
end if

    loop .loop2
    pop  edx eax esi
    ret

uglobal
align 4
    ReadGIF.globalColor rd 1
    ReadGIF.cur_info rd 1        ; image table pointer
    ReadGIF.codesize rd 1
    ReadGIF.compsize rd 1
    ReadGIF.bit_count rd 1
    ReadGIF.CC rd 1
    ReadGIF.EOI rd 1
if ~(COLOR_ORDER eq PALETTE)
    ReadGIF.Palette rd 1
end if
    ReadGIF.block_ofs rd 1
if defined GIF_SUPPORT_INTERLACED
    ReadGIF.row_end rd 1
    ReadGIF.img_end rd 1
    ReadGIF.img_start rd 1
    ReadGIF.pass rd 1
    ReadGIF.width rd 1
end if
if defined gif_img_count
    ReadGIF.anim_delay rd 1
    ReadGIF.anim_disp rd 1
end if
    ReadGIF.gif_workarea rb 16*1024
endg
