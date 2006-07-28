; BGIFONT.INC v1.0 beta
;
; Written in pure assembler by Ivushkin Andrey aka Willow
;
; Created: December 16, 2004
;
; Last changed: February 2, 2005
;
; Compile with FASM

; BGI constants
BGI_NODRAW   equ 0x10000
BGI_ITALIC   equ 0x20000
BGI_BOLD     equ 0x40000
BGI_HALEFT   equ 0x0
BGI_HARIGHT  equ 0x1000
BGI_HACENTER equ 0x2000
BGI_VABOTTOM equ 0x0
BGI_VATOP    equ 0x4000
BGI_VACENTER equ 0x8000

BGI_FREE     equ 0x80000000
BGI_HAMASK   equ 0x3000
BGI_VAMASK   equ 0xc000

; Freetext structure
struc BGIfree FontName,XY,Angle,ScaleX,ScaleY,StrPtr,StrLen,Color,Align
{
    dd FontName ;0
    dd XY	    ;4
    dd Angle	;8
    dd ScaleX	;12
    dd ScaleY	;16
    dd StrPtr	;20
    dd StrLen	;24
    dd Color	;28
    dd Align	;32
}

; font options structure
struc BGIrec FontName,CharsCount,FirstChar,UpperMargin,LowerMargin,\
    Widths,FirstData,EOF,font_data
{
 .FontName     dd ?  ; 0
 .CharsCount   db ?  ; 4
 .FirstChar    db ?  ; 5
 .UpperMargin  db ?  ; 6
 .LowerMargin  db ?  ; 7
 .Widths       dd ?  ; 8
 .FirstData    dd ?  ; 12
 .EOF	       dd ?    ; 16
 .font_data    dd ?  ; 20 follows (Offsets)
}

macro BGIfont_GetID
{
    call _BGIfont_GetID
}

macro BGIfont_Prepare
{
    call _BGIfont_Prepare
}

macro BGIfont_Freetext
{
    call _BGIfont_Freetext
}

macro BGIfont_Outtext
{
    call _BGIfont_Outtext
}

macro _FI name,_size
{
    db name
if BGI_LEVEL eq KERNEL
    dw _size
end if
}

BGIfont_names:
_FI 'LCOM',11485   ;7
_FI 'EURO',8117    ;5
_FI 'GOTH',13816   ;6
_FI 'LITT',3596    ;8
_FI 'TRIP',11932   ;14
_FI 'SCRI',8490    ;11
_FI 'SMAL',4162    ;13
_FI 'TSCR',12134   ;15
_FI 'SANS',8453    ;10
_FI 'SIMP',9522    ;12
BGIfont_names_end:

macro BGIfont_Init
{
; in:  ecx - number of fonts to load;
;      esi-> _FI structure
;      edi-> where to load
    push edi
if  BGI_LEVEL eq KERNEL
    mov  edi,0x40000
end if
  .nfont:
    mov  edx,[esi]
if  BGI_LEVEL eq KERNEL
    movzx ebx,word[esi+4]
    mov  [BGIfont_Prepare.okflag],'N'
end if
    call _BGIfont_Prepare
if ~ BGI_LEVEL eq KERNEL
    add  esi,4
else
    push esi
    test eax,eax
    jz	 .fail
    mov  [BGIfont_Prepare.okflag],'*'
  .fail:
    mov  esi,BGIfont_Prepare.font
    call boot_log
    pop  esi
    add  esi,6
end if
    loop .nfont
    dph2 _BGI_BOLD,300,550
;    movzx edi,byte[0x40000]
    pop  edi
}

BGIfont_get2head:
    shr  ecx,28 ; font #
    sub  ecx,4
    jb	 .exit2	; invalid #
    mov  edi,[BGIfont_Ptr]
    inc  edi
    cmp  cl,[edi-1]
    jae  .exit2 ; # too large
    jecxz .ex
  .fnext:
    mov  edi,[edi+16]
    loop .fnext
    jmp  .ex
  .exit2:
    xor  edi,edi
  .ex:
    ret

BGIfont_GetName:
; in: ecx-fontID;
; out: edx-font name.
    call BGIfont_get2head
    xor  edx,edx
    test edi,edi
    jz   .ex
    mov  edx,[edi]
  .ex:
    ret

macro dps2 _str
{
if ~ BGI_LEVEL eq KERNEL
  if LOAD_MSG eq 1
    dps _str
  end if
else
    pusha
    mov  esi,BGIfont_Prepare.okflag
    mov  byte[esi], _str
    call boot_log
    popa
end if
}

macro dph2 num,x,y
{
if  BGI_LEVEL eq KERNEL
    pusha
    mov  eax,0x00080100
    mov  ebx,num
    mov  ecx,x shl 16+y
    mov  edx,0xFF0000
    call display_number
    popa
end if
}

_BGIfont_GetID:
; in:  edx-font name;
; out: eax-fontID, edi->BGIrec
    push ecx edi
    mov  edi,[BGIfont_Ptr]
    movzx ecx,byte[edi] ; ecx-font count
    mov  eax,ecx
    inc  edi ; edi->FontName
    jecxz .ex
  .fnext:
    cmp  edx,[edi]
    jne  .floop
    sub  eax,ecx
    add  eax,4
    shl  eax,28
    jmp  .ex
  .floop:
    mov  edi,[edi+16]
    loop .fnext
  .num0:
    xor  eax,eax
  .ex:
    pop  edi ecx
    ret

_BGIfont_Prepare:
; in:  edx-font name, edi->pointer to load fonts (fonts_count)
; out: eax-ID of new font loaded; eax=0 error
    cmp  [BGIfont_Ptr],0
    jne  .already
    mov  [BGIfont_Ptr],edi
  .already:
    pusha
    mov  edi,[BGIfont_Ptr]
    movzx ecx,byte[edi] ; ecx-font count
    mov  eax,ecx
    inc  edi ; edi->FontName
    jecxz .fload
  .fnext:
    cmp  edx,[edi]
    jne  .loop
    sub  eax,ecx
    inc  eax
    jmp  .cr_id
  .loop:
    mov  edi,[edi+16]
    loop .fnext
  .fload:
    mov  dword[.font],edx ; filename
    mov  esi,edi     ; esi->FontName
    mov  [.dest],edi ; ptr to load font
if ~ BGI_LEVEL eq KERNEL
    mov  [.fsize],1
    mov  eax,58
    mov  ebx,.fontinfo
    int  0x40
    test eax,eax
    jnz   .fail
    dps2  '1'
    shr  ebx,9
    inc  ebx
    mov  [.fsize],ebx
    mov  ebx,.fontinfo
    mov  eax,58
    int  0x40	     ; ebx - file size
else
    push edi esi edx
    mov  eax,.font
    xor  ebx,ebx
    mov  esi,12
    mov  ecx,ebx
    mov  edx,edi
    call fileread
    pop  edx esi edi
    mov  ebp,edi
    add  ebp,ebx
    cmp  ebp,0x50000
    ja	 .fail
end if
    cmp  dword[edi],0x08084b50 ; 'PK',8,8
    jne  .fail
    dps2 '2'
    inc  edi
    mov  eax,26 ; #EOF
    mov  ecx,253
    cld
    repne scasb  ; skip Copyright
    test ecx,ecx
    jz	 .fail
    dps2  '3'
    cmp  edx,[edi+2] ; FontName
    jne  .fail
    dps2  '4'
    movzx ecx,word[edi] ; HeaderSize
    sub  ebx,ecx  ; Filesize-Headersize
    movzx eax,word[edi+6] ; FontSize
    cmp  eax,ebx
    jb	 .fail	  ; file truncated
    add  ecx,[.dest]
    dps2  '5'
    cmp  byte[ecx],'+'	; ParPrefix
    jne  .fail
; font is valid, let's fill parameter table
    dps2  '>'
    mov  [esi],edx ; FontName
    mov  edx,eax
    add  eax,ecx
    mov  [esi+16],eax ; Font EOF
    movzx eax,word[ecx+5]
    add  eax,ecx
    mov  [esi+12],eax
    lea  edi,[esi+4]  ; edi->CharsCount
    lea  esi,[ecx+1] ; esi->ParPrefix+1
    xor  eax,eax
    lodsw
    stosb  ; CharsCount
    inc  esi
    movsb  ; FirstChar
    add  esi,3
    lodsw
    stosb  ; UpperMargin
    movsb  ; LowerMargin
    add  esi,5 ; esi->offsets
    mov  eax,[esi]
    push edi ; edi->Widths
; prepare moving data
    add  edi,12 ; edi->offsets
    lea  ecx,[edx-16]
    rep  movsb
    pop  edi ; edi->Widths
    mov  [edi+8],esi ; EOF
;    mov  eax,[edi]
    movzx ecx,byte[edi-4] ; CharsCount
    lea  eax,[edi+12+ecx*2] ; eax->widths
    stosd  ; edi->FirstData
    add  eax,ecx
    stosd  ; edi->EOF
    mov  eax,[esp] ; eax->fonts_count
    inc  byte[eax] ; increase font counter
    movzx eax,byte[eax]
  .cr_id:
    add  eax,0x3   ; create unique ID
    shl  eax,28    ; to easy use in color(ecx)
    jmp  .exit
  .fail:
    xor  eax,eax
  .exit:
    mov  [esp+28],eax
    popa
    ret

if ~ BGI_LEVEL eq KERNEL
.fontinfo:
	dd 0
	dd 0
.fsize	dd 0
.dest	dd 0
	dd 0x10000
	db BGIFONT_PATH
.font	db 'FONT.CHR',0
else
  .dest   dd 0
  .font   db 'FONT    CHR'
  .okflag db ' ',0
end if

BGIfont_Coo:
; y->word[txt.y1], x->word[txt.x1]
    fild [txt.y1] ;y
    fmul st0,st0; y*y
    fild [txt.x1] ;x
    fmul st0,st0; x*x
    faddp  ; x*x+y*y
    fsqrt  ; sqrt, angle
    fild [txt.y1];y
    fabs
    fild [txt.x1] ; x
    fabs
    fpatan ; arctg(y/x)
  .skip:
    cmp  [txt.x1],0
    jge  .xplus
    fchs
    fadd st0,st3
  .xplus:
    cmp  [txt.y1],0
    jge  .yplus
    fchs
  .yplus:
    fadd st0,st2
    fsincos
    fmul st0,st2
    fiadd [txt.x0]
    fistp [txt.x1] ; x=r*cos a
    fmulp ; y=r*sin a,angle
    fiadd [txt.y0]
    fistp [txt.y1]
    ret

_BGIfont_Freetext:
; in: ebx-BGIfree structure
; out: eax-new drawing coords
    mov  edx,[ebx]
    call _BGIfont_GetID
    test eax,eax
    jnz  .fexists
    ret
  .fexists:
    pusha
    fninit
    fldpi
    fld  [pi180]
    fimul dword[ebx+8]
    fst  [BGIangle]
    mov  esi,[ebx+28]
    and  esi,0xffffff
    add  esi,eax
    mov  eax,[ebx+32]
    and  [deform],0
    test eax,BGI_ITALIC
    jz	 .norm
    mov  [deform],0.4
  .norm:
    mov  ebp,eax
    or	  ebp,BGI_FREE
    mov  eax,[ebx+12]
    mov  [Xscale],eax
    mov  eax,[ebx+16]
    mov  [Yscale],eax
    mov  ecx,[ebx+20]
    mov  edx,ebp
    and  edx,BGI_FREE+BGI_VAMASK+BGI_HAMASK
    add  edx,[ebx+24]
    mov  eax,[ebx+4]
    mov  ebx,esi
    add  ebx,0x6000000
    mov  [esp+4],edx
    mov  [esp+20],ecx
    jmp  txt

    pi180 dd 0.017453

_BGIfont_Outtext:
; in: ebx-[x][y], ecx-color, edx-string, esi-length
    pusha
    mov  ebp,esi
if ~ BGI_LEVEL eq KERNEL
    mov  eax,ebx
    mov  ebx,ecx
    mov  ecx,edx
    mov  edx,esi
end if
; in: eax-[x][y], ebx-color, ecx-string, edx-length
txt:
if  ~ BGI_LEVEL eq KERNEL
  if  BGI_WINDOW_CLIP eq 1
    pusha
    mov  eax,9
    mov  ebx,BGI_PRC_INFO
    mov  ecx,-1
    int  0x40
    popa
  end if
end if
    mov  [.y0],ax
    shr  eax,16
    mov  [.x0],ax
    mov  ecx,ebx ; color
    and  ebx,0xfffffff
    mov  [.color],ebx
    call BGIfont_get2head
    test edi,edi
    jz   .exit
    mov  ecx,[esp+4]; str length
    mov  esi,[esp+20]; str ptr
    movzx eax,byte[edi+5]
    push ecx
    and  ecx,0xff
    jnz  .lenok
    add  esp,4
    jmp  .ex2
  .lenok:
    pusha
    push dword[txt.y0]
    and  dword[txt.y0],0
    xor  edx,edx
    mov  ebx,[edi+8]
   .next:
    call txt.BGIfont_GetChar
    movzx eax,byte[ebx+eax]
    add  edx,eax
    loop .next
    mov  ecx,edx ; ecx - x size
    movzx dx,byte[edi+6]
    mov  ebx,[esp+36]
    and  ebx,BGI_HAMASK
    cmp  ebx,BGI_HARIGHT
    je   .nova
    ja   .subv
    xor  ecx,ecx
    jmp  .nova
  .subv:
    shr  cx,1
  .nova:
    mov  ebx,[esp+36]
    and  ebx,BGI_VAMASK
    cmp  ebx,BGI_VATOP
    je   .def
    ja   .subh
    xor  edx,edx
    jmp  .def
  .subh:
    shr  dx,1
  .def:
    call txt.BGIfont_Deform
    pop  dword[txt.y0]
    popa
    pop  ebx
    mov  ax,[txt.y1]
    sub  [txt.y0],ax
    mov  ax,[txt.x1]
    sub  [txt.x0],ax
    xor  eax,eax
    cld
.mloop:
    push [.y0]
    pop  [.y]
    push [.x0]
    pop  [.x]
    call .BGIfont_GetChar
    push esi
    lea  esi,[edi+20] ; offset
    movzx edx,word[esi+eax*2] ; ofs1
    add  edx,[edi+12]
    inc  eax
    cmp  al,[edi+4]
    je	 .eof
    movzx eax,word[esi+eax*2]; ofs2
    add   eax,[edi+12]
    jmp   .prc_vec
  .eof:
    mov  eax,[edi+16] ; ofs2=eof
  .prc_vec:  ; edx-vec cmd ifs, eax-cmd limit
    mov  [.vec_end],eax
    push ecx
  .vec_loop:
    mov  ax,word[edx]
    push edx
    mov  ecx,eax
    and  eax,0x8080 ; op
    and  ecx,0x7f ; xx
    mov  edx,[edx+1]
    and  edx,0x7f ; yy
    cmp  edx,63
    jbe  .positive
    sub  edx,128  ; yy-=128
  .positive:
    cmp  ecx,63
    jbe  .positive2
    sub  ecx,128  ; xx-=128
  .positive2:
    call .BGIfont_Deform
    cmp  eax,0x8080
    jne  .noline
    test ebp,BGI_NODRAW
    jnz  .noline
; draw vector
if ~ BGI_LEVEL eq KERNEL
    push eax
    mov  ebx,dword[.x1]
    mov  ecx,dword[.y1]
  if BGI_WINDOW_CLIP eq 1
    movzx eax,[.x]
    cmp  eax,dword[BGI_PRC_INFO+42]
    ja   .nobold
    movzx eax,[.y]
    cmp  eax,dword[BGI_PRC_INFO+46]
    ja  .nobold
    xor  eax,eax
    cmp  ax,bx
    jg   .nobold
    cmp  ax,cx
    jg   .nobold
  end if
    mov  edx,[.color]
    mov  eax,38
    int  0x40
    test ebp,BGI_BOLD
    jz	 .nobold
    test ebp,BGI_FREE
    jnz  .free5
  .free5:
    add  ebx,1 shl 16+1
    int  0x40
  .nobold:
    pop  eax
else
    pusha
    mov  eax,dword[.x1]
    mov  ebx,dword[.y1]
    mov  ecx,[.color]
;    call syscall_drawline
    test dword[esp+8],BGI_BOLD
    jz	 .nobold
    add  eax,1 shl 16+1
;    call syscall_drawline
  .nobold:
    popa
end if
  .noline:
    pop  edx
    test eax,eax
    je	 .eovecs  ; op=0
    push [.y1]
    pop  [.y]
    push [.x1]
    pop  [.x]
    add  edx,2
    cmp  edx,[.vec_end]
    jb	 .vec_loop
  .eovecs:
    pop  ecx esi
    push [.y]
    pop  [.y0]
    push [.x]
    pop  [.x0]
    loop .mloop1
    jmp  .exit
  .mloop1:
    jmp  .mloop
  .exit:
    mov  eax,dword[.y0]
    mov  [esp+28],eax
  .ex2:
    popa
    ret

.BGIfont_Deform:
    test ebp,BGI_FREE
    jnz  .free0
    movzx ebx,byte[.color+3] ;ebx=scale
    imul ecx,ebx
    add  ecx,2
    shr  ecx,2
    imul edx,ebx
    add  edx,2
    shr  edx,2
    neg  edx
    mov  [.x1],cx
    mov  [.y1],dx
    jmp  .add
  .free0:
    mov  [.x1],cx
    mov  [.y1],dx
    fild [.y1]
    fld  st0
    fmul [Yscale]
    fchs
    fistp [.y1]
    fmul [deform]
    fiadd [.x1]
    fmul [Xscale]
    fistp [.x1]
    cmp  [BGIangle],0
    je	 .add
    call BGIfont_Coo
    jmp  .eax
  .add:
    mov  cx,[.x0]
    add  [.x1],cx
    mov  cx,[.y0]
    add  [.y1],cx
  .eax:
    ret

.BGIfont_GetChar:
; in:  esi -> string; edi -> BGIrec
; out: esi -> next char; al - char obtained
    lodsb  ; al - char from str
    sub  al,[edi+5]
    jb	 .out
    cmp  al,[edi+4]
    jb	 .in
  .out:
    xor  al,al ; al - 1st symbol available
  .in:
    ret

.y0	 dw ?
.x0	 dw ?

.x1	 dw ?
.x	 dw ?
.y1	 dw ?
.y	 dw ?

.color	 dd ?
.vec_end dd ?
BGIfont_Ptr  dd 0
width  dd ?
deform dd ?
BGIangle dd ?
Xscale  dd ?
Yscale  dd ?
