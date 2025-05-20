macro Return _result
{
    mov  eax,_result
    jmp  .__ex
}

macro Epilog
{
  .__ex:
    mov  [esp+28],eax
    popa
    ret
}

macro isdigit _reg, lbl_fail
{
   cmp  _reg,'0'
   jb   lbl_fail
   cmp  _reg,'9'
   ja   lbl_fail
}

macro islower _reg, lbl_fail
{
   cmp  _reg,'a'
   jb   lbl_fail
   cmp  _reg,'z'
   ja   lbl_fail
}

macro isalpha _reg, lbl_fail
{
local .ok
   cmp  _reg,'A'
   jb   lbl_fail
   cmp  _reg,'Z'
   jbe  .ok
   islower _reg, lbl_fail
 .ok:
}

macro dump _ptr, _len
{
if DEBUG_BLOCK eq 1
    pusha
    lea  edx,[_ptr]
    dps  'Dump at '
    dpd  edx
    newline
    mov  eax,0x0d0a
    xchg eax,[edx+_len]
    pusha
    call debug_outstr
    popa
    mov  [edx+_len],eax
    mcall 10
    mcall 2
    popa
 end if
}

macro lods_block
{
local .noeof,.eof,.lod,.nog
  .lod:
    lodsb
    cmp  esi,[block_end]
    jb   .noeof
    dump esi-20,20
    mov  esi,[cur_block]
    cmp  esi,[max_block]
    jae   .eof
  if DEBUG_BLOCK eq 1
    pusha
    mcall 4,[Free+4],0xff0000,sym,1
    popa
;    dps  'Block='
;    dpd  esi
  end if
    call read_next_block
;    test [mode],RTF_TOEOF
;    jz   .nog
    test [mode],RTF_BLIND
    jne  .nog
    mov  eax,dword[prcinfo+42]
    mul  [cur_block]
    div  [max_block]
    mov  edx,0xff0000
    call draw_progress
  .nog:
    dump esi,20
    jmp  .noeof;lod
  .eof:
    mov  eax,ecOK;ecEndOfFile
    jmp  .__ex
  .noeof:
}

macro RetError routine, route
{
    call routine
    test  eax,eax
  if route eq
    je  .getc
  else
    je  route
  end if
    Return eax
}

macro Prcheck _prop
{
local .no
    cmp ch, prop#_prop
    jne .no
    mov esi, _prop
    jmp .checked
  .no:
}

macro MemSet _prop,_struc
{
local .no
    cmp  al,_prop
    jne .no
    mov  ecx,SIZE_#_struc
    mov  edi, _struc
    jmp  .ms
  .no:
}

macro atoi
{
; in:  esi->pasParameter
; out: eax->long
local .nxt
    push ebx ecx
    xor  eax,eax
    xor  ebx,ebx
    movzx ecx,byte[esi]
    inc  esi
  .nxt:
    lodsb
    sub  eax,'0'
    imul ebx,10
    add  ebx,eax
    loop .nxt
    mov  eax,ebx
    pop  ecx ebx
}

macro uc2oem
{
local .ex1
    push edi
    mov  ecx,(oemutbl-uctbl)/2+1
    mov  edi,uctbl
    repne scasw
    jne  .ex1
    sub  edi,uctbl
    shr  edi,1
    dec  edi
    mov  al,[oemutbl+edi]
  .ex1:
    pop  edi
}

macro ansi2oem
{
local .ex2,.notintbl
    push edi ecx
    mov  ecx,oematbl-ansitbl
    mov  edi,ansitbl
    repne scasb
    jne  .notintbl
    dec  edi
    mov  al,[oematbl-ansitbl+edi]
    jmp  .ex2
  .notintbl:
    cmp  al,0xc0
    jb   .ex2
    sub  al,0x40
    cmp  al,0xb0
    jb   .ex2
    add  al,0x30
  .ex2:
    pop  ecx edi
}

draw_progress:
    lea  ebx,[eax+05 shl 16]
    mov  ecx,dword[prcinfo+46]
    shl  ecx,16
    add  ecx,2+5 shl 16
    mcall 13
    ret

fTrue equ 1
fFalse equ 0

macro CHP                  ; CHaracter Properties
{
    db ?;fBold;             0
    db ?;fUnderline;        1
    db ?;fItalic;           2
    db ?;fColor             3
    db ?;fFont              4
    dd ?;fFontSize          5
}
SIZE_Chp equ 9

macro PAP                                 ; PAragraph Properties
{
    dd ?;xaLeft;                 ; left indent in twips           0
    dd ?;xaRight;                ; right indent in twips          4
    dd ?;xaFirst;                ; first line indent in twips     8
    db ?;   just;                   ; justification               12
}
SIZE_Pap equ 13

macro SEP
{
    dd ?;cCols;                  ; number of columns                    0
    db ?;sbk;                      ; section break type                 4
    dd ?;xaPgn;                  ; x position of page number in twips   5
    dd ?;yaPgn;                  ; y position of page number in twips   9
    db ?;pgnFormat;                ; how the page number is formatted   13
}                                 ; SEction Properties
SIZE_Sep equ 14

macro DOP
{
    dd ?;xaPage;                 ; page width in twips                  0
    dd ?;yaPage;                 ; page height in twips                 4
    dd ?;xaLeft;                 ; left margin in twips                 8
    dd ?;yaTop;                  ; top margin in twips                  12
    dd ?;xaRight;                ; right margin in twips                16
    dd ?;yaBottom;               ; bottom margin in twips               20
    dd ?;pgnStart;               ; starting page number in twips        24
    db ?;fFacingp;              ; facing pages enabled?                 28
    db ?;fLandscape;            ; landscape or portrait??               29
} ;                  ; DOcument Properties
SIZE_Dop equ 30

justL equ 0
justF equ 1
justR equ 2
justC equ 3

sbkNon equ 0
sbkCol equ 1
sbkEvn equ 2
sbkOdd equ 3
sbkPg  equ 4

pgDec equ 0
pgURom equ 1
pgLRom equ 2
pgULtr equ 3
pgLLtr equ 4

rdsNorm equ 0
rdsSkip equ 1              ; Rtf Destination State
rdsColor equ 2

risNorm equ 0
risBin equ 1
risHex equ 2       ; Rtf Internal State

macro SAVE             ; property save macrostructure
{
    CHP ;chp;        0
    PAP ;pap;        9
    SEP ;sep;        22
    DOP ;dop;        36
    RDS db ?;rds;    66
    RIS db ?;ris;    67
}
SIZE_save equ (SIZE_Chp+SIZE_Pap+SIZE_Sep+SIZE_Dop+2)

; What types of properties are there?
ipropBold equ 0
ipropItalic equ 1
ipropUnderline equ 2
ipropLeftInd equ 3
ipropRightInd equ 4
ipropFirstInd equ 5
ipropCols equ 6
ipropPgnX equ 7
ipropPgnY equ 8
ipropXaPage equ 9
ipropYaPage equ 10
ipropXaLeft equ 11
ipropXaRight equ 12
ipropYaTop equ 13
ipropYaBottom equ 14
ipropPgnStart equ 15
ipropSbk equ 16
ipropPgnFormat equ 17
ipropFacingp equ 18
ipropLandscape equ 19
ipropJust equ 20
ipropPard equ 21
ipropPlain equ 22
ipropSectd equ 23
ipropForeColor equ 24
ipropFont equ 25
ipropFontSize equ 26
ipropMax equ 27


actnSpec equ 0
actnByte equ 1
actnWord equ 2
actnFloatFS equ 3

propChp equ 0
propPap equ 1
propSep equ 2
propDop equ 3

macro PROP _actn,_prop,_offset
{
    db _actn;      0        ; size of value
    db _prop;      1        ; macrostructure containing value
    db _offset;    2        ; offset of value from base of macrostructure
}
PROP_SIZE equ 3

ipfnBin equ 0
ipfnHex equ 1
ipfnSkipDest equ 2

idestPict equ 0
idestSkip equ 1
idestColor equ 2

kwdChar equ 0
kwdDest equ 1
kwdProp equ 2
kwdSpec equ 3
kwdColor equ 4
kwdUC equ 5

isymMax =0
isymImplemented=0
macro SYM _szKeyword,_dflt,_fPassDflt,_kwd,_idx,_impl
{
local .r
  if  _impl eq 1
    isymImplemented=isymImplemented+1
  end if
    dw  _dflt;        0      ; default value to use
    db  _fPassDflt;   2      ; true to use default value from this table
    db  _kwd;         3      ; base action to take
    db  _idx;         4      ; index into property table if kwd == kwdProp
                            ; index into destination table if kwd == kwdDest
                            ; character to print if kwd == kwdChar
    db  .r-$-1 ;        5
    db  _szKeyword;        ; RTF keyword
.r:
    isymMax=isymMax+1
}
; RTF parser error codes

ecOK equ                0       ; Everything's fine!
ecStackUnderflow equ    1       ; Unmatched '}'
ecStackOverflow equ     2       ; Too many '{' -- memory exhausted
ecUnmatchedBrace equ    3       ; RTF ended during an open group.
ecInvalidHex equ        4       ; invalid hex character found in data
ecBadTable equ          5       ; RTF table (sym or prop) invalid
ecAssertion equ         6       ; Assertion failure
ecEndOfFile equ         7       ; End of file reached while reading RTF
ecOutOfWindow equ       8       ; Decoding interrupted after window edge
