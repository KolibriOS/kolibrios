
rgsymRtf:
; Keyword descriptions
;rgsymRtf[] = {
;  keyword     dflt    fPassDflt   kwd         idx    implemented
if  syms eq 12
SYM    "f",        0,      fFalse,     kwdProp,    ipropFont
SYM    "*",        0,      fFalse,     kwdSpec,    ipfnSkipDest    ,1
SYM    "'",        0,      fFalse,     kwdSpec,    ipfnHex         ,1
SYM    "par",      0,      fFalse,     kwdChar,    0x0a            ,1
SYM    "u",        0,      fFalse,     kwdUC,      0               ,1
SYM    "line",     0,      fFalse,     kwdChar,    0xa             ,1
SYM    "plain",    0,      fFalse,     kwdProp,    ipropPlain      ,1
SYM    0x9,        0,      fFalse,     kwdChar,    0x09            ,1
SYM    "tab",      0,      fFalse,     kwdChar,    0x09            ,1
SYM    "b",        1,      fFalse,     kwdProp,    ipropBold       ,1
SYM    "ul",       1,      fFalse,     kwdProp,    ipropUnderline
SYM    "i",        1,      fFalse,     kwdProp,    ipropItalic     ,1
SYM    "li",       0,      fFalse,     kwdProp,    ipropLeftInd
SYM    "ri",       0,      fFalse,     kwdProp,    ipropRightInd
SYM    "fi",       0,      fFalse,     kwdProp,    ipropFirstInd
SYM    "~",        0,      fFalse,     kwdChar,    ' '             ,1
SYM    "cols",     1,      fFalse,     kwdProp,    ipropCols
SYM    "qc",       justC,  fTrue,      kwdProp,    ipropJust       ,1
SYM    "ql",       justL,  fTrue,      kwdProp,    ipropJust       ,1
SYM    "qr",       justR,  fTrue,      kwdProp,    ipropJust       ,1
SYM    "qj",       justF,  fTrue,      kwdProp,    ipropJust
SYM    "cf",       0,      fFalse,     kwdProp,    ipropForeColor  ,1
SYM    "red",      0,      fFalse,     kwdColor,   16              ,1
SYM    "green",    0,      fFalse,     kwdColor,   8               ,1
SYM    "blue",     0,      fFalse,     kwdColor,   0               ,1
SYM    "fs",       0,      fFalse,     kwdProp,    ipropFontSize   ,1
SYM    "sbknone",  sbkNon, fTrue,      kwdProp,    ipropSbk
SYM    "sbkcol",   sbkCol, fTrue,      kwdProp,    ipropSbk
SYM    "sbkeven",  sbkEvn, fTrue,      kwdProp,    ipropSbk
SYM    "sbkodd",   sbkOdd, fTrue,      kwdProp,    ipropSbk
SYM    "sbkpage",  sbkPg,  fTrue,      kwdProp,    ipropSbk
SYM    "pgnx",     0,      fFalse,     kwdProp,    ipropPgnX
SYM    "pgny",     0,      fFalse,     kwdProp,    ipropPgnY
SYM    "pgndec",   pgDec,  fTrue,      kwdProp,    ipropPgnFormat
SYM    "pgnucrm",  pgURom, fTrue,      kwdProp,    ipropPgnFormat
SYM    "pgnlcrm",  pgLRom, fTrue,      kwdProp,    ipropPgnFormat
SYM    "pgnucltr", pgULtr, fTrue,      kwdProp,    ipropPgnFormat
SYM    "pgnlcltr", pgLLtr, fTrue,      kwdProp,    ipropPgnFormat
SYM    "paperw",   12240,  fFalse,     kwdProp,    ipropXaPage
SYM    "paperh",   15480,  fFalse,     kwdProp,    ipropYaPage
SYM    "margl",    1800,   fFalse,     kwdProp,    ipropXaLeft
SYM    "margr",    1800,   fFalse,     kwdProp,    ipropXaRight
SYM    "margt",    1440,   fFalse,     kwdProp,    ipropYaTop
SYM    "margb",    1440,   fFalse,     kwdProp,    ipropYaBottom
SYM    "pgnstart", 1,      fTrue,      kwdProp,    ipropPgnStart
SYM    "facingp",  1,      fTrue,      kwdProp,    ipropFacingp
SYM    "landscape",1,      fTrue,      kwdProp,    ipropLandscape
SYM    "ldblquote",0,      fFalse,     kwdChar,    '"'             ,1
SYM    "rdblquote",0,      fFalse,     kwdChar,    '"'             ,1
SYM    "bin",      0,      fFalse,     kwdSpec,    ipfnBin         ,1
SYM    "author",   0,      fFalse,     kwdDest,    idestSkip
SYM    "buptim",   0,      fFalse,     kwdDest,    idestSkip
SYM    "bullet",   0,      fFalse,     kwdChar,    '*'             ,1
SYM    "cell",     0,      fFalse,     kwdChar,    0x09            ,1
SYM    "colortbl", 0,      fFalse,     kwdDest,    idestColor      ,1
SYM    "comment",  0,      fFalse,     kwdDest,    idestSkip
SYM    "creatim",  0,      fFalse,     kwdDest,    idestSkip
SYM    "doccomm",  0,      fFalse,     kwdDest,    idestSkip
SYM    "fonttbl",  0,      fFalse,     kwdDest,    idestSkip
SYM    "footer",   0,      fFalse,     kwdDest,    idestSkip
SYM    "footerf",  0,      fFalse,     kwdDest,    idestSkip
SYM    "footerl",  0,      fFalse,     kwdDest,    idestSkip
SYM    "footerr",  0,      fFalse,     kwdDest,    idestSkip
SYM    "footnote", 0,      fFalse,     kwdDest,    idestSkip
SYM    "ftncn",    0,      fFalse,     kwdDest,    idestSkip
SYM    "ftnsep",   0,      fFalse,     kwdDest,    idestSkip
SYM    "ftnsepc",  0,      fFalse,     kwdDest,    idestSkip
SYM    "header",   0,      fFalse,     kwdDest,    idestSkip
SYM    "headerf",  0,      fFalse,     kwdDest,    idestSkip
SYM    "headerl",  0,      fFalse,     kwdDest,    idestSkip
SYM    "headerr",  0,      fFalse,     kwdDest,    idestSkip
SYM    "info",     0,      fFalse,     kwdDest,    idestSkip
SYM    "keywords", 0,      fFalse,     kwdDest,    idestSkip
SYM    "operator", 0,      fFalse,     kwdDest,    idestSkip
SYM    "pict",     0,      fFalse,     kwdDest,    idestSkip
SYM    "printim",  0,      fFalse,     kwdDest,    idestSkip
SYM    "private1", 0,      fFalse,     kwdDest,    idestSkip
SYM    "revtim",   0,      fFalse,     kwdDest,    idestSkip
SYM    "row",      0,      fFalse,     kwdChar,    0x0a            ,1
SYM    "rxe",      0,      fFalse,     kwdDest,    idestSkip
SYM    "stylesheet",0,     fFalse,     kwdDest,    idestSkip
SYM    "subject",  0,      fFalse,     kwdDest,    idestSkip
SYM    "tc",       0,      fFalse,     kwdDest,    idestSkip
SYM    "title",    0,      fFalse,     kwdDest,    idestSkip
SYM    "trowd",    0,      fFalse,     kwdChar,    0x0a            ,1
SYM    "txe",      0,      fFalse,     kwdDest,    idestSkip
SYM    "xe",       0,      fFalse,     kwdDest,    idestSkip
SYM    "{",        0,      fFalse,     kwdChar,    '{'             ,1
SYM    "}",        0,      fFalse,     kwdChar,    '}'             ,1
SYM    "\",        0,      fFalse,     kwdChar,    '\'             ,1
end if
SYM    0xa,        0,      fFalse,     kwdChar,    0x0a            ,1
SYM    0xd,        0,      fFalse,     kwdChar,    0x0a            ,1

;
; %%Function: ecApplyPropChange
;
; Set the property identified by _iprop_ to the value _val_.
;
ecApplyPropChange:
; in : eax-iprop, ebx - value
    pusha
    cmp  [rds],rdsSkip
    jne  .noskip
  .ok:
    Return ecOK
  .noskip:
    lea  esi,[eax*2+eax+rgprop]  ;rgprop[iprop]
    mov  cx,word[esi]
    movzx edx,byte[esi+2]
    Prcheck Dop
    Prcheck Sep
    Prcheck Pap
    Prcheck Chp
    cmp  byte[esi],actnSpec
    jne  .bad
  .checked:
    cmp  cl,actnByte
    jne  .nob
    mov  byte[esi+edx],bl
    jmp  .ok
  .nob:
    cmp  cl,actnWord
    jne  .now
    mov  [esi+edx],ebx
    jmp  .ok
  .now:
    cmp  cl,actnSpec
    jne  .nos
    call ecParseSpecialProperty
    jmp  .__ex
  .nos:
    cmp  cl,actnFloatFS
    jne  .bad
    mov  [Chp+5],ebx
    call updFS
    jmp  .ok
  .bad:
    mov  eax,ecBadTable
    Epilog

updFS:
    cmp  dword[Chp+5],0
    jnz  .no0
    mov  dword[Chp+5],24
  .no0:
    fild dword[Chp+5]
    fmul dword[Free-4]
    fst  dword[Free+12]
    fstp dword[Free+16]
    ret
;
; %%Function: ecParseSpecialProperty
;
; Set a property that requires code to evaluate.
;

ecParseSpecialProperty:
; in : eax-iprop, ebx - value
;    cmp  al,ipropPard
;    jne  .nopap
;    push eax
;    mov  ecx,SIZE_Pap
;    mov  edi,RetroSave+9
;    xor  eax,eax
;    rep stosb
;    pop  eax
;  .nopap:
    MemSet ipropPard,Pap
    MemSet ipropPlain,Chp
    MemSet ipropSectd,Sep
    mov  eax,ecBadTable
    ret
  .ms:
    xor  eax,eax
    rep  stosb
    ret

;
; %%Function: ecTranslateKeyword.
;
; Step 3.
; Search rgsymRtf for szKeyword and evaluate it appropriately.
;
; Inputs:
; szKeyword:   The RTF control to evaluate.
; param:       The parameter of the RTF control.
; fParam:      fTrue if the control had a parameter; (that is, if param is valid

;              fFalse if it did not.
ecTranslateKeyword:
; in: esi->pasKeyword; ebx-param, eax-fParam
    pusha
    mov  edx,isymMax
    mov  edi,rgsymRtf
  .next:
    add  edi,5
    movzx ecx,byte[edi]
    inc  ecx
    push  edi esi ecx
    repe cmpsb
    pop  ecx esi edi
    jz   .found
    add  edi,ecx
;    inc edi
    dec  edx
    test edx,edx
    jne  .next
    cmp  dl,[fSkipDestIfUnk]
    jz   .discard
    mov  [rds],rdsSkip
    mov  [fSkipDestIfUnk],dl
  .discard:
    Return ecOK
  .found:
    and  [fSkipDestIfUnk],0
    mov  cx,[edi-3]
    cmp  ch,kwdProp
    jne  .nop
    test cl,cl
    jnz  .usedeflt
    test al,al
    jnz  .validparm
  .usedeflt:
    mov  ebx,[edi-5]
  .validparm:
    movzx eax,byte[edi-1]
    call ecApplyPropChange
    jmp  .__ex
  .nop:
    movzx eax,byte[edi-1]
    cmp  ch,kwdChar
    jne  .noc
  .pchar:
    mov  edi,[esp]
    test eax,eax
    jz   .__ex
    mov  esi,[esp+36]
    call  ecParseChar
    mov  [esp],edi
    mov  [esp+36],esi
    jmp  .__ex
  .noc:
    cmp  ch,kwdDest
    jne  .nod
    call ecChangeDest
    jmp  .__ex
  .nod:
    cmp  ch,kwdSpec
    jne  .nos
    call ecParseSpecialKeyword
    jmp  .__ex
  .nos:
    cmp  ch,kwdColor
    jne  .nocol
    cmp  [rds],rdsColor
    jne  .bad
    mov  edx,[colorptr]
    mov  ecx,eax
    mov  esi,0xff
    shl  esi,cl
    not  esi
    shl  ebx,cl
    and  [edx],esi
    or   [edx],ebx
  .okk:
    Return ecOK
  .nocol:
    cmp  ch,kwdUC
    jne  .bad
    mov  eax,[esp+36]
    cmp  byte[eax],'\'
    je   .nobyte
    inc  dword[esp+36]
  .nobyte:
    cmp  ebx,0x400
    jb   .okk
    mov  eax,ebx
    uc2oem
    cmp  ecx,1
    jae  .pchar
    lea  eax,[ebx-0x410+0xc0]
    jmp  .pchar
  .bad:
    mov  eax,ecBadTable
    Epilog

; %%Function: ecChangeDest
;
; Change to the destination specified by idest.
; There's usually more to do here than this...

ecChangeDest:
; in: eax- idest
    cmp  [rds],rdsSkip
    jne  .noskip
    jmp  .okk;Return ecOK
  .noskip:
    cmp  eax,rdsColor
    jne  .nocol
    mov  [rds],al
    jmp  .okk
  .nocol:
    mov  [rds],rdsSkip
  .okk:
    mov  eax,ecOK
  .ex:
    ret

; %%Function: ecEndGroupAction
;
; The destination specified by rds is coming to a close.
; If there's any cleanup that needs to be done, do it now.
ecEndGroupAction:
; in: eax-rds
      mov  eax,ecOK
      ret

; %%Function: ecParseSpecialKeyword
;
; Evaluate an RTF control that needs special processing.

ecParseSpecialKeyword:
; in: eax-IPFN
    cmp  [rds],rdsSkip
    jne  .noskip
    cmp  eax,ipfnBin
    jne  .ok
  .noskip:
    cmp  eax,ipfnBin
    jne  .nobin
    mov  [ris],risBin
    push [lParam]
    pop  [cbBin]
    jmp  .ok
  .nobin:
    cmp  eax,ipfnSkipDest
    jne  .noskd
    mov  [fSkipDestIfUnk],fTrue
    jmp  .ok
  .noskd:
    cmp  eax,ipfnHex
    jne  .nohex
    mov  [ris],risHex
    jmp  .ok
  .nohex:
    Return ecBadTable
  .ok:
    mov  eax,ecOK
  .__ex:
    ret

; RTF parser tables
rgprop:
; Property descriptions
;PROP rgprop [ipropMax] = {
PROP    actnByte,   propChp,    0       ; ipropBold
PROP    actnByte,   propChp,    1     ; ipropItalic
PROP    actnByte,   propChp,    2  ; ipropUnderline
PROP    actnWord,   propPap,    0      ; ipropLeftInd
PROP    actnWord,   propPap,    4     ; ipropRightInd
PROP    actnWord,   propPap,    8     ; ipropFirstInd
PROP    actnWord,   propSep,    0       ; ipropCols
PROP    actnWord,   propSep,    5       ; ipropPgnX
PROP    actnWord,   propSep,    9       ; ipropPgnY
PROP    actnWord,   propDop,    0      ; ipropXaPage
PROP    actnWord,   propDop,    4      ; ipropYaPage
PROP    actnWord,   propDop,    8      ; ipropXaLeft
PROP    actnWord,   propDop,    12     ; ipropXaRight
PROP    actnWord,   propDop,    16       ; ipropYaTop
PROP    actnWord,   propDop,    20    ; ipropYaBottom
PROP    actnWord,   propDop,    24    ; ipropPgnStart
PROP    actnByte,   propSep,    4         ; ipropSbk
PROP    actnByte,   propSep,    13   ; ipropPgnFormat
PROP    actnByte,   propDop,    28    ; ipropFacingp
PROP    actnByte,   propDop,    29  ; ipropLandscape
PROP    actnByte,   propPap,    12  ; ipropJust
PROP    actnSpec,   propPap,    0   ; ipropPard
PROP    actnSpec,   propChp,    0   ; ipropPlain
PROP    actnSpec,   propSep,    0   ; ipropSectd
PROP    actnByte,   propChp,    3   ; ipropForeColor
PROP    actnByte,   propChp,    4   ; ipropFont
PROP    actnFloatFS,propChp,    5   ; ipropFontSize
