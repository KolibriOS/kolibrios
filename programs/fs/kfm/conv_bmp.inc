  convert_bmp_to_img:
    movzx eax,word [ebp+28]
    mul dword [ebp+18]
    mov  ebx,32
    div  ebx
    test edx,edx
    je   noaddword
    inc  eax
  noaddword:
    mov  [dwps],eax  ;dwps-doublewords per string
    shl  eax,2
    mov  [bps],eax   ;bps-bytes per string

    cmp dword [ebp+34],0
    jne  yespicsize  ;if picture size is defined
    mul dword [ebp+22]
    mov dword [ebp+34],eax

  yespicsize:
    mov  eax,ebp
    push eax
    sub  eax, [ebp+2];file size
    mov  [soi],eax   ;soi-start of image area for drawing
    pop  eax
    add  eax, [ebp+10]
    mov  [sop],eax   ;sop-start of picture in file
    add  eax, [ebp+34]
    mov  [eop],eax   ;eop-end of picture in file
    mov  eax, [ebp+18]
    mov  ebx,3
    mul  ebx             ;3x pixels in eax

    mov  edi,[soi]   ;initializing
    mov  esi,[eop]
    sub  esi,[bps]


  nextstring:
    push edi
    mov  ecx,[dwps]
    cld
    rep movsd
  convert1:
    pop  edi
    sub  esi,[bps]
    sub  esi,[bps]
    cmp  esi,[sop]
    jb   nomorestring
    add  edi,eax
    jmp  nextstring

  nomorestring:
    ret
    
bps      dd 0
dwps     dd 0
soi      dd 0
sop      dd 0
eop      dd 0
