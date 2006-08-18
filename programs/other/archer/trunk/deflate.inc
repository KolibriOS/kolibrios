; @RCHER main algorythm
; Written in pure assembler by Ivushkin Andrey aka Willow

macro get_a _type,_size,c1,c2,c3,c4,c5
{
get_#_type:
local .no,.no0,.ex
    push edx
    and  [Flags],not 1
if _type eq Len
    cmp  eax,c4
    jne   .no
    mov  eax,c5
    jmp  .ex
   .no:
end if
    sub  eax,c1
    ja   .no0
    add  eax,c2
    jmp  .ex
   .no0:
    add  eax,c3
    push eax
    mov  ecx,eax
    shr  ecx,_size
    xor  eax,eax
    call read_bits
    pop  edx
    and  edx,1 shl _size-1
    shl  edx,cl
    movzx ecx,[tblH#_type+ecx*2]
    add  edx,ecx
    add  eax,edx
   .ex:
    or  [Flags],1
    pop  edx
    ret
}
; *************************

Deflate:
    mov  edi,[outp]
  .init:
    mov  [bits],8
    lodsb
    call setcurb
  .blkbegin:
    and  [lastblk],0
    and  [Flags],not 1
    rbits 0,1
    test eax,eax
    je  .nolast
    mov  [lastblk],1
  .nolast:
    rbits 0,2
    cmp  eax,10b
    je   .DynHuff
    cmp  eax,01b
    je   .static
    test eax,eax
    jnz  .errorID
    Msg  30
    movzx ecx,[bits]
    call read_bits
    movzx ecx,word[esi-1]
    add  esi,3
    rep  movsb
    jmp  .check_last
  .errorID:
    Msg  6
    ret
; Static Huffman
  .static:
if SHOW_METH eq 1
    Msg  20
end if
    mov  edi,[outp]
    or  [Flags],1
  .next:
    rbits 0,7
;    stop
    cmp  eax,0x17
    ja   .no7
    add  eax,256
    cmp  eax,256
    jne  .noend
  .check_last:
    mov  [outp],edi
    cmp  [lastblk],1
    je   .ex
    jmp  .blkbegin
   .noend:
    call get_Len
    mov  ebx,eax
    rbits 0,5
    call get_Dist
    neg  eax
    push esi
    lea  esi,[edi+eax]
    mov  ecx,ebx
    rep  movsb
    pop  esi
    jmp  .next
   .no7:
    rbits eax,1
    cmp  eax,0xc8
    jb   .no9
    rbits eax,1
    sub  eax,0xd0
    jmp  .no81
  .no9:
    cmp  eax,0xc0
    jb   .no81
    add  eax,0x58
    jmp  .noend
  .no81:
    sub  eax,0x30
    stosb
    jmp .next
  .ex:
    ret
; ************* dynamic Huffman ************

.DynHuff:
;    dps  '##'
if SHOW_METH eq 1
    Msg  19
end if
    pusha
    xor  eax,eax
    mov  ecx,(output-bl_count) / 4
    mov  edi,bl_count
    rep  stosd
    popa

; max_len=0
    and  [max_len],0
    rbits 0,5
; hlit-257
    add  eax,257
    mov  [hlit],ax
    rbits 0,5
; hdist-1
    inc  eax
    mov  [hdist],al
    rbits 0,4
; hclen-4
    add  eax,4
    mov  [hclen],al
    mov  ecx,eax
    push edi
    mov  edi,tmp_clit
; read  code lengths for code lengths
  .alphloop:
    push ecx
    rbits 0,3
    stosb
    pop  ecx
    loop .alphloop
; sort code lengths for code lengths
    push esi
    movzx ecx,[hclen]
    xor  eax,eax
    mov  edi,tmp_clit
    mov  esi,tblSort
  .sortloop:
    lodsb
    movzx bx,byte[edi]
    mov  [sorted_clit+eax*2],bx
    inc  edi
    loop .sortloop
    pop  esi edi
.generate:
    mov  ecx,19
    mov  ebx,calph
    mov  edx,seql
    mov  eax,sorted_clit
    call Huffc
    and  [tblCount],0
    or  [Flags],1
    mov  edi,Lit_c
    mov  ebp,sorted_clit
   .again:
    cmp  edi,output+OUTBUF
    jb   ._ok
    Msg  16
    jmp  .ex
  ._ok:
    mov  edx,seql
    mov  ebx,calph
    call get_code
    call ExpLen
    cmp  [hlit],ax
    ja   .again
if SHOW_CHARS eq 1
    mov  edi,Lit_c
    call Show_codes
end if
    mov  edi,Dist_c
    and  [tblCount],0
   .again2:
    mov  ebx,calph

    call get_code
    call ExpLen
    cmp  [hdist],al
    ja   .again2
    movzx ecx,[hlit]
    mov  ebx,Literal
    mov  edx,seql
    mov  eax,Lit_c
    call Huffc
    movzx ecx,[hdist]
    mov  ebx,Distance
    mov  edx,seqd
    mov  eax,Dist_c
    call Huffc

    push [hlit]
    pop  [tblLen]
    mov  ebp,Lit_c
    mov  edx,seql
    mov  ebx,Literal
    mov  edi,[outp]
    and  [tblCount],0
  .again3:               ; <------------
    call get_code
    cmp  eax,256
    je   .check_last
    ja   .dist
    stosb
    jmp  .again3
  .dist:
    call get_Len
    push eax ebx edx ebp
    mov  ecx,32
    mov  ebp,Dist_c
    mov  edx,seqd
    mov  ebx,Distance
    mov  [tblLen],32
    call get_code
    call get_Dist
    push [hlit]
    pop  [tblLen]
    neg  eax
    pop  ebp edx ebx ecx
    push esi
    lea  esi,[edi+eax]
    rep  movsb
    pop  esi
    jmp  .again3

; ******************************************
Huffc:
; EBX - dest array, ECX - length, EDX - br_seq dest, EAX - source array
    push esi edi eax ecx
    mov  edi,bl_count
    xor  eax,eax
    mov  ecx,BITS
    rep  stosw
    pop  ecx
    mov  esi,[esp]
    mov  [tblLen],cx
    mov  [max_len],ax
; Count the number of codes for each code length
  .cnt_loop:
    lodsw
    cmp  [max_len],ax
    jae  .skip
    mov  [max_len],ax
  .skip:
    inc  byte[bl_count+eax]
    loop .cnt_loop
    movzx ecx,[max_len]
    xor  eax,eax
    and  [bl_count],al
    xor  esi,esi   ; edx - bits
    mov  edi,next_code+2
    push ebx
; Find the numerical value of the smallest code for each code length
  .nc_loop:
    movzx bx,byte[bl_count+esi]
    add  ax,bx
    shl  ax,1
    stosw
    inc  esi
    loop .nc_loop
    pop  ebx
; clear table
    movzx ecx,[tblLen]
    xor  eax,eax
    dec  eax
    mov  edi,ebx
    rep  stosw
    inc  eax
    movzx ecx,[tblLen]
    mov  esi,[esp]
    mov  edi,ebx
; Assign numerical values to all codes
  .loop3:
    lodsw
    test eax,eax
    jz   .lp
    push [next_code+eax*2]
    pop  word[edi]
    inc  [next_code+eax*2]
  .lp:
    add  edi,2
    loop .loop3
; Clear all codes
    xor  eax,eax
    mov  edi,edx
    movzx ecx,[max_len]
    mov  [edi-1],al
; Prepare read bit sequences
  .rebiloop:
    inc  eax
    cmp  [bl_count+eax],0
    jz   .sk
    stosb
    inc  byte[edx-1]
  .sk:
    loop .rebiloop
    movzx ecx,byte[edx-1]
    dec  ecx
    jecxz .noreb2
  .reb2loop:
    mov  al,[edx+ecx-1]
    sub  [edx+ecx],al
    loop .reb2loop
  .noreb2:
    pop  eax edi esi
    ret

; ******************************************

; get Codes of variable sizes
get_code:
; EDX - br_seq, EBX - source table, EBP - codelength table
    push edx edi
    xor  eax,eax
    movzx ecx,byte[edx-1]
    mov  [codel],ax
   .rb3:
    push ecx
    movzx ecx,byte[edx]
    add  [codel],cx
    call read_bits
    movzx ecx,[tblLen]
    inc  ecx
    mov  edi,ebx
   .scas:
    repne scasw
    jecxz .notfound
    push edi ecx
    sub  edi,ebx
    sub  edi,2
    mov  cx,[codel]
    cmp  cx,[ds:ebp+edi]
    jne  .notfound2
    mov  eax,edi
    shr  eax,1
    add  esp,12
   .pp:
    pop  edi edx
    ret
   .notfound2:
    pop  ecx
    pop  edi
    jmp  .scas
   .notfound:
    pop  ecx
    inc  edx
    loop .rb3
    Msg  7
    jmp  .pp

codel dw ?
; ******************************************
ExpLen:
    cmp  eax,16
    jae  .noliteral
    inc  [tblCount]
    stosw
    jmp  .nomatch
  .noliteral:
    and  [Flags],not 1
    mov  ebx,3
    cmp  eax,17
    jae  .code1718
    mov  ecx,2
    xor  eax,eax
    call read_bits
    lea  ecx,[eax+ebx]
    mov  ax,[edi-2]
  .cc:
    add  [tblCount],cx
    rep  stosw
    or  [Flags],1
    jmp  .nomatch
  .code1718:
    jne  .code18
    mov  ecx,3
  .cc2:
    xor  eax,eax
    call read_bits
    lea  ecx,[eax+ebx]
    xor  eax,eax
    jmp  .cc
  .code18:
    mov  ebx,11
    mov  ecx,7
    jmp  .cc2
  .nomatch:
    mov  ax,[tblCount]
    ret
get_a Len,2,256+8,10,3,285,258
get_a Dist,1,3,4,1


; ******************************************
read_bits: ; eax-dest; ecx-count
    push edx ecx
  .shift:
  if RBLOCK eq 4
    ror  [cur_byte],1
  else
    ror  byte[cur_byte],1
  end if
    pushf
    test [Flags],1
    je   .noh1
    popf
    rcl  eax,1
    jmp  .dec
  .noh1:
    popf
    rcr  eax,1
  .dec:
    dec  [bits]
    jnz  .loop1
  .push:
    push eax
    mov  eax,[esi]
    call setcurb
    pop  eax
  if RBLOCK eq 1
    inc  esi
    inc  [IDATcount]
  else
    inc  esi
    inc  [IDATcount]
  end if
    cmp  esi,area+INBUF-BSIZE
    jbe   .ok
    pusha
if SHOW_RBLOCK eq 1
    Msg  9
end if
    mov  eax,0
    mov  ebx,1
    call FileSeek
    mov  [esp+4],esi
    popa
  .ok:
    test [Flags],PNG_MODE
    jz   .idatok
    mov  edx,[IDATcount]
    cmp  edx,[IDATsize]
    jbe  .idatok
    pusha
    lodsd
    call PngParse.nxt_sec
    mov  [IDATcount],1
    mov  [esp+4],esi
    mov  [esp+20],edx
    popa
    cmp  edx,21
    jne  .idatok
    mov  eax,256
    pop  ecx
    jmp  .exx
  .idatok:

    mov  [bits],8
  .loop1:
    loop .shift2
    jmp  .popc
  .shift2:
    jmp  .shift
  .popc:
    pop  ecx
    test [Flags],1
    jne  .exx
  .noh2:
    rol  eax,cl
  .exx:
    pop  edx
    ret

if SHOW_CHARS eq 1
Show_codes:
    pusha
    movzx ecx,[tblLen]
    mov ecx,256
    xor  eax,eax
  .lp2:
    mov  [braces+1],al
    push eax ecx
    invoke StrFormat,eax,strbuf,20
    invoke WriteConsole,[cons],strbuf,16,param1,NULL
    invoke WriteConsole,[cons],braces,6,param1,NULL
    mov  eax,[esp+4]
    movzx  eax,word[edi+eax*2]
    test eax,eax
    jz     .skip
    invoke WriteConsole,[cons],exist,6,param1,NULL
  .skip:
    invoke WriteConsole,[cons],braces+6,2,param1,NULL
    pop  ecx eax
    inc  eax
    loop .lp
    jmp  .ex
   .lp:
    jmp  .lp2
   .ex:
    popa
    ret

    cons dd ?
    param1 dd ?
    braces db '( ) = ',0xa, 0xd
    strbuf rb 20
    exist db 'exists'
end if

makeCRC:
    pusha
    Msg  8
    mov  edi,CRC32table
    add  edi,255*4
    std
    mov  ecx,255
    mov  ebx,0xedb88320
  .m1:
    mov  eax,ecx
    push ecx
    mov  ecx,8
  .m2:
    shr  eax,1
    jnc  .m3
    xor  eax,ebx
  .m3:
    loop .m2
    pop  ecx
    stosd
    loop .m1
    popa
    cld
    ret

UCRC:
; in:  esi - data to calculate CRC
;      ecx - its length
;      [CRC32] - previous CRC32
; out: [CRC32]- partial CRC32 (no pre- & post-conditioning!)
    pusha
    cmp  dword[CRC32table+4],0x77073096
    je   .tbl_rdy
    call makeCRC
  .tbl_rdy:
    mov  eax,[CRC32]
    not  eax
  .m1:
    movzx ebx,al
    shr  eax,8
    xor  bl,[esi]
    xor  eax,[CRC32table+ebx*4]
    inc  esi
    loop .m1
    not  eax
    mov  [CRC32],eax
    popa
    ret

UAdler:
; in:  esi - data to calculate CRC
;      ecx - its length
;      [Adler32] - previous Adler32
; out: [Adler32]- partial Adler32
    pusha
    mov  ebp,65521
    movzx ebx,word[Adler32] ; s1-ebx
    movzx edi,word[Adler32+2] ; s2-edi
  .m1:
    movzx eax,byte[esi]
    add  eax,ebx
    xor  edx,edx
    div  ebp
    mov  ebx,edx
    lea  eax,[edi+ebx]
    xor  edx,edx
    div  ebp
    mov  edi,edx
    inc  esi
    loop .m1
    shl  edi,16
    add  edi,ebx
    mov  [Adler32],edi
    popa
    ret

tblSort db 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15
tblHLen dw 7,11,19,35,67,131
tblHDist dw 3,5,9,17,33,65,129,257,513,1025,2049,4097,8193,16385
