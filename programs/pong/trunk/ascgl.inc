lang equ ru

;
;   Assembler
;     SMALL
;       CODE
;         Graphics
;           Libary
;
;   Ver 0.10 By Pavlushin Evgeni (RUSSIA)
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

;DrawBox
macro drawfbox x,y,xs,ys,color
{
    mov  ecx,y
    mov  ebx,x
    shl  ebx,16
    add  ebx,xs
    shl  ecx,16
    add  ecx,ys
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
if (_1dbounce_count = 1)
_1dbounce:
	cmp ebx,eax
	jnae anot
	add eax,esi
	cmp eax,ebx
	jna nbc
	jmp bc
anot: 
    add ebx,edi
	cmp ebx,eax
	jna nbc
bc:	
    mov ecx,1
	ret
nbc:
	mov ecx,0
	ret
end if
bounce:	
	mov otv,1
exit:	
}

; SETBMP -  Set bmp to window
; (SYNTAX)  SETBMP dd xstart ,dd ystart ,BMP_offset,dd soi
; (SAMPLE)  SETBMP dword [xt],dword [yt],I_END,dword [tsoi]
;           SETBMP 15,10,I_END,dword [tsoi]
; ( NOTE )  SOI - Start of image

macro setbmp arg1,arg2,arg3,arg4
{
local nodi
    cmp  word [arg3],word 'BM'
    jne  nodi
    mov  eax,7
    mov  ebx,arg4 ;[soi]
    mov  ecx,dword [arg3+18]
    shl  ecx,16
    add  ecx,dword [arg3+22]
if (arg1 eqtype 0) & (arg2 eqtype 0)
    mov  edx,arg1*65536+arg2
else
    mov  edx,arg1
    shl  edx,16
    add  edx,arg2
end if    
    int  0x40
  nodi:
}

macro setimg arg1,arg2,arg3
{
local nodi
    mov  eax,7
    mov  ebx,arg3
    add  ebx,8
    mov  ecx,dword [arg3]
    shl  ecx,16
    add  ecx,dword [arg3+4]
if (arg1 eqtype 0) & (arg2 eqtype 0)
    mov  edx,arg1*65536+arg2
else   
    mov  edx,arg1
    shl  edx,16
    add  edx,arg2
end if   
    int  0x40
}

;Not optimiz
macro getimg imgsrc,x,y,xs,ys,imgdest
{
local cyc
    mov eax,xs
    mov dword [imgdest],eax
    mov eax,ys
    mov dword [imgdest+4],eax

    mov eax,dword [imgsrc] ;getx size
    mov edi,eax
    mov ebx,y
    mul ebx
    add eax,x
    mov ebx,3
    mul ebx     ;eax=offset on imsrc

    mov ecx,0
    mov ebx,0
    mov ebp,eax
    mov esi,0

    add esi,8
    add ebp,8

cyc:
    mov al,byte [imgsrc+ebp]
    mov [imgdest+esi],al
    mov al,byte [imgsrc+ebp+1]
    mov [imgdest+esi+1],al
    mov al,byte [imgsrc+ebp+2]
    mov [imgdest+esi+2],al
    add esi,3
    add ebp,3
    inc ecx
    cmp ecx,xs
    jne cyc
    add ebp,edi
    add ebp,edi
    add ebp,edi
    sub ebp,xs
    sub ebp,xs
    sub ebp,xs
    mov ecx,0
    inc ebx
    cmp ebx,ys
    jne cyc

}

;
macro bmptoimg bmp_off,soi,img_off
{
local nodix,conv
    cmp  word [bmp_off],word 'BM'
    jne  nodix
    mov  eax,dword [bmp_off+18]
    mov  ebx,dword [bmp_off+22]
    mov  dword [img_off],eax
    mov  dword [img_off+4],ebx
    mul  ebx
    lea  ecx,[eax+2*eax]
    lea  edi,[img_off+8]
    mov  esi,dword [soi]
    cld
    rep  movsb
nodix:
}

macro copyimg img2_off,img1_off
{
local cop
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




macro convbmp bmp_load_area,bmp_soi
{
local status,bps,dwps,soi,sop,eop,eos,process,fileinfo,string,end_bmp
local converttable,noaddelem,nextbit,convert1bpp,convert4bpp,convert2
local nextelem,convertno32,nomorestring,convert1,nextstring,yespicsize
local noaddword
;local qwe,bmpfn

;  convert:
    movzx eax,word [bmp_load_area+28]
    mul  dword [bmp_load_area+18]
    mov  ebx,32
    div  ebx
    test edx,edx
    je   noaddword
    inc  eax
  noaddword:
    mov  dword [dwps],eax  ;dwps-doublewords per string
    shl  eax,2
    mov  dword [bps],eax   ;bps-bytes per string

    cmp dword [bmp_load_area+34],0
    jne  yespicsize  ;if picture size is defined
    mul dword [bmp_load_area+22]
    mov dword [bmp_load_area+34],eax

  yespicsize:
    mov  eax,bmp_load_area
    mov  ebx,eax
    add  ebx, [bmp_load_area+2];file size
    inc  ebx
    mov  dword [bmp_soi],ebx   ;soi-start of image area for drawing
    add  eax, [bmp_load_area+10]
    mov  dword [sop],eax   ;sop-start of picture in file
    add  eax, [bmp_load_area+34]
    mov  dword [eop],eax   ;eop-end of picture in file
    mov  eax, [bmp_load_area+18]
    lea  eax,[eax+2*eax]   ;3x pixels in eax

    mov  edi,dword [bmp_soi]   ;initializing
    mov  esi,dword [eop]
    sub  esi,dword [bps]


  nextstring:
    push edi
    cmp  word [bmp_load_area+28],24
    jne  convertno32

    mov  ecx,[dwps]
    cld
    rep movsd
  convert1:
    pop  edi
    sub  esi,dword [bps]
    sub  esi,dword [bps]
    cmp  esi,dword [sop]
    jb   end_bmp;nomorestring
    add  edi,eax
    jmp  nextstring

;  nomorestring:
;    jmp end_bmp

  convertno32:
    mov  ebx,bmp_load_area
    add  ebx, [bmp_load_area+14]
    add  ebx,14          ;start of color table
    push esi
    add  esi,dword [bps]
    mov  dword [eos],esi
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
    cmp  esi,dword [eos]
    jae  convert1
    add  edi,3
    jmp  nextelem

  convert4bpp:
    shl  ax,4
    shr  al,4
    push ax
    movzx eax,ah
    call converttable
    add  edi,3
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
    cmp  edx,0xffffffff
    je   convert2
    add  edi,3
    jmp  nextbit

  converttable:
    shl  eax,2
    add  eax,ebx
    mov  edx, dword [eax]
    mov  dword [edi],edx
    ret


; DATA AREA

;status   dd 0  ;bit0=1 if file thread is created
bps      dd 0
dwps     dd 0
;soi      dd 0
sop      dd 0
eop      dd 0
eos      dd 0
;process  dd 0

end_bmp:
;    mov eax,dword [soi]
;    mov dword [bmp_soi],eax

}




; LOADBMP - Load bmp image from file
; (SYNTAX)  LOADBMP 'file_path',temp_area(rb 0x10000),load_area,dd soi
; (SAMPLE)  LOADBMP '/rd/1/menuet.bmp',temp_area,I_END,tsoi
; ( NOTE )  Macros create on Base of
;           Picture browser by lisovin@26.ru & Ivan Poddubny
;           SOI - Start of image

macro loadbmp bmp_file_name,bmp_temp_area,bmp_load_area,bmp_soi
{
local status,bps,dwps,soi,sop,eop,eos,process,fileinfo,string,end_bmp
local converttable,noaddelem,nextbit,convert1bpp,convert4bpp,convert2
local nextelem,convertno32,nomorestring,convert1,nextstring,yespicsize
local noaddword
;local qwe,bmpfn

    mov  dword [fileinfo+8],1 ; how many blocks to read (1)
    mov  eax,58
    mov  ebx,fileinfo
    int  0x40
    mov  eax,[bmp_load_area+2]
    shr  eax,9 ; поделим на 512 и прибавим 1 - получим число блоков
    inc  eax
    mov  dword [fileinfo+8],eax
    mov  eax,58
    mov  ebx,fileinfo
    int  0x40

;     jmp  qwe
;bmpfn db bmp_file_name
;qwe:
;     mov eax,6
;     mov ebx,bmpfn
;     mov ecx,0
;     mov edx,0xFFFFFF
;     mov esi,bmp_load_area
;     mov edi,0
;     int 0x40

;  convert:
    movzx eax,word [bmp_load_area+28]
    mul dword [bmp_load_area+18]
    mov  ebx,32
    div  ebx
    test edx,edx
    je   noaddword
    inc  eax
  noaddword:
    mov  dword [dwps],eax  ;dwps-doublewords per string
    shl  eax,2
    mov  dword [bps],eax   ;bps-bytes per string

    cmp dword [bmp_load_area+34],0
    jne  yespicsize  ;if picture size is defined
    mul dword [bmp_load_area+22]
    mov dword [bmp_load_area+34],eax

  yespicsize:
    mov  eax,bmp_load_area
    push eax
    add  eax, [bmp_load_area+2];file size
    inc  eax
    mov  dword [soi],eax   ;soi-start of image area for drawing
    pop  eax
    add  eax, [bmp_load_area+10]
    mov  dword [sop],eax   ;sop-start of picture in file
    add  eax, [bmp_load_area+34]
    mov  dword [eop],eax   ;eop-end of picture in file
    mov  eax, [bmp_load_area+18]
    mov  ebx,3
    mul  ebx             ;3x pixels in eax

    mov  edi,dword [soi]   ;initializing
    mov  esi,dword [eop]
    sub  esi,dword [bps]


  nextstring:
    push edi
    cmp word [bmp_load_area+28],24
    jne  convertno32

    mov  ecx,[dwps]
    cld
    rep movsd
  convert1:
    pop  edi
    sub  esi,dword [bps]
    sub  esi,dword [bps]
    cmp  esi,dword [sop]
    jb   nomorestring
    add  edi,eax
    jmp  nextstring

  nomorestring:
    jmp end_bmp

  convertno32:
    mov  ebx,bmp_load_area
    add  ebx, [bmp_load_area+14]
    add  ebx,14          ;start of color table
    push esi
    add  esi,dword [bps]
    mov  dword [eos],esi
    pop  esi
  nextelem:
    push eax
    movzx eax,byte [esi]
    cmp word [bmp_load_area+28],4
    je   convert4bpp
    cmp word [bmp_load_area+28],1
    je   convert1bpp
    call converttable
  convert2:
    pop  eax
    inc  esi
    cmp  esi,dword [eos]
    jae  convert1
    add  edi,3
    jmp  nextelem

  convert4bpp:
    shl  ax,4
    shr  al,4
    push ax
    movzx eax,ah
    call converttable
    add  edi,3
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
    cmp  edx,0xffffffff
    je   convert2
    add  edi,3
    jmp  nextbit

  converttable:
    shl  eax,2
    add  eax,ebx
    mov  edx, dword [eax]
    mov  dword [edi],edx
    ret


; DATA AREA

;status   dd 0  ;bit0=1 if file thread is created
bps      dd 0
dwps     dd 0
soi      dd 0
sop      dd 0
eop      dd 0
eos      dd 0
;process  dd 0

; DATA AREA
fileinfo:
     dd 0
     dd 0
     dd 1          ;number of blocks  of 512 bytes
     dd bmp_load_area
     dd bmp_temp_area
string:
  db bmp_file_name,0
  db '                                            '
  db '                                            '
  db '                                            '
  db '                                            ',0


end_bmp:
    mov eax,dword [soi]
    mov dword [bmp_soi],eax

}
