lang equ ru

;
;   Assembler
;     SMALL
;       CODE
;         Graphics
;           Libary
;
;   Ver 0.18 By Pavlushin Evgeni (RUSSIA)
;   www.waptap@mail.ru

;InfoList
;0.01 LoadImage
;0.02 SetBmp
;0.03 Bmptoimg, Setimg ~01.03.2004
;0.04 Bug deleted, copyimg ~03.05.2004
;0.05 fullimg, collimg ~05.05.2004
;0.06 getimg ~09.05.2004
;0.07 convbmp ~13.05.2004
;0.08 fps ~14.05.2004
;0.09 drawfbox ~03.06.2004
;0.10 all macros optimized by halyavin, add at ~07.06.2004
;0.11 many macros optimized by halyavin, add at ~30.08.2004
;0.12 bmptoimg ~07.09.2004
;0.13 imgtoimg ~08.09.2004
;0.14 imgtoimg modify not brake bmp pict! ~09.09.2004
;0.15 giftoimg, giftoani ~10.09.2004
;0.16 setframe, rgbtobgr, setbmp deleted ~20.09.2004
;0.17 modification giftoimg, giftoani, getframeoff ~01.10.2004
;0.18 aframetoimg,aimgtoimg,frametoimg ~03.10.2004

aframetoimg_use_count=0
macro aframetoimg img, x, y, canvas,acol
{
local loo,loo2,acolor
aframetoimg_use_count=aframetoimg_use_count+1
if aframetoimg_use_count = 1

     jmp end_aframetoimg_proc

acolor dd 0
aframetoimg_proc:
;getout coord
    mov [acolor],ebp

    mov edx,ebx ;img   ;xsize
    movzx eax,word [edx]
    add eax,esi ;y cor

;    mov eax,esi ;y cor
    mul dword [ecx] ;canvas xsize
    add eax,edi ;x cor

    mov ebp,ebx ;img   ;xsize
    movzx edx,word [ebp]
    add eax,edx

    mov ebp,eax
    shl eax,1
    add ebp,eax
    add ebp,ecx ;canvas+8;start
    add ebp,8
;get img size
    add ebx,4
    mov eax,ebx ;img   ;xsize
    movzx esi,word [eax]
    movzx edi,word [eax+2]
    add ebx,4
    mov edx,ebx ;img+8
loo2:
push esi
loo:
;test on alpha color
    mov eax,[edx]
    shl eax,8
    shr eax,8
    cmp eax,[acolor]
    jne  yx
    add edx,3
    add ebp,3
    jmp nx
yx:

    mov al,byte [edx]
    mov byte [ebp],al
    inc ebp
    inc edx
    mov al,byte [edx]
    mov byte [ebp],al
    inc ebp
    inc edx
    mov al,byte [edx]
    mov byte [ebp],al
    inc ebp
    inc edx
nx:
    dec esi
    jnz loo
pop esi
    sub ebp,3
    mov eax,[ecx]  ;offset = offset+((canxsize-imgxsize)*3)
    sub eax,esi
    add ebp,eax
    shl eax,1
    add ebp,eax

    add ebp,3

    dec edi
    jnz loo2
    ret
end_aframetoimg_proc:
end if
    push img
    push canvas
    push x
    push y
    push acol
    pop  ebp
    pop  esi
    pop  edi
    pop ecx
    pop ebx
    call aframetoimg_proc
}

frametoimg_use_count=0
macro frametoimg img, x, y, canvas
{
local loo,loo2
frametoimg_use_count=frametoimg_use_count+1
if frametoimg_use_count = 1

    jmp end_frametoimg_proc

frametoimg_proc:
;getout coord
    mov edx,ebx ;img   ;xsize
    movzx eax,word [edx]
    add eax,esi ;y cor

;    mov eax,esi ;y cor
    mul dword [ecx] ;canvas xsize
    add eax,edi ;x cor

    mov ebp,ebx ;img   ;xsize
    movzx edx,word [ebp]
    add eax,edx

    mov ebp,eax
    shl eax,1
    add ebp,eax
    add ebp,ecx ;canvas+8;start
    add ebp,8
;get img size
    add ebx,4
    mov eax,ebx ;img   ;xsize
    movzx esi,word [eax]
    movzx edi,word [eax+2]
    add ebx,4
    mov edx,ebx ;img+8
loo2:
push esi
loo:
    mov al,byte [edx]
    mov byte [ebp],al
    inc ebp
    inc edx
    mov al,byte [edx]
    mov byte [ebp],al
    inc ebp
    inc edx
    mov al,byte [edx]
    mov byte [ebp],al
    inc ebp
    inc edx

    dec esi
    jnz loo
pop esi
    sub ebp,3
    mov eax,[ecx]  ;offset = offset+((canxsize-imgxsize)*3)
    sub eax,esi
    add ebp,eax
    shl eax,1
    add ebp,eax

    add ebp,3

    dec edi
    jnz loo2
    ret
end_frametoimg_proc:
end if
    push img
    push canvas
    push x
    push y
    pop  esi
    pop  edi
    pop ecx
    pop ebx
    call frametoimg_proc
}


aimgtoimg_use_count=0
macro aimgtoimg img, x, y, canvas,acol
{
local loo,loo2,acolor
aimgtoimg_use_count=aimgtoimg_use_count+1
if aimgtoimg_use_count = 1

     jmp end_aimgtoimg_proc

acolor dd 0
aimgtoimg_proc:
;getout coord
    mov [acolor],ebp

    mov eax,esi ;y cor
    mul dword [ecx] ;canvas xsize
    add eax,edi ;x cor
    mov ebp,eax
    shl eax,1
    add ebp,eax
    add ebp,ecx ;canvas+8;start
    add ebp,8
;get img size
    mov eax,ebx ;img   ;xsize
    mov esi,[eax]
    add ebx,4
    mov eax,ebx ; img+4 ;ysize
    mov edi,[eax]
    add ebx,4
    mov edx,ebx ;img+8
loo2:
push esi
loo:

;test on alpha color
    mov eax,[edx]
    shl eax,8
    shr eax,8
    cmp eax,[acolor]
    jne  yx
    add edx,3
    add ebp,3
    jmp nx
yx:

    mov al,byte [edx]
    mov byte [ebp],al
    inc ebp
    inc edx
    mov al,byte [edx]
    mov byte [ebp],al
    inc ebp
    inc edx
    mov al,byte [edx]
    mov byte [ebp],al
    inc ebp
    inc edx
nx:
    dec esi
    jnz loo
pop esi
    sub ebp,3
    mov eax,[ecx]  ;offset = offset+((canxsize-imgxsize)*3)
    sub eax,esi
    add ebp,eax
    shl eax,1
    add ebp,eax

    add ebp,3

    dec edi
    jnz loo2
    ret
end_aimgtoimg_proc:
end if
    push img
    push canvas
    push x
    push y
    push acol
    pop  ebp
    pop  esi
    pop  edi
    pop ecx
    pop ebx
    call aimgtoimg_proc
}




imgtoimg_use_count=0
macro imgtoimg img, x, y, canvas
{
local loo,loo2
imgtoimg_use_count=imgtoimg_use_count+1
if imgtoimg_use_count = 1

     jmp end_imgtoimg_proc
imgtoimg_proc:
;getout coord
    mov eax,esi ;y cor
    mul dword [ecx] ;canvas xsize
    add eax,edi ;x cor
    mov ebp,eax
    shl eax,1
    add ebp,eax
    add ebp,ecx ;canvas+8;start
    add ebp,8
;get img size
    mov eax,ebx ;img   ;xsize
    mov esi,[eax]
    add ebx,4
    mov eax,ebx ; img+4 ;ysize
    mov edi,[eax]
    add ebx,4
    mov edx,ebx ;img+8
loo2:
push esi
loo:
    mov al,byte [edx]
    mov byte [ebp],al
    inc ebp
    inc edx
    mov al,byte [edx]
    mov byte [ebp],al
    inc ebp
    inc edx
    mov al,byte [edx]
    mov byte [ebp],al
    inc ebp
    inc edx
    dec esi
    jnz loo
pop esi
    sub ebp,3
    mov eax,[ecx]  ;offset = offset+((canxsize-imgxsize)*3)
    sub eax,esi
    add ebp,eax
    shl eax,1
    add ebp,eax

    add ebp,3

    dec edi
    jnz loo2
    ret
end_imgtoimg_proc:
end if
    push img
    push canvas
    push x
    push y
    pop  esi
    pop  edi
    pop  ecx
    pop  ebx
    call imgtoimg_proc
}


;DrawBox
macro drawfbox x,y,xs,ys,color
{
    wordstoreg ebx,x,xs ;x*65536+xs
    wordstoreg ecx,y,ys ;y*65536+ys
    mov  edx,color
    mov  eax,13
    int  0x40
}

; FPS - Set Frame Per Second Display
fps_show_frequency=40
macro fps x,y,color,delcolor
{
local spdat,savetime,new_time,fps,fps_cntr,out_fps,new_time,ttt
local no_out_fps
    jmp spdat
savetime dd 0
fps_cntr dd 0
fps      dd 0
ttt      dd 0
spdat:
get_time:
    mov eax,3
    int 0x40
    cmp eax,[savetime]
    jne new_time
    inc [fps_cntr]
    cmp dword [ttt],0
    je  out_fps
    dec dword [ttt]
    jmp no_out_fps
new_time:
    mov [savetime],eax
    mov ebx,[fps_cntr]
    mov [fps],ebx
    mov [fps_cntr],0
out_fps:
if ~(delcolor eq )
    mov ebx,x*65536+30
    mov ecx,y*65536+7
    mov edx,delcolor
    mov eax,13
    int 0x40
end if
    mov dword [ttt],fps_show_frequency
    mov eax,47
    mov ebx,5*65536
;   mov bl,0
    mov edx,x*65536+y
    mov esi,color
    mov ecx,[fps]
    int 0x40
no_out_fps:
}

; COLLIMG - Collusion image's
_1dbounce_count=0;
macro collimg img1_off,x1,y1,img2_off,x2,y2,otv
{
local bounce,exit,anot,bc,nbc
	mov esi,[img1_off] ;xs1
	mov edi,[img2_off] ;ys2
	mov eax,x1 ;
	mov ebx,x2 ;
	call _1dbounce
	mov edx,ecx
	mov esi,[img1_off+4] ;ys1
	mov edi,[img2_off+4] ;ys2
	mov eax,y1 ;
	mov ebx,y2 ;
	call _1dbounce
	add edx,ecx
	cmp edx,2
	je bounce
	mov otv,0
	jmp exit
_1dbounce_count=_1dbounce_count+1
if _1dbounce_count = 1
_1dbounce:
	cmp ebx,eax
	jb  anot
	add eax,esi
	cmp eax,ebx
	jbe nbc
bc:
	mov ecx,1
	ret
anot:
    add ebx,edi
	cmp ebx,eax
	ja  bc
nbc:
	xor ecx,ecx
	ret
end if
bounce:	
	mov otv,1
exit:	
}

macro rgbtobgr image
{
local loo
    mov eax,[image]
    mul dword [image+4]
    mov ecx,eax
    mov esi,image+8
;   add esi,8
loo:
   mov al,[esi]
   mov bl,[esi+2]
   mov [esi],bl
   mov [esi+2],al
   add esi,3
   dec ecx
   jnz loo
}


macro setimg x , y ,arg3
{
    mov  eax,7
    mov  ebx,arg3
    add  ebx,8
    mov  cx,[arg3]
    shl  ecx,16
    add  cx,[arg3+4]
;    wordstoreg ecx,[arg3],[arg3+4]
    wordstoreg edx, x , y  ;arg1*65536+arg2
    int  0x40
}

macro setframe x , y ,arg3
{
    mov  eax,7
    mov  ebx,arg3
    add  ebx,8
    wordstoreg edx, x , y  ;arg1*65536+arg2
    add  edx,dword [arg3]
    mov  ecx,dword [arg3+4]
    int  0x40
}


macro getimg imgsrc,x,y,xs,ys,imgdest
{
local cyc
if xs eqtype 0
    mov dword [imgdest],xs
else
    mov eax,xs
    mov dword [imgdest],eax
end if
if ys eqtype 0
    mov dword [imgdest+4],ys
else
    mov eax,ys
    mov dword [imgdest+4],eax
end if

    mov eax,dword [imgsrc] ;getx size
;    lea ecx,[eax+2*eax]
    mov ecx,eax
    shl ecx,1
    add ecx,eax

    mov ebx,y
    mul ebx
    add eax,x
    mov edx,ecx
    lea eax,[eax+2*eax]  ;eax=offset on imsrc
;    mov ebp,eax
;    shl eax,1
;    add eax,ebp

    mov ecx,xs
    mov ebx,ys

    mov edi,8+imgdest
    lea esi,[eax+8+imgsrc]
;    mov esi,eax
;    add esi,8
;    add esi,imgsrc

    cld
cyc:
    movsw
    movsb
    dec ecx
    jne cyc
    add esi,edx
    mov ecx,xs
    sub esi,ecx
    sub esi,ecx
    sub esi,ecx
    dec ebx
    jne cyc
}

macro copyimg img2_off,img1_off
{
    mov  eax,dword [img1_off]
    mov  ebx,dword [img1_off+4]
    mul  ebx
    lea  ecx,[eax+2*eax]
    lea  esi,[img1_off+8]
    lea  edi,[img2_off+8]
    cld
    rep  movsb
}

macro fullimg img_off,xs,ys,color
{
local cop
    mov eax,xs
    mov ebx,ys
    mov  dword [img_off],eax
    mov  dword [img_off+4],ebx
    mul  ebx
    lea  ebp,[eax+2*eax]
    mov  esi,color
if color eqtype 0
    mov  ecx,color/65536
else
    mov  ecx,esi
    shr  ecx,16
end if
    xor  edi,edi
cop:
    mov  word [img_off+8+edi],si
    add  edi,2
    mov  byte [img_off+8+edi],cl
    inc  edi
    cmp  edi,ebp
    jne  cop
}



  ; number of frame in ecx
  ; callculatin offset of raw data

macro getframeoff num_of_frame,offset_of_animation,offset_of_frame
{
local loo,setpic
  mov ebp,num_of_frame ;ecx
  mov esi,offset_of_animation;Image
loo:
  cmp ebp,0
  je  setpic
  movzx eax,word [esi+4]
  movzx ebx,word [esi+6]
  mul ebx ;dword [esi+4]
  mov ebx,3
  mul ebx
  add eax,8
  add esi,eax
  dec ebp
  jmp loo
setpic:
  mov dword offset_of_frame,esi
}


; BMPTOIMG -Convert BMP format TO IMG format
; (SYNTAX)  BMPTOIMG BMP_source_offset,IMG_dest_ofset
; (SAMPLE)  View BMPLS.ASM sample.
; ( NOTE )  This is macros is not brake bmp structure! Tested in 32,8,4 bits


bmptoimg_data_area_count=0
macro bmptoimg bmp_load_area,img_dest_area
{
local fileinfo,string,end_bmp,nodix
local converttable,noaddelem,nextbit,convert1bpp,convert4bpp,convert2
local nextelem,convertno32,nomorestring,convert1,nextstring,yespicsize
;local qwe,bmpfn

;  convert:
    movzx eax,word [bmp_load_area+28]
    mul  dword [bmp_load_area+18]
    add  eax,31
    shr  eax,5
    mov  dword [bmptoimg_data_area_dwps],eax  ;dwps-doublewords per string
    shl  eax,2
    mov  dword [bmptoimg_data_area_bps],eax   ;bps-bytes per string

    cmp dword [bmp_load_area+34],0
    jne  yespicsize  ;if picture size is defined
    mul dword [bmp_load_area+22]
    mov dword [bmp_load_area+34],eax

  yespicsize:
    mov ebp,img_dest_area+8

    mov  eax,bmp_load_area
    mov  ebx,eax
    add  ebx, [bmp_load_area+2];file size
    inc  ebx
    mov  dword [bmptoimg_soi],ebx   ;soi-start of image area for drawing

    add  eax, [bmp_load_area+10]
    mov  dword [bmptoimg_data_area_sop],eax   ;sop-start of picture in file
    add  eax, [bmp_load_area+34]
    mov  dword [bmptoimg_data_area_eop],eax   ;eop-end of picture in file
    mov  eax, [bmp_load_area+18]
    lea  eax,[eax+2*eax]   ;3x pixels in eax

    mov  edi,dword [bmptoimg_soi]   ;initializing
    mov  esi,dword [bmptoimg_data_area_eop]
    sub  esi,dword [bmptoimg_data_area_bps]


  nextstring:
    push edi
    push ebp
    cmp  word [bmp_load_area+28],24
    jne  convertno32

    mov edi,ebp
    mov  ecx,[bmptoimg_data_area_dwps]
    cld
    rep movsd

  convert1:
    pop  ebp
    pop  edi
    sub  esi,dword [bmptoimg_data_area_bps]
    sub  esi,dword [bmptoimg_data_area_bps]
    cmp  esi,dword [bmptoimg_data_area_sop]
    jb   end_bmp
    add  edi,eax
    add  ebp,eax
    jmp  nextstring

  convertno32:
    mov  ebx,bmp_load_area
    add  ebx, [bmp_load_area+14]
    add  ebx,14          ;start of color table
    push esi
    add  esi,dword [bmptoimg_data_area_bps]
    mov  dword [bmptoimg_data_area_eos],esi
    pop  esi
  nextelem:
    push eax
    movzx eax,byte [esi]
    cmp  word [bmp_load_area+28],4
    je   convert4bpp
    cmp  word [bmp_load_area+28],1
    je   convert1bpp
    call converttable

  convert2:
    pop  eax
    inc  esi
    cmp  esi,dword [bmptoimg_data_area_eos]
    jae  convert1
    add  edi,3

    add  ebp,3

    jmp  nextelem

  convert4bpp:
    shl  ax,4
    shr  al,4
    push ax
    movzx eax,ah
    call converttable
    add  edi,3

    add ebp,3

    pop  ax
    movzx eax,al
    call converttable
    jmp  convert2

  convert1bpp:
    mov  ecx,eax
    mov  edx,7
  nextbit:
    xor  eax,eax
    bt   ecx,edx
    jnc  noaddelem
    inc  eax
  noaddelem:
    push edx
    call converttable
    pop  edx
    dec  edx
    js   convert2
    add  edi,3

    add  ebp,3

    jmp  nextbit

  converttable:
    shl  eax,2
    add  eax,ebx
    mov  edx, dword [eax]
;    mov  dword [edi],edx
    mov [ebp],edx
    ret

bmptoimg_data_area_count=bmptoimg_data_area_count+1
if bmptoimg_data_area_count = 1
; DATA AREA
bmptoimg_soi                dd 0
bmptoimg_data_area_bps      dd 0
bmptoimg_data_area_dwps     dd 0
bmptoimg_data_area_sop      dd 0
bmptoimg_data_area_eop      dd 0
bmptoimg_data_area_eos      dd 0
end if

end_bmp:
    mov  eax,dword [bmp_load_area+18]
    mov  ebx,dword [bmp_load_area+22]
    mov  dword [img_dest_area],eax
    mov  dword [img_dest_area+4],ebx
}

; For convert RGB to BGR
COLOR_ORDER equ MENUETOS

macro giftoani gifsrc,imgsrc,num_of_frames
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

;    jmp sss
;    if defined gif_hash_offset
;    else
;    hasharea:
;    times 4096 dd 0 ;4096
;    end if
;sss:

    mov  esi,gifsrc           ;Указатель на ГИФ фаил в памяти
    mov  edi,imgsrc           ;Указатель на список картинок

    if defined gif_hash_offset
    mov  eax,gif_hash_offset    ;Рабочая область минимум 4096*4 байт
    else
    mov  eax,hasharea         ;Рабочая область минимум 4096*4 байт
    end if

    call ReadGIF
    push ecx
    pop  dword num_of_frames
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
    jne  er            ; signature
    mov  ecx,[esi+0xa]
    inc  eax
    add  esi,0xd
    mov  edi,esi
    bt   ecx,7
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
    movsd
    movsd

    push edi
    movzx ecx,word[esi]
    inc  esi
    bt   ecx,7
    jc   uselocal
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
    lodsb               ; eax - block_count
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
    je   reinit
    call Gif_output
cycle:
    movzx ebx,ax
    call Gif_get_sym
    cmp  eax,edx
    jae  notintable
    cmp  eax,[CC]
    je   reinit
    cmp  eax,[EOI]
    je   zend
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
    cmp  byte[edi],0x3b    ;read next frame
    jne  nextblock
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
    inc  ecx            ; color map size
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
    jb   noblock
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
    cur_info dd 1        ; image table pointer
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



macro giftoimg gifsrc,imgsrc
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

    mov  esi,gifsrc           ;Указатель на ГИФ фаил в памяти
    mov  edi,imgsrc           ;Указатель на список картинок

    if defined gif_hash_offset
    mov  eax,gif_hash_offset    ;Рабочая область минимум 4096*4 байт
    else
    mov  eax,hasharea         ;Рабочая область минимум 4096*4 байт
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
    jne  er            ; signature
    mov  ecx,[esi+0xa]
    inc  eax
    add  esi,0xd
    mov  edi,esi
    bt   ecx,7
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
    bt   ecx,7
    jc   uselocal
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
    lodsb               ; eax - block_count
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
    je   reinit
    call Gif_output
cycle:
    movzx ebx,ax
    call Gif_get_sym
    cmp  eax,edx
    jae  notintable
    cmp  eax,[CC]
    je   reinit
    cmp  eax,[EOI]
    je   zend
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
    inc  ecx            ; color map size
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
    jb   noblock
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
    cur_info dd 1        ; image table pointer
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

