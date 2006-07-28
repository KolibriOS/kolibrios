;workarea:    <- RTF_work
;  listptr dd savelist    0
;  szKeyword rb 31        4
;  szParameter rb 21      35

include 'rtftype.inc'
include 'rtfactn.inc'

read_next_block:
  inc  [cur_block]
read_block:
  mov  esi,I_END
  pusha
  mov  ecx,[cur_block]
  mov  ebx,fileinfo
  mov  eax,ecx
  shl  eax,7
  mov  [ebx+4],eax
;  mov  [ebx+12],esi
  mcall 58
if DEBUG_BLOCK eq 1
;  dps  'B='
;  dpd  ecx
;  dps  <13,10>
end if
  cmp  ecx,[max_block]
  je   .last
  mov  ebx,I_END+RTFSIZE
  jmp  .add
.last:
  mov  ebx,[tail]
;  dpd  ebx
.add:
;  dpd  ebx
  mov  [block_end],ebx
  popa
;  dpd  esi
;  dps  <13,10>
  ret
;
; %%Function: ecRtfParse
;
; Step 1:
; Isolate RTF keywords and send them to ecParseRtfKeyword;
; Push and pop state at the start and end of RTF groups;
; Send text to ecParseChar for further processing.
;
macro CopySave _src,_dest
{
    pusha
    mov  ecx,SIZE_save
    mov  esi,_src
    mov  edi,_dest
    rep  movsb
    popa
}

RtfParse:

 if BENCH eq 1
    mcall 26,9
    mov  [bench],eax
 end if
    mov  [RetroPtr],esi
    CopySave Chp,RetroSave
    push dword[Free+4]
    pop  dword[RetroXY]
    xor  eax,eax
    mov  [cur_block],eax
    mov  [RetroBlock],eax
    push [cGroup]
    pop  [RetroGroup]
    and  [mode],not RTF_BLIND
    mov  [listptr],save_stack
    mov  [fileinfo.size],128
;    test ebp,RTF_HELP
    test [mode],RTF_HELP
    jne  .noread
    call read_block
  .noread:
    mov  [RetroPtr],esi
  .nib2:
    mov  [nibble],2
    and  [b],al
  .getc:
    xor  eax,eax
    lods_block
    cmp  [cGroup],0
    jge  .ok1
    Return ecStackUnderflow
  .ok1:
    cmp  [ris],risBin
    jne  .nobin
    RetError ecParseChar
  .nobin:
    cmp  al,'{'
    jne  .nobr1
    RetError ecPushRtfState
  .nobr1:
    cmp  al,'}'
    jne  .nobr2
    RetError ecPopRtfState
  .nobr2:
    cmp  al,'\'
    jne  .noslash
    RetError ecParseRtfKeyword
  .noslash:
    cmp  al,0xd
    je   .getc
    cmp  al,0xa
    je   .getc
  .nouc:
    cmp  [ris],risNorm
    jne  .nonorm
    call ecParseChar
    cmp  eax,ecOutOfWindow
    je   .__ex
    test eax,eax
    je   .getc
    jmp  .__ex
  .nonorm:
    cmp  [ris],risHex
    je   .noassert
    Return ecAssertion
  .noassert:
    shl  [b],4
    isdigit al, .nodig
    sub  al,'0'
    add  [b],al
    jmp  .nibble
  .nodig:
    islower al, .nolow
    cmp  al,'a'
    jb   .inval
    cmp  al,'f'
    ja   .inval
    sub  al,'a'-10
    jmp  .nib0
  .inval:
  if INVALHEX eq 0
    jmp  .getc
  else
;    sub  esi,I_END+1
;    dpd  esi
;    movzx eax,al
;    dpd  eax
;    movzx eax,[b]
;    dpd  eax
    Return ecInvalidHex
  end if
  .nolow:
    cmp  al,'A'
    jb   .inval
    cmp  al,'F'
    ja   .inval
    sub  al,'A'-10
  .nib0:
    add  [b],al
  .nibble:
    dec  [nibble]
    cmp  [nibble],0
    jnz  .getc
    movzx eax,[b]
    mov  [ris],risNorm
    call ecParseChar
    test eax,eax
    jnz  .__ex
;    mov  [ris],risNorm
    jmp  .nib2
  .eof:
    xor  eax,eax
    cmp  eax,[cGroup]
    je   .__ex
    jg   .unbr
    Return ecStackUnderflow
  .unbr:
    mov  eax,ecUnmatchedBrace
  .__ex:
    ret
nibble db 2
b      db 0
RTF_maxlist dd ?
;
; %%Function: ecParseRtfKeyword
;
; Step 2:
; get a control word (and its associated value) and
; call ecTranslateKeyword to dispatch the control.
;

ecParseRtfKeyword:
; ch-al,bl-fParam, bh-fNeg
    mov  [ris],risNorm
    xor  eax,eax
    xor  ebx,ebx
    push edx
    mov  [szKeyword],al
    mov  [szParameter],al
    lods_block
    isalpha al,.ctrl
    jmp  .alph
  .ctrl:
    push esi
    mov  esi,szKeyword
    inc  byte[esi]
    mov  [esi+1],al
    mov  eax,ebx
    xor  ebx,ebx
    call ecTranslateKeyword
    pop  esi
    jmp  .__ex
  .alph:
    push edi
    mov  edi,szKeyword+1
  .loop1:
    stosb
    inc  [szKeyword]
    lods_block
    isalpha al,.outloop1
    jmp  .loop1
  .outloop1:
    pop  edi
    cmp  al,'-'
    jne  .noneg
    not  bh
    lods_block
  .noneg:
    isdigit al,.nodig
    not  bl
    push edi
    mov  edi,szParameter+1
  .loop2:
    stosb
    inc  [szParameter]
    lods_block
    isdigit al,.outloop2
    jmp  .loop2
  .outloop2:
    pop  edi
    push eax esi
    mov  esi,szParameter
    atoi
    pop  esi
    mov  edx,eax
    pop  eax
    mov  [lParam],edx
    test bh,bh
    jz   .nodig
    neg  edx
  .nodig:
    cmp  al,' '
    je   .space
    cmp  esi,I_END
    jne  .ok_block
    dec  [cur_block]
    call read_block
    mov  esi,[block_end]
  .ok_block:
    dec  esi
  .space:
    mov eax,ebx
    mov ebx,edx
    push esi
    mov  esi,szKeyword
    call ecTranslateKeyword
    pop  esi
  .__ex:
    pop  edx
    ret

;
; %%Function: ecParseChar
;
; Route the character to the appropriate destination stream.
;

ecParseChar:
;in: ch-al, esi->rtf
   cmp  [ris],risBin
   jne  .nobin
   dec  [cbBin]
   cmp  [cbBin],0
   jg   .nobin
   mov  [ris],risNorm
 .nobin:
   cmp  [rds],rdsColor
   jne  .nodelim
   cmp  al,';'
   jne  .non
   mov  eax,[colorptr]
   cmp  eax,ct_end-4
   jae  .non
   add  [colorptr],4
   jmp  .non
 .nodelim:
   cmp  [rds],rdsSkip
   je  .non
   cmp  [rds],rdsNorm
   je   ecPrintChar
;   ret
 .non:
   mov  eax,ecOK
   ret

macro PrintTrap _char
{
local .notrap
    cmp  byte[esi],_char
    jne  .notrap
    sub  esi,I_END
    dps  'Trapped at '
    dpd  esi
    dps  <13,10>
    ud2
  .notrap:
}
;
; %%Function: ecPrintChar
;
; Send a character to the output file.
;

ecPrintChar:
; in:ch-al, esi - rtf pointer
;    stosb
;    jmp  .nowrap
    mov  ebp,[mode]
    cmp  al,0xa
    jne   .nopar
;    and  ebp,not RTF_NO1STLINE
    and  [mode],not RTF_NO1STLINE
    jmp  .par
  .nopar:
    cmp  al,0x9
    jne  .notab
    add  word[Free+6],CHARW*3
    jmp  .chkwrap
  .notab:
    xor  ebx,ebx
  if ~ RENDER eq FREE
    cmp  word[Free+4],TOP
    jl   .nodraw
  end if
    ansi2oem
    mov  [char],al
;    PrintTrap '/'
    pusha
    xor   eax,eax
;    test  [mode],RTF_BLIND
    test  ebp,RTF_BLIND
    je   .rend
;    test  [mode],RTF_COLORLESS
    test  ebp,RTF_COLORLESS
    jz    .setcolor
    mov   ecx,DEFCOLOR
    jmp   .rend
  .setcolor:
    movzx ecx,byte[Chp+3]
    mov   ecx,[colortbl+ecx*4]
  .rend:
  if RENDER eq FREE
    mov   ebx,Free
    mov   dword[ebx+32],eax
    mov   [ebx+28],ecx
;    test  [mode], RTF_BOTTOM
    test  ebp, RTF_BOTTOM
    jne   .nodraw2
    cmp  word[Free+4],TOP
    jl   .nodraw2
    cmp   byte[Chp],fTrue
    jne   .nobold
    or    dword[ebx+32],BGI_BOLD
  .nobold:
;    test  [mode], RTF_BLIND
    test  ebp, RTF_BLIND
    jne   .freet
  .nodraw2:
    or    dword[ebx+32],BGI_NODRAW
  .freet:
    BGIfont_Freetext

    mov  [Free+4],eax
    test [mode],RTF_BLIND
;    jmp  .nohei
    jne  .nohei
    fild word[BGIheight]
    fmul dword[Free+12]
;    fistp word[curheight]
    fistp word[maxheight]
;    movzx eax,[curheight]
;    dpd  eax
;    cmp  ax,[maxheight]
;    jae  .nohei
;    mov  [maxheight],ax
;    dps 'M'
;    dpd  eax

;    dps <13,10>
  .nohei:
  else
    and   ecx,0xffffff
    mov   ebx,[Free+4]
    mov   edx,char
    mov   esi,1
  end if

  if RENDER eq BGI
    add   ecx,0x44000000
    cmp   byte[Chp],fTrue
    jne   .nobold
    or    esi,BGI_BOLD
  .nobold:
    test  ebp,RTF_BLIND
;    test  [mode],RTF_BLIND
    jne    .freet
    or    esi,BGI_NODRAW
  .freet:
    BGIfont_Outtext
    mov   [Free+4],eax
  end if

  if RENDER eq PIX
;    test  [mode],RTF_TOEOF
;    jne   .blind
;    test  [mode],RTF_BOTTOM
    test  ebp,RTF_BOTTOM
    jne   .nobold
  .blind:
;    test  [mode],RTF_BLIND
    test  ebp,RTF_BLIND
    je    .nobold
    mcall 4
    cmp   byte[Chp],fTrue
    jne   .nobold
    add   ebx,1 shl 16
    mcall
  .nobold:
  end if
    popa
  .nodraw:
  if  RENDER eq PIX
    add  word[Free+6],CHARW
  end if
    movsx eax,[pitch]
    add  word[Free+6],ax
  .chkwrap:
    mov  eax,dword[prcinfo+42]
    cmp  ax,word[Free+6]
    ja   .nowrap
;    or   ebp,RTF_NO1STLINE
    or   [mode],RTF_NO1STLINE
  .par:
    xor  [mode],RTF_BLIND
;    not  [blind]
    test [mode] ,RTF_BLIND
    je   .makewrap
    ; [blind]=false
    movzx eax,word[Free+6]
    sub  ax,word[RetroXY+2]
    push dword[RetroXY]
    pop  dword[Free+4]
    mov  [mark],0xff0000
    test [mode],RTF_ALIGNLESS
    jnz  .letsdraw
    cmp  byte[Pap+12],justR
    jb   .letsdraw
    mov  [mark],0xff
    mov  ebx,dword[prcinfo+42] ; wnd width
    sub  ebx,eax
    cmp  byte[Pap+12],justC
    jne  .nocenter
    shr  ebx,1
    mov  [mark],0x00ff00
  .nocenter:
    mov  word[Free+6],bx
  .letsdraw:
;    test [mode],RTF_NO1STLINE
;    jnz  .no1st
;    add  word[Free+6],60
  .no1st:
  if STEPBYSTEP eq 1
;    movzx eax,[mode]
;    dph  eax
;    test [mode],RTF_NO1STLINE
;    jnz  .no1st
;    mcall 4,[RetroXY],[mark],sym,1
;    dps  '1st '
;  .no1st:
    dps  <'false ',13,10>
;    dpd  eax
;    dpd  ebx
  end if
  if SHOWALIGN eq 1
    mcall 4,[RetroXY],[mark],sym,1
  end if
  if STEPBYSTEP eq 1
    mcall 10
    mcall 2
  end if
    mov  eax,[RetroBlock]
    cmp  eax,[cur_block]
    je   .norblock
    mov  [cur_block],eax
    call read_block
  .norblock:
    mov  esi,[RetroPtr]
    push [RetroGroup]
    pop  [cGroup]
    CopySave RetroSave,Chp
    jmp  .nowrap
  .makewrap:       ; second pass preparing
    ; [blind]=true
  if STEPBYSTEP eq 1
    dps  'true '
    mcall 10
    mcall 2
  end if
    mov  word[Free+6],LMARGIN
  if RENDER eq FREE
    fld [line_space]
    fimul [maxheight]
    fistp [maxheight]
    movzx eax,[maxheight]
    add  word[Free+4],ax
;    and  [maxheight],0
;    add  word[Free+4],CHARH
  else
    mov  eax,CHARH
    add  word[Free+4],ax
  end if
    test [mode],RTF_TOEOF
    je   .nohdoc
    add  [HDoc],eax
    inc  [line_count]
  .nohdoc:
    test [mode],RTF_BOTTOM
    jne  .text
;    dps  '1'
    mov  ebx,dword[prcinfo+46]
    cmp  bx,word[Free+4]
    jge   .text
    or   [mode],RTF_BOTTOM
    dps  <'btm',13,10>
    test [mode],RTF_TOEOF
    jne  .text
    mov  eax,ecOutOfWindow
    ret
;  end if
  .text:
    push dword[Free+4]
    pop  dword[RetroXY]
    mov  word[RetroXY+2],LMARGIN
    mov  [RetroPtr],esi
    push [cur_block]
    pop  [RetroBlock]
    CopySave Chp,RetroSave
    push [cGroup]
    pop  [RetroGroup]
;  if STEPBYSTEP eq 1
;    mcall 10
;    mcall 2
;  end if
  .nowrap:
    mov  eax,ecOK
    ret
mark dd ?
sym db 0x10
line_space dd 1.6
;
; %%Function: ecPushRtfState
;
; Save relevant info on a linked list of SAVE structures.
;

ecPushRtfState:
    pusha
    mov  edi,[listptr]
    mov  eax,edi
    mov  ecx,SIZE_save
    add  eax,ecx
    cmp  eax,save_limit
    jb   .malloc
    Return ecStackOverflow
  .malloc:
    mov  esi,Chp
    rep  movsb
    mov  [listptr],edi
    mov  [ris],risNorm
    inc  [cGroup]
    xor  eax,eax
    Epilog

; %%Function: ecPopRtfState
;
; If we're ending a destination (that is, the destination is changing),
; call ecEndGroupAction.
; Always restore relevant info from the top of the SAVE list.

ecPopRtfState:
    pusha
    mov  esi,[listptr]
    cmp  esi,save_stack
    ja  .okpop
    Return ecStackUnderflow
  .okpop:
    movzx eax,[rds]
    cmp  al,[esi-2]
    je   .noega
    RetError ecEndGroupAction, .noega
  .noega:
    mov  ecx,SIZE_save
    sub  esi,ecx
    mov  [listptr],esi
    mov  edi,Chp
    rep  movsb
    dec  [cGroup]
    xor  eax,eax
    Epilog

ansitbl:
    db 0xaa,0xba,0xbf,0xaf
    db 0xa7,0xa8,0xa1,0xab,0xb0,0xb2,0xb3,0xb6,0xb7,0xb8,0xb9
    db 0xa2,0xbb,0x93,0x94,0x85
oematbl:
 if RENDER eq PIX
    db 0xf2,0xf3,0xf5,0xf4
 else
    db 0x85,0xa5,0x69,0x49
 end if
    db 0x15,0xf0,0xf6,0x22,0x1d,0x49,0x69,0x14,0x1c,0xf1,0x23
    db 0xf7,0x22,0x22,0x22,0x16
uctbl:
    dw 0x451
oemutbl:
    db 0xb8
