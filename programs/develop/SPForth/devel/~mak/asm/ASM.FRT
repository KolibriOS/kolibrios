(  Из SMAL32
   Модифицированно Максимовым М.О.
  email:mak@rtc.ru
  http://forth.spb.su:8888
  т д {812}705-92-03
  т р {812}552-47-64
)
REQUIRE [IF] ~MAK\CompIF.f
REQUIRE PLACE ~MAK\PLACE.f
REQUIRE CASE  lib\ext\case.f

VOCABULARY S_ASSEM
C" HIDDEN" FIND NIP 0=
[IF] VOCABULARY HIDDEN
[THEN]

ALSO HIDDEN DEFINITIONS

C" FLOAD" FIND NIP
[IF]   FLOAD asmbase.f
[ELSE] REQUIRE S= ~mak\asm\asmbase.f
[THEN]
: 1Op1 ( name ( byte --> )
              ( --> )
   CREATE C, DOES> C@ C,
;

4 VALUE *DefDatasz
\ 4 VALUE *DefDatasz

: 1Op1W ( name ( byte --> )
              ( --> )
   CREATE C, DOES> C@ C, *DefDatasz 4 = IF 0x66 C, THEN
;

: 1Op1D ( name ( byte --> )
              ( --> )
   CREATE C, DOES> C@ C, *DefDatasz 2 = IF 0x66 C, THEN
;

: 1Op2 ( name ( word --> )
              ( --> )
   CREATE W, DOES> W@ W,
;
:  REF-ERROR IF 87 THROW THEN ;
:  #OPER-ERROR  SWAP IF H. 94 THROW THEN DROP ;

0x2E 1Op1 CS:
0x3E 1Op1 DS:
0x26 1Op1 ES:
0x64 1Op1 FS:
0x65 1Op1 GS:
0x36 1Op1 SS: 
: IFIND ( c-addr --> xt -1 | c-addr 0 )
   DUP  COUNT CONTEXT @ SEARCH-WORDLIST
   IF   NIP -1
   ELSE 0
   THEN  ;

0 VALUE *OpcSize
0 VALUE *OpCode
0 VALUE *Mod
0 VALUE *Reg
0 VALUE *R/M
0 VALUE *Scale
0 VALUE *Index
0 VALUE *Base
0 VALUE *OpSize
0 VALUE *ImSize
0 VALUE *AdSize
0 VALUE *Imm
0 VALUE *OfSize
0 VALUE *Offset
0 VALUE *SegReg
0 VALUE *OpArray
0 VALUE *OfRel
0 VALUE *OpRel
CREATE *OffName 256 ALLOT
CREATE *ImmName 256 ALLOT
CREATE *GenName 256 ALLOT

: (OthSz) ( n1 --> n2 )   6 XOR ;

: *OpSize>DEF *OpSize 4 = IF *DefDatasz TO *OpSize THEN ;

: (Seg) ( --> )
   *SegReg CASE
    0 OF ES: ENDOF
    1 OF CS: ENDOF
    2 OF SS: ENDOF
    3 OF DS: ENDOF
    4 OF FS: ENDOF
    5 OF GS: ENDOF
           ENDCASE
;

: CompileCommand ( --> )
   *OpcSize IF
    *AdSize *DefDatasz (OthSz) = IF 0x67 C, THEN
    *OpSize *DefDatasz (OthSz) = IF 0x66 C, THEN
    *SegReg 0< IFNOT (Seg) THEN
    *OpCode HERE ! *OpcSize ALLOT
    *Mod 0< IFNOT *Mod 3 AND 0x40 * *Reg 7 AND 8 * *R/M 7 AND OR OR C, THEN
    *Scale 0< *Mod 0< OR IFNOT
     *Scale 3 AND 0x40 * *Index 7 AND 8 * *Base 7 AND OR OR C,
    THEN
    *OfRel IF
     *OfSize *DefDatasz <> IF 0x54 THROW THEN 
     *OffName C@ IFNOT 1 *OffName 1+ C! THEN
     *OffName 1+ C@ IF *OffName HERE 2 AddStrObject
     ELSE
      *OffName C@ 1- *OffName 1+ C!
      *OffName 1+ HERE 5 AddStrObject
     THEN
    THEN
    *Offset HERE ! *OfSize ALLOT
    *OpRel IF
     *ImSize *DefDatasz <> IF 0x55 THROW THEN
     *ImmName C@ IFNOT 1 *ImmName 1+ C! THEN
     *ImmName 1+ C@ IF *ImmName HERE 2 AddStrObject
     ELSE
      *ImmName C@ 1- *ImmName 1+ C!
      *ImmName 1+ HERE 5 AddStrObject
     THEN
    THEN
    *Imm HERE ! *ImSize ALLOT
   THEN
;
: InitCommand ( --> )
   0 TO *OpcSize 0 TO *AdSize 0 TO *OpSize 0 TO *ImSize 0 TO *OfSize
   0 TO *OfRel 0 TO *OpRel 0 TO *Offset 0 TO *Imm -1 TO *SegReg -1 TO *Mod
   -1 TO *Scale -1 TO *Base -1 TO *Index *OffName 0! *ImmName 0! *GenName 0!
;

: GetOp ( --> c-addr|char|n type )
   (GetOp) CASE
    1 OF
      1+ C@ DUP [CHAR] ; = IF SOURCE >IN ! 2DROP NullString 0 ELSE 1 THEN
     ENDOF
    2 OF
      BASE @ >R
      DUP C@ OVER + C@ CASE
       [CHAR] H OF 16 BASE ! DUP 1-! ENDOF
       [CHAR] B OF 2 BASE ! DUP 1-! ENDOF
       [CHAR] O OF 8 BASE ! DUP 1-! ENDOF
       [CHAR] D OF 10 BASE ! DUP 1-! ENDOF
            ENDCASE
      VAL 0= IF 0x61 THROW THEN DROP R> BASE ! 2
     ENDOF
    4 OF COUNT ConvertString OVER 1- C! 1- 4 ENDOF
    5 OF COUNT ConvertString OVER 1- C! 1- 5 ENDOF
\       HEX
     DUP
\    [ HERE DROP ]
   ENDCASE
;

: SCopy ( c-addr1 c-addr2 --> )
   OVER C@ 1+ CMOVE
;

: ?Reg8 ( --> n true | false )
   IN>R GetOp 3 = IF
    C" AL" S= 0 ?S=
    C" CL" S= 1 ?S=
    C" DL" S= 2 ?S=
    C" BL" S= 3 ?S=
    C" AH" S= 4 ?S=
    C" CH" S= 5 ?S=
    C" DH" S= 6 ?S=
    C" BH" S= 7 ?S=
   THEN
   DROP FALSE R>IN
;
: ?Reg16 ( --> n true | FALSE )
   IN>R GetOp 3 = IF
    C" AX" S= 0 ?S=
    C" CX" S= 1 ?S=
    C" DX" S= 2 ?S=
    C" BX" S= 3 ?S=
    C" SP" S= 4 ?S=
    C" BP" S= 5 ?S=
    C" SI" S= 6 ?S=
    C" DI" S= 7 ?S=
   THEN
   DROP FALSE R>IN
;
: ?Reg32 ( --> n true | FALSE )
   IN>R GetOp 3 = IF
    C" EAX" S= 0 ?S=
    C" ECX" S= 1 ?S=
    C" EDX" S= 2 ?S=
    C" EBX" S= 3 ?S=
    C" ESP" S= 4 ?S=
    C" EBP" S= 5 ?S=
    C" ESI" S= 6 ?S=
    C" EDI" S= 7 ?S=
   THEN
   DROP FALSE R>IN
;
: ?SegReg ( --> n true | FALSE )
   IN>R GetOp 3 = IF
    C" ES" S= 0 ?S=
    C" CS" S= 1 ?S=
    C" SS" S= 2 ?S=
    C" DS" S= 3 ?S=
    C" FS" S= 4 ?S=
    C" GS" S= 5 ?S=
   THEN
   DROP FALSE R>IN
;
: SEG ( --> )
   ?SegReg 0= IF 104 THROW THEN (Seg)
;
: (?Ptr_) ( --> n true | FALSE )
   IN>R GetOp 3 = IF
    C" DWORD" S= 4 ?S=
    C" BYTE"  S= 1 ?S=
    C" WORD"  S= 2 ?S=
   THEN
   DROP FALSE R>IN
;

' (?Ptr_) ->VECT (?Ptr)
: ?Ptr ( --> flag )
   (?Ptr) IF
    *OpSize IF *OpSize <> IF 0x50 THROW THEN ELSE TO *OpSize THEN
    IN>R GetOp 3 = IF
     C" PTR" S= IF RDROP TRUE EXIT ELSE DROP THEN
    ELSE DROP THEN
    R>IN TRUE
   ELSE FALSE THEN
;
: ?FWord ( ? --> ? )
   IN>R GetOp 3 = IF
    C" FWORD" S= IF
     RDROP IN>R GetOp 3 = IF
      C" PTR" S= IF RDROP ELSE R>IN DROP THEN
     ELSE R>IN DROP THEN TRUE EXIT
    ELSE DROP FALSE THEN
   ELSE DROP FALSE THEN R>IN
;
: ?Reg ( --> flag )
   ?Reg32 IF
    *OpSize 1 2 BETWEEN IF 0x50 THROW THEN 4 TO *OpSize TO *Reg TRUE
   ELSE
    ?Reg8 IF
     *OpSize 2 4 BETWEEN IF 0x50 THROW THEN 1 TO *OpSize TO *Reg TRUE
    ELSE
     ?Reg16 IF
      *OpSize 4 = *OpSize 1 = OR IF 0x50 THROW THEN 2 TO *OpSize TO *Reg TRUE
     ELSE FALSE THEN
    THEN
   THEN
;
: Comma ( --> )
   GetOp 1 = IF [CHAR] , = ?EXIT THEN  0x51 THROW 
;
: ?Colon ( --> flag )
   IN>R GetOp 1 = IF [CHAR] : = ELSE DROP FALSE THEN
   DUP IF RDROP ELSE R>IN THEN
;
: ?Bracket[ ( --> flag )
   IN>R GetOp 1 = IF [CHAR] [ = IF RDROP TRUE EXIT THEN
   ELSE DROP THEN R>IN FALSE
;
: (?Label) ( --> addr|0 TRUE | FALSE )
   IN>R GetOp DUP 3 = IF
    DROP DUP 1 FindStrObject IF RDROP NIP TRUE
    ELSE
     RDROP \ DUP FIND IF
\      SWAP >R DUP >Name Off_Attr + @ 0x1000000 AND IF  \ IF PUBLIC
\       RDROP EXECUTE TRUE
\      ELSE
\       *GenName C@ REF-ERROR R> *GenName SCopy DROP 0 TRUE
\      THEN
\     ELSE
    (  DROP ) *GenName C@ REF-ERROR *GenName SCopy 0 TRUE
\     THEN
    THEN
   ELSE
    DUP 1 = IF
     DROP [CHAR] $ = IF RDROP HERE TRUE
     ELSE R>IN FALSE THEN
    ELSE
     4 = IF
      *GenName C@ REF-ERROR
      *GenName 1+ SCopy *GenName 1+ C@ 1+ *GenName W! RDROP 0 TRUE
     ELSE DROP R>IN FALSE THEN
    THEN
   THEN
;
: 'XFA ( --> cfa )
   GetOp 3 5 BETWEEN IF
    CONTEXT @ >R PREVIOUS FIND ALSO R> CONTEXT !
    0= IF 0x64 THROW THEN
   ELSE   0x63 THROW THEN 
;
: ?Label ( --> addr|0 TRUE | FALSE )
   IN>R GetOp
\       DUP 1 = 
\    IF DROP [CHAR] { = 
\       IF    RDROP [CHAR] } PARSE EVALUATE TRUE
\       ELSE  R>IN FALSE
\       THEN  EXIT
\    ELSE
     3 = IF
       C" OFFSET" S= IF RDROP (?Label) 0= IF 0x5D THROW THEN  TRUE EXIT
                     THEN
          C" CFA" S= IF RDROP 'XFA TRUE EXIT THEN
          C" PFA" S= IF RDROP 'XFA >BODY TRUE EXIT THEN
          C" NFA" S= IF RDROP 'XFA >NAME TRUE EXIT THEN
     THEN
\   THEN
   DROP R>IN (?Label)
;

: ?Number ( --> n TRUE | FALSE )
   IN>R GetOp CASE
    2 OF TRUE RDROP ENDOF
    1 OF DUP
      [CHAR] { = IF DROP  [CHAR] } PARSE EVALUATE TRUE RDROP ELSE
      [CHAR] - = IF GetOp 2 = IF NEGATE TRUE RDROP 
                    ELSE DROP FALSE R>IN THEN       ELSE
      FALSE R>IN THEN THEN
     ENDOF
    5 OF DUP C@ IF 1+ C@ ELSE DROP 0 THEN TRUE RDROP ENDOF
    3 OF 7 FindStrObject IF RDROP TRUE ELSE FALSE R>IN THEN ENDOF
    2DROP FALSE DUP R>IN
   ENDCASE
;
VARIABLE *Sign
: Imm ( --> )
   *Sign 0! 0 TO *Imm 0 TO *OpRel *GenName 0!
   BEGIN
    ?Number IF
\     *Sign @ IF -To *Imm ELSE +To *Imm THEN
     *Sign @ IF NEGATE THEN  *Imm + TO *Imm
    ELSE
     ?Label IF
      *GenName C@ IF
       *ImmName C@ REF-ERROR *GenName *ImmName SCopy
       *Sign @ REF-ERROR *GenName 0!
      THEN
\      *Sign @ IF 1-To *OpRel -To *Imm ELSE 1+To *OpRel +To *Imm THEN
      *Sign @ IF   *OpRel 1- TO *OpRel NEGATE
              ELSE *OpRel 1+ TO *OpRel
              THEN  *Imm + TO *Imm
     ELSE
      GetOp IF 0x59 ELSE 0x66 THEN THROW 
     THEN
    THEN
    *OpRel 0 1 BETWEEN 0= REF-ERROR
    *OpRel IF
     *OpSize 1 =
     *OpSize *DefDatasz (OthSz) = OR IF 0x50 THROW THEN 
     *DefDatasz TO *OpSize
    THEN
    GetOp DUP 1 > REF-ERROR
    IF
     CASE
      [CHAR] + OF *Sign 0! FALSE ENDOF
      [CHAR] - OF *Sign ON FALSE ENDOF
      [CHAR] , OF TRUE >IN 1-! ENDOF
        0x59 THROW
     ENDCASE
    ELSE DROP TRUE THEN
   UNTIL
   *OpRel IF *DefDatasz TO *ImSize EXIT THEN
   *OpSize CASE
    1 OF *Imm -256 AND IF 0x5A THROW THEN 1 TO *ImSize ENDOF
    2 OF
\      *Imm -65536 AND IF 0x5A THROW THEN
      *Imm -128 AND DUP 0= SWAP -128 = OR IF 1 ELSE 2 THEN TO *ImSize
     ENDOF
    4 OF
      *Imm -128 AND DUP 0= SWAP -128 = OR IF 1 ELSE 4 THEN TO *ImSize
     ENDOF
    DROP *Imm -32768 AND DUP 0= SWAP -32768 = OR
    IF 0x5C THROW THEN
     *DefDatasz TO *OpSize *DefDatasz TO *ImSize
    DUP
   ENDCASE
;
: Mult ( n1 --> n2 )
   CASE
    1 OF 0 ENDOF
    2 OF 1 ENDOF
    4 OF 2 ENDOF
    8 OF 3 ENDOF
    TRUE IF 0x58 THROW THEN
   ENDCASE
;

0 VALUE *OldReg
: ?MemReg ( --> flag )
   *Reg ?Reg IF
    *Reg TO *R/M TO *Reg 3 TO *Mod
   ELSE
    DROP ?Ptr *Sign ! *SegReg TO *OldReg IN>R
    ?SegReg IF
     TO *SegReg
     ?Colon IFNOT
      *Sign @ IF 82 THROW THEN *OldReg TO *SegReg R>IN FALSE EXIT
     THEN
     *Sign ON
    THEN
    RDROP ?Bracket[ DUP *Sign @ OR *Sign !
    *Sign @ IFNOT DROP FALSE EXIT THEN
    0= IF 0x56 THROW THEN *Sign 0! 0 TO *Offset
    BEGIN
     ?Reg32 IF
      *Sign @ REF-ERROR
      *AdSize 2 = IF 0x50 THROW THEN 4 TO *AdSize IN>R
      GetOp 1 = SWAP [CHAR] * = AND IF
       *Index 0< 0= REF-ERROR
       GetOp 2 <> REF-ERROR
       Mult TO *Scale TO *Index RDROP
       *Index 4 = REF-ERROR
      ELSE
       R>IN *Base 0< IFNOT
        *Index 0< 0= REF-ERROR
        0 TO *Scale *Base TO *Index
        *Index 4 = REF-ERROR
       THEN TO *Base
      THEN
     ELSE
      ?Reg16 IF
       *Sign @ REF-ERROR
       *AdSize 4 = IF 0x50 THROW THEN
       2 TO *AdSize
       CASE
        3 OF *Base  0< 0= REF-ERROR 3 TO *Base ENDOF
        5 OF *Base  0< 0= REF-ERROR 5 TO *Base ENDOF
        6 OF *Index 0< 0= REF-ERROR 6 TO *Index ENDOF
        7 OF *Index 0< 0= REF-ERROR 7 TO *Index ENDOF
        TRUE REF-ERROR
       ENDCASE
      ELSE
       ?Number IF
        *AdSize IFNOT *DefDatasz TO *AdSize THEN
        *Sign @ IF NEGATE *Offset + TO *Offset
        ELSE
         IN>R GetOp 1 = SWAP [CHAR] * = AND IF
          *Index 0< 0= REF-ERROR
          ?Reg32 0= REF-ERROR
          TO *Index Mult TO *Scale RDROP
          *Index 4 = REF-ERROR
         ELSE R>IN *Offset + TO *Offset THEN
        THEN
       ELSE
        ?Label IF
         *AdSize 2 = IF 0x50 THROW THEN
         *DefDatasz TO *AdSize
         *GenName C@ IF
          *OffName C@ REF-ERROR
          *GenName *OffName SCopy
          DUP HERE <> IF *Sign @ REF-ERROR THEN *GenName 0!
         THEN
         *Sign @ IF   *OfRel 1- TO *OfRel NEGATE
                 ELSE *OfRel 1+ TO *OfRel THEN *Offset + TO *Offset
        ELSE  89 THROW THEN
       THEN
      THEN
     THEN
     *OfRel 0 1 BETWEEN 0= REF-ERROR
     GetOp 1 <> REF-ERROR
     CASE
      [CHAR] + OF *Sign 0! FALSE ENDOF
      [CHAR] - OF *Sign ON FALSE ENDOF
      [CHAR] ] OF TRUE ENDOF
       89 THROW
     ENDCASE
    UNTIL
   THEN
   *OfSize IFNOT
    *Offset IF
     *Offset 127 > *Offset -128 < OR IF
      *DefDatasz TO *OfSize
      *AdSize 2 = IF
       *Offset 32767 > *Offset -32768 < OR 
       IF  91 THROW
       ELSE 2 TO *OfSize
       THEN
      THEN
     ELSE 1 TO *OfSize THEN
    THEN
   THEN
   *OfRel IF *DefDatasz TO *OfSize THEN
   *AdSize *DefDatasz = IF
    *Base 0< IF
     0 TO *Mod
     *Index 0< IF 5 TO *R/M ELSE 4 TO *R/M 5 TO *Base THEN
     *DefDatasz TO *OfSize
    ELSE
     *OfSize 4 = IF 2 ELSE *OfSize THEN TO *Mod
     *Base 5 = *Mod 0= AND IF 0 TO *Offset 1 TO *Mod 1 TO *OfSize THEN
     *Index 0< IF
      *Base 4 = IF 0 TO *Scale 4 TO *Index 4 TO *R/M THEN
      *Base TO *R/M
     ELSE 4 TO *R/M THEN
    THEN
   ELSE
    *AdSize 2 = IF
     *OfSize TO *Mod
     *Base 6 = *Index 0< *OfSize 0= AND AND IF
      1 TO *Mod 0 TO *Offset 1 TO *OfSize
     THEN
     *Base CASE
      3 OF
        *Index CASE
         6 OF 0 ENDOF
         7 OF 1 ENDOF
         DROP 7 DUP
        ENDCASE
       ENDOF
      5 OF
        *Index CASE
         6 OF 2 ENDOF
         7 OF 3 ENDOF
         DROP 6 DUP
        ENDCASE
       ENDOF
        *Index CASE
         6 OF 4 ENDOF
         7 OF 5 ENDOF
         DROP 0 TO *Mod 2 TO *OfSize 6 DUP
        ENDCASE
     ENDCASE
     TO *R/M
    THEN
   THEN TRUE
;
ALSO S_ASSEM DEFINITIONS PREVIOUS

: FCALL ( --> ) 
  0xE8 C,              \ машинная команда CALL
  ' HERE CELL+ - , ;

: FOR
 CONTEXT @ >R PREVIOUS
 INTERPRET
 ALSO  R> CONTEXT ! ;

: EQU ( --> )
   LSP @ C@ 1 =
   LSP @ 3 + @
   HERE \  *DefDatasz 2 = IF 0xFFFF AND   THEN
    = AND IF
    InitCommand  *DefDatasz TO *OpSize
    Imm *Imm LSP @ 3 + !
    *OpRel IFNOT 7 LSP @ C! THEN
   ELSE 93 THROW THEN
;

: DD ( --> )
   BEGIN
    InitCommand 4 TO *OpSize
    Imm 4 TO *ImSize
    *OpRel IF *DefDatasz 4 <> REF-ERROR
     *ImSize 4 <> IF 85 THROW THEN
     *ImmName C@ IFNOT 1 *ImmName 1+ C! THEN
     *ImmName 1+ C@ IF *ImmName HERE 2 AddStrObject
     ELSE
      *ImmName C@ 1- *ImmName 1+ C!
      *ImmName 1+ HERE 5 AddStrObject
     THEN
    THEN
    *Imm ,
    IN>R GetOp 1 = SWAP [CHAR] , = AND IF RDROP FALSE
    ELSE R>IN TRUE THEN
   UNTIL
;

: DW ( --> )
   BEGIN
    InitCommand 4 TO *OpSize
    Imm *OpRel REF-ERROR
    2 TO *OpSize 2 TO *ImSize
\    *Imm -65536 AND DUP 0= SWAP -65536 = OR 0= IF 0x5A THROW THEN
    *Imm W,
    IN>R GetOp 1 = SWAP [CHAR] , = AND IF RDROP FALSE
    ELSE R>IN TRUE THEN
   UNTIL
;

: DS ( --> )
   InitCommand 4 TO *OpSize
   Imm 4 TO *ImSize
   *OpRel IF 101 THROW THEN
   HERE *Imm DUP ALLOT ERASE
;
: DB ( --> )
   BEGIN
    InitCommand
    IN>R GetOp 1 INVERT AND 4 = IF
     RDROP COUNT ?DUP IF HERE SWAP DUP ALLOT CMOVE ELSE DROP THEN
    ELSE
        DROP R>IN 4 TO *OpSize
        Imm *OpRel REF-ERROR
        1 TO *OpSize
        *Imm -128 AND DUP 0= SWAP -128 = OR *Imm 256 U< OR 0=
        IF 0x5A THROW THEN
        *Imm C,
    THEN
    IN>R GetOp 1 = SWAP [CHAR] , = AND IF RDROP FALSE
    ELSE R>IN TRUE THEN
   UNTIL
;
HIDDEN DEFINITIONS
0 VALUE ?ENDCODE

: DEF+! ( N ADDR -- )
 *DefDatasz 4 = IF +!  ELSE DUP>R @ + R> W!  THEN ;

: DEF! ( N ADDR -- )
 *DefDatasz 4 = IF  !  ELSE              W!  THEN ;

: ENDCODE ( 0x5030F8 --> )
   TRUE TO ?ENDCODE
   PREVIOUS  0x5030F8 ?PAIRS LSP @ >R
   BEGIN
   R@ C@ WHILE
    R@ C@ 5 = IF
     NullString R@ 3 + @ 2 AddStrObject
     R@ 7 + 9 FindStrObject IF
      R@ 3 + @ DEF+! 10 R@ C!
     ELSE
      HERE R@ 3 + @ DEF+! HERE R@ 3 + !
      R@ 8 + HERE R@ 7 + C@ DUP ALLOT CMOVE 9 R@ C!
     THEN
    THEN
    R@ 1+ W@ R> + >R
   REPEAT
   RDROP  LSP @ >R
   BEGIN
   R@ C@ WHILE
    R@ C@ 8 = IF
     NullString R@ 3 + @ 2 AddStrObject
     NullString HERE 2 AddStrObject
     HERE R@ 3 + @ DEF! R@ 7 + @ ,
    THEN
    R@ 1+ W@ R> + >R
   REPEAT
   RDROP  LSP @ >R
   BEGIN
   R@ C@ WHILE
    R@ C@ 2 = IF
     R@ 7 + C@ IF
      R@ 7 + 1 FindStrObject IF
       R@ 3 + @ DEF+! 0 R@ 7 + C!
      ELSE
\       R@ 7 + >ASCIIZ 0x21 - ErrNo ! 0x53 Error
       ."  ASM: Label not found:" R@ 7 + COUNT TYPE ERR_
      THEN
     THEN
\     R@ 3 + @ LAST @ Name> - RMark
    ELSE
     R@ C@ 4 = IF
      R@ 7 + 1 FindStrObject IF
       R@ 3 + @ - 1- DUP -128 AND DUP 0= SWAP -128 = OR IF
        R@ 3 + @ C!
       ELSE  96 THROW THEN
      ELSE
\       R@ 7 + >ASCIIZ 0x21 - ErrNo ! 0x53 Error
       ."  ASM: Label not found:" R@ 7 + COUNT TYPE ERR_
      THEN
     ELSE
      R@ C@ 3 = IF
       R@ 7 + 1 FindStrObject IF
        R@ 3 + @ - *DefDatasz - R@ 3 + @ DEF!
       ELSE
\        R@ 7 + >ASCIIZ 0x21 - ErrNo ! 0x53 Error
       ."  ASM: Label not found:" R@ 7 + COUNT TYPE ERR_
       THEN
      THEN
     THEN
    THEN
    R@ 1+ W@ R> + >R
   REPEAT
   RDROP ( Save-Input) LSP @ >R
   BEGIN
   R@ C@ WHILE
    R@ C@ 6 = IF
     R@ 7 + 1 FindStrObject IF
\      R@ 7 + COUNT SetStream VALUE 5 RMark Public
     ELSE
      R@ 7 + 7 FindStrObject IF
\       R@ 7 + COUNT SetStream VALUE Public
      ELSE
\       R@ 7 + >ASCIIZ 0x21 - ErrNo ! 0x62 Error
       ." ASM: Unresolved PUBLIC reference:  R@ 7 + COUNT TYPE ERR_
      THEN
     THEN
    THEN
    R@ 1+ W@ R> + >R
   REPEAT
   RDROP ( Restore-Input DROP )
;

:  STARTCODE ( -- )
  ALSO S_ASSEM FALSE TO  ?ENDCODE
   LSP @ >R 0x5030F8 0 >L
   BEGIN
    BEGIN
    GetOp DUP WHILE
     DUP 3 = IF
      DROP IFIND IF
        EXECUTE
          ?ENDCODE IF
            R> LSP ! EXIT
          THEN
      ELSE
       *GenName SCopy GetOp 1 = SWAP [CHAR] : = AND IF
        *GenName 1 FindStrObject IF
\         *GenName >ASCIIZ 0x21 - ErrNo ! 0x5F Error
         ."  ASM: Label already defined:"  R@ 7 + COUNT TYPE ERR_
        ELSE *GenName HERE \ *DefDatasz 2 =  IF 0xFFFF AND  THEN
             1 AddStrObject THEN
       ELSE  -321 THROW THEN
      THEN
     ELSE
      1 = IF
       [CHAR] [ = IF
        [CHAR] ] WORD COUNT EVALUATE
       ELSE  -321 THROW  THEN
      ELSE  -321 THROW  THEN
     THEN
    REPEAT
    2DROP REFILL 0= ABORT" endcode not found"
   AGAIN
;

: PUBLIC ( --> )
   BEGIN
    GetOp 3 = IF 0 6 AddStrObject ELSE 0x5D THROW THEN
    IN>R GetOp 1 = SWAP [CHAR] , = AND IF RDROP FALSE
    ELSE R>IN TRUE THEN
   UNTIL
;
: 2Op ( name ( c1opc c1reg ... c9opc c9reg --> )
             ( --> )
  \ One-byte opcodes only, but with possible "reg" modIFier
   CREATE 9 0 DO SWAP C, C, LOOP
DOES>
   TO *OpArray InitCommand 1 TO *OpcSize
   ?Reg IF
    Comma ?MemReg IF
     *OpSize  CASE
      1 OF *OpArray 2+ ENDOF
      2 OF *OpArray ENDOF
      4 OF *OpArray ENDOF
      0x5C THROW
     ENDCASE
    ELSE
     3 TO *Mod *Reg TO *R/M
     Imm  *OpSize
       CASE
      1 OF
        *OpArray
        *Reg IF 12 ELSE -1 TO *Mod -1 TO *Scale 0 TO *OfSize 16 THEN +
        DUP 1+ C@ TO *Reg
       ENDOF
      2 OF
        *OpArray
        *Reg IF *ImSize 1 = IF 8 ELSE 10 THEN
        ELSE -1 TO *Mod -1 TO *Scale 0 TO *OfSize *OpSize TO *ImSize 14 THEN +
        DUP 1+ C@ TO *Reg
       ENDOF

      4 OF
        *OpArray
        *Reg *ImSize 1 = OR IF *ImSize 1 = IF 8 ELSE 10 THEN
        ELSE -1 TO *Mod -1 TO *Scale 0 TO *OfSize *OpSize TO *ImSize 14 THEN +
        DUP 1+ C@ TO *Reg
       ENDOF
       0x5C THROW
     ENDCASE
    THEN
   ELSE
    ?MemReg 0= IF 94 THROW THEN
    Comma ?Reg IF
     *OpSize CASE
      1 OF *OpArray 6 + ENDOF
      2 OF *OpArray CELL+ ENDOF
      4 OF *OpArray CELL+ ENDOF
      0x5C THROW
     ENDCASE
    ELSE
     Imm *OpSize CASE
      1 OF *OpArray 12 + DUP 1+ C@ TO *Reg ENDOF
      2 OF
        *OpArray *ImSize 1 = IF 8 ELSE 10 THEN + DUP 1+ C@ TO *Reg
       ENDOF
      4 OF
        *OpArray *ImSize 1 = IF 8 ELSE 10 THEN + DUP 1+ C@ TO *Reg
       ENDOF
       0x5C THROW
     ENDCASE
    THEN
   THEN
   C@ DUP TO *OpCode 0xF7 = IF *OpSize TO *ImSize THEN
   CompileCommand
;
: PUSH ( --> )
   InitCommand
   ?MemReg IF
    1 TO *OpcSize
    *OpSize 2 < IF 0x50 THROW THEN
    *Mod 3 = IF -1 TO *Mod *R/M 0x50 + TO *OpCode
    ELSE 0xFF TO *OpCode 6 TO *Reg THEN
   ELSE
    ?SegReg IF
     CASE
      0 OF 1 0x06 ENDOF
      1 OF 1 0x0E ENDOF
      2 OF 1 0x16 ENDOF
      3 OF 1 0x1E ENDOF
      4 OF 2 0xA00F ENDOF
      5 OF 2 0xA80F ENDOF
      DUP
     ENDCASE
     TO *OpCode TO *OpcSize
    ELSE
     *DefDatasz TO *OpSize Imm 1 TO *OpcSize
     *ImSize 4 = *Imm -32768 AND DUP 0= SWAP -32768 = OR AND *OpRel 0= AND
     IF 2 TO *ImSize THEN
     *ImSize TO *OpSize
     *ImSize 1 = IF 0x6A ELSE 0x68 THEN TO *OpCode
    THEN
   THEN
   CompileCommand
;

: POP ( --> )
   InitCommand
   ?MemReg IF
    1 TO *OpcSize
    *OpSize 2 <  IF 0x50 THROW THEN
    *Mod  3 = IF -1 TO *Mod *R/M  0x58 + TO *OpCode
    ELSE 0x8F TO *OpCode 0 TO *Reg THEN
   ELSE
    ?SegReg IF
     CASE
      0 OF 1 0x07 ENDOF
      1 OF TRUE 1 #OPER-ERROR ENDOF
      2 OF 1 0x17 ENDOF
      3 OF 1 0x1F ENDOF
      4 OF 2 0xA10F ENDOF
      5 OF 2 0xA90F ENDOF
      DUP
     ENDCASE
     TO *OpCode TO *OpcSize
    ELSE  92 THROW THEN
   THEN  
   CompileCommand
;

: IncDec ( name ( byte_opc b_reg word_opc w_reg reg_baseopc dummy --> )
                ( --> )
  \ One-byte opcodes only, but with possible "reg" modIFier
   CREATE 3 0 DO SWAP C, C, LOOP
DOES>
   TO *OpArray InitCommand
   ?MemReg IF
    *OpSize 0= IF 92 THROW THEN
    1 TO *OpcSize
    *Mod 3 = *OpSize 1 > AND IF -1 TO *Mod *OpArray C@ *R/M +
    ELSE *OpSize 1 = IF 4 ELSE 2 THEN *OpArray + DUP 1+ C@ TO *Reg C@ THEN
    TO *OpCode
   ELSE TRUE 2 #OPER-ERROR THEN
   CompileCommand
;
: NegNot ( name ( byte_opc b_reg word_opc w_reg --> )
                ( --> )
  \ One-byte opcodes only, but with possible "reg" modIFier
   CREATE 2 0 DO SWAP C, C, LOOP
DOES>
   TO *OpArray InitCommand
   ?MemReg IF
    *OpSize 0= IF 92 THROW THEN
    1 TO *OpcSize
    *OpArray *OpSize 1 = IF 2 + THEN DUP 1+ C@ TO *Reg C@ TO *OpCode
   ELSE TRUE 3 #OPER-ERROR THEN
   CompileCommand
;
: MOV ( --> )  
   InitCommand 1 TO *OpcSize
   ?MemReg IF
    Comma *Mod 3 = IF
     *R/M TO *Reg -1 TO *Mod
     ?MemReg IF
      *Reg 0= *Mod 0= *R/M 5 = AND AND IF
       -1 TO *Mod *OpSize 1 = IF 0xA0 ELSE 0xA1 THEN
      ELSE
       *OpSize 1 = IF 0x8A ELSE 0x8B THEN
      THEN
     ELSE
      ?SegReg IF
       *OpSize 2 < IF 0x50 THROW THEN
       *Reg TO *R/M 3 TO *Mod TO *Reg 0x8C
      ELSE
       Imm *OpSize TO *ImSize
       *Reg *OpSize 1 = IF 0xB0 ELSE 0xB8 THEN +
      THEN
     THEN
    ELSE
     ?Reg  IF
      *Reg  0= *Mod 0= *R/M 5 =  AND AND  IF
       -1 TO *Mod *OpSize 1 = IF 0xA2 ELSE 0xA3 THEN
      ELSE
       *OpSize 1 = IF 0x88 ELSE 0x89 THEN
      THEN
     ELSE
      Imm *OpSize TO *ImSize 0 TO *Reg
      *OpSize 1 = IF 0xC6 ELSE 0xC7 THEN
     THEN
    THEN
   ELSE
    ?SegReg IF
     Comma TO *Reg
     ?MemReg IF *DefDatasz TO *OpSize 0x8E ELSE TRUE 4 #OPER-ERROR THEN
    ELSE TRUE 5 #OPER-ERROR THEN
   THEN
   TO *OpCode CompileCommand
;
: INT ( --> )
   InitCommand 1 TO *OpSize
   Imm *Imm 3 = IF 0xCC C, ELSE 0xCD C, *Imm C, THEN
;
: OUT ( --> )
   InitCommand
   ?Reg IF
    *Reg 2 = *OpSize 2 = AND IF
     Comma InitCommand
     ?Reg IF
      *Reg IF TRUE 6 #OPER-ERROR THEN
      *OpSize CASE
       1 OF 0xEE C, ENDOF
       2 OF 0xEF66 W, ENDOF
       4 OF 0xEF C, ENDOF
       DUP
      ENDCASE
     ELSE TRUE 7 #OPER-ERROR THEN
    ELSE TRUE 8 #OPER-ERROR THEN
   ELSE
    1 TO *OpSize Imm Comma *Imm >R InitCommand
    ?Reg IF
     *Reg IF TRUE 9 #OPER-ERROR THEN
      *OpSize CASE
       1 OF 0xE6 C, ENDOF
       2 OF 0xE766 W, ENDOF
       4 OF 0xE7 C, ENDOF
       DUP
      ENDCASE
    ELSE TRUE 10 #OPER-ERROR THEN
    R> C,
   THEN
;
: IN ( --> )
   InitCommand
   ?Reg IF *Reg  11 #OPER-ERROR ELSE TRUE 12 #OPER-ERROR THEN
   Comma *OpSize >R InitCommand
   ?Reg IF
    *Reg 2 = *OpSize 2 = AND 0= 13 #OPER-ERROR
    R> CASE
     1 OF 0xEC C, ENDOF
     2 OF 0xED66 W, ENDOF
     4 OF 0xED C, ENDOF
     DUP
    ENDCASE
   ELSE
    1 TO *OpSize Imm
    R> CASE
     1 OF 0xE4 C, ENDOF
     2 OF 0xE566 W, ENDOF
     4 OF 0xE5 C, ENDOF
     DUP
    ENDCASE
    *Imm C,
   THEN
;
: LxS ( name ( opcode opcsize --> )
             ( --> )
   CREATE C, W,
DOES>
   InitCommand
   DUP C@ TO *OpcSize 1+ W@ TO *OpCode
   ?Reg IF
    *OpSize TO *OpArray
    Comma *OpSize 2 < IF 0x50 THROW THEN
    *OpSize 2 = *OpCode 0x8D = OR IF  \ Opcode 8D belongs TO LEA
     *OpCode 0x8D = IF 0 ELSE 4 THEN TO *OpSize
     ?MemReg IF
      *Mod 3 = 14 #OPER-ERROR
      *OpCode 0x8D = IF *OpArray ELSE 2 THEN TO *OpSize
     ELSE TRUE 15 #OPER-ERROR THEN
    ELSE
     ?Ptr IF 0x50 THROW THEN
     ?FWord DROP *DefDatasz TO *OpSize
     ?MemReg IF *Mod 3 = 16 #OPER-ERROR
     ELSE TRUE 17 #OPER-ERROR THEN
    THEN
   ELSE TRUE 18 #OPER-ERROR THEN
   CompileCommand
;
: JShort ( --> )
   *OpArray C@ 0= 19 #OPER-ERROR
   *DefDatasz TO *OpSize
   Imm *ImmName C@ IF
    *OpArray 1+ W@ HERE ! *OpArray C@ ALLOT
    *ImmName HERE 4 AddStrObject
    0 C,
   ELSE
    *Imm HERE 2+ - -128 AND DUP 0= SWAP -128 = OR IF
     *Imm HERE 2+ - *OpArray 1+ W@ HERE ! *OpArray C@ ALLOT C,
    ELSE 96 THROW THEN
   THEN
;
: Jxx ( name ( indir_opcsize i_opc i_reg near_sze n_opc short_sz s_opc --> )
             ( --> )
   CREATE SWAP C, W, SWAP C, W, ROT C, SWAP W, C,
DOES>
   TO *OpArray
   InitCommand
   IN>R GetOp 3 = IF C" SHORT" S= IF RDROP JShort EXIT THEN THEN DROP R>IN
   0 TO *AdSize
   ?MemReg IF
    *OpArray 6 + C@ ?DUP IF TO *OpcSize ELSE 20 #OPER-ERROR THEN
    *OpArray 7 + W@ TO *OpCode *OpArray 9 + C@ TO *Reg
    CompileCommand EXIT
   THEN
   *DefDatasz TO *OpSize
   Imm  *OpSize>DEF *OpArray C@ *OpArray 3 + C@ OR 0=  21 #OPER-ERROR
   *ImmName C@ IF
    *OpArray 3 + C@ IF
     *OpArray CELL+ W@ HERE ! *OpArray 3 + C@ ALLOT
     *ImmName HERE 3 AddStrObject 0 *DefDatasz 4 = IF , ELSE W, THEN
    ELSE
     *OpArray 1+ W@ HERE ! *OpArray C@ ALLOT
     *ImmName HERE 4 AddStrObject 0 C,
    THEN
   ELSE
\    *Imm 0x100 KernelSize + LAST @ Name> BETWEEN IF
\     0 TO *OpRel *OffName 0! *ImmName 0! 0 TO *ImSize 0 TO *Offset 4 TO *OfSize
\     4 TO *AdSize -1 TO *Base -1 TO *Index -1 TO *Scale 5 TO *R/M 0 TO *Mod
\     *OpArray 6 + C@ ?DUP 0= ABORT" ASM: External address reference not allowed"
\     TO *OpcSize 
\     *OpArray 7 + W@ TO *OpCode *OpArray 9 + C@ TO *Reg
\     *Imm HERE *OpcSize + 1+ 8 AddNumObject
\     CompileCommand EXIT
\    THEN
    *OpArray C@ *Imm HERE 2+ - -128 AND DUP 0= SWAP -128 = OR AND IF
     *Imm HERE 1+ *OpArray C@ + - *OpArray 1+ W@ HERE ! *OpArray C@ ALLOT C,
    ELSE
     *OpArray 3 + C@ IF
      *Imm HERE *DefDatasz 4 = IF CELL+ THEN *OpArray 3 + C@ + -
      *OpArray CELL+ W@ HERE ! *OpArray 3 + C@ ALLOT 
      *DefDatasz 4 = IF , ELSE W, THEN
     ELSE 96 THROW THEN
    THEN
   THEN
;
: ShIFt ( name ( reg --> )
               ( --> )
  \ Hardcoded opcodes - "reg" modIFiers only
   CREATE C,
DOES>
   InitCommand
   C@ TO *Reg
   ?MemReg IF
    Comma ?Reg8 IF
     1 <> 22 #OPER-ERROR
     *OpSize 1 = IF 0xD2 ELSE 0xD3 THEN
    ELSE
     Imm *Imm 0xFF U> IF 90 THROW THEN
     *Imm 1 = IF
      *OpSize 1 = IF 0xD0 ELSE 0xD1 THEN 0 TO *ImSize
     ELSE
      *OpSize 1 = IF 0xC0 ELSE 0xC1 THEN 1 TO *ImSize
     THEN
    THEN
   ELSE TRUE 23 #OPER-ERROR THEN
   TO *OpCode 1 TO *OpcSize
   CompileCommand
;
: XCHG ( --> )
   InitCommand
   ?MemReg IF
    Comma *Mod 3 = IF
     -1 TO *Mod *R/M TO *Reg
     ?MemReg IF
      *Mod 3 = *R/M 0= *Reg 0= OR AND *OpSize 1 > AND IF
       *OpSize 2 = IF 0x66 C, THEN
       0x90 *Reg + *R/M + C, EXIT
      THEN
     ELSE TRUE 24 #OPER-ERROR THEN
    ELSE
     ?Reg 0=  25 #OPER-ERROR 
    THEN
   ELSE 25 #OPER-ERROR THEN
   *OpSize 1 = IF 0x86 ELSE 0x87 THEN TO *OpCode 1 TO *OpcSize
   CompileCommand
;
: IMUL ( --> )
   InitCommand
   ?MemReg IF
    IN>R GetOp 1 = SWAP [CHAR] , = AND IF
     RDROP *Mod 3 <> 26 #OPER-ERROR
     *OpSize 1 > 0= 27 #OPER-ERROR
     *R/M TO *Reg -1 TO *Mod
     ?MemReg IF
      IN>R GetOp 1 = SWAP [CHAR] , = AND IF
       RDROP Imm 1 TO *OpcSize
       *ImSize 1 = IF 0x6B ELSE 0x69 THEN TO *OpCode
      ELSE
       R>IN 0xAF0F TO *OpCode 2 TO *OpcSize
      THEN
     ELSE
      Imm 3 TO *Mod *Reg TO *R/M 1 TO *OpcSize
      *ImSize 1 = IF 0x6B ELSE 0x69 THEN TO *OpCode
     THEN
    ELSE
     R>IN *OpSize 1 = IF 0xF6 ELSE 0xF7 THEN TO *OpCode 1 TO *OpcSize 5 TO *Reg
    THEN
   ELSE TRUE 28 #OPER-ERROR THEN
   CompileCommand
;
: MOVxx ( name ( opc1 opc2 --> )
               ( --> )
  \ Two-byte opcodes only
   CREATE W, W,
DOES>
   InitCommand
   TO *OpArray 2 TO *OpcSize
   ?Reg IF
    *OpSize >R 0 TO *OpSize
    Comma ?MemReg IF
     *OpSize 4 = IF 0x50 THROW THEN
     *OpSize 2 = R> TO *OpSize IF
      *OpSize 4 <> IF 0x50 THROW THEN
      *OpArray W@ TO *OpCode
     ELSE *OpArray 2+ W@ TO *OpCode THEN
    ELSE TRUE 29 #OPER-ERROR THEN
   ELSE TRUE 29 #OPER-ERROR THEN
   CompileCommand
;
: Bit ( name ( c1opc c1reg c2opc c2reg --> )
             ( --> )
  \ Two-byte opcodes only, with possible "reg" modIFier
   CREATE SWAP W, C, SWAP W, C,
DOES>
   InitCommand
   TO *OpArray 2 TO *OpcSize
   ?MemReg IF
    *OpSize 2 < 30 #OPER-ERROR
    Comma ?Reg IF *OpArray 3 +
    ELSE
     Imm *Imm 0xFF > IF 90 THROW THEN
     1 TO *ImSize *OpArray 2+ C@ TO *Reg *OpArray
    THEN
    W@ TO *OpCode
   ELSE TRUE 31 #OPER-ERROR THEN
   CompileCommand
;
: SETxx ( name ( opc --> )
               ( --> )
  \ Two-byte opcodes only
   CREATE W,
DOES>
   InitCommand
   W@ TO *OpCode 2 TO *OpcSize
   ?MemReg IF
    *OpSize 1 <>  33 #OPER-ERROR
    0 TO *Reg
   ELSE TRUE 34 #OPER-ERROR THEN
   CompileCommand
;
: SHxD ( name ( opc1 opc2 --> )
              ( --> )
  \ Two-byte opcodes only
   CREATE W, W,
DOES>
   InitCommand
   TO *OpArray 2 TO *OpcSize
   ?MemReg IF
    *OpSize 2 < 0= 35 #OPER-ERROR
    Comma ?Reg IF
     Comma ?Reg8 IF 1 <> 36 #OPER-ERROR *OpArray
     ELSE Imm *Imm 0xFF > IF 90 THROW THEN *OpArray 2+ THEN
    ELSE TRUE 37 #OPER-ERROR THEN
   ELSE TRUE 38 #OPER-ERROR THEN
   W@ TO *OpCode
   CompileCommand
;
: BSx ( name ( WORD --> )
             ( --> )
   CREATE W,
DOES>
   InitCommand W@ TO *OpCode 2 TO *OpcSize
   ?Reg IF
    *OpSize 2 <  39 #OPER-ERROR
    Comma ?MemReg 0= TRUE 40 #OPER-ERROR
   ELSE TRUE 41 #OPER-ERROR THEN
   CompileCommand
;
: Rxx ( name ( opc --> )
             ( --> )
   CREATE C, DOES> C@ C, 1 C, 0xC3 C,
;
: ALIGN ( --> )
   InitCommand *DefDatasz TO *OpSize Imm 4 TO *ImSize
   *OpRel IF 101 THROW THEN
   BEGIN
    HERE *Imm MOD WHILE
    0 C,
   REPEAT
;

: I'  BL WORD  IFIND 0= IF -321 THROW THEN (  -? ) ;

ALSO S_ASSEM DEFINITIONS

S" ~mak\asm\ASM_SIF.F" INCLUDED

0xC9 1Op1 LEAVE
0xCC 1Op1 INT3
0xCE 1Op1 INTO
0x37 1Op1 AAA
0x3F 1Op1 AAS
0x99 1Op1D CDQ
0x98 1Op1D CWDE
0xF8 1Op1 CLC
0xFC 1Op1 CLD
0xFA 1Op1 CLI
0xF5 1Op1 CMC
0xA6 1Op1 CMPSB
0xA7 1Op1D CMPSD
0x27 1Op1 DAA
0x2F 1Op1 DAS
0xF4 1Op1 HLT
0x6C 1Op1 INSB
0x6D 1Op1D INSD
0xCF 1Op1D IRETD
0x9F 1Op1 LAHF
0xAC 1Op1 LODSB
0xAD 1Op1D LODSD
0xA4 1Op1 MOVSB
0xA5 1Op1D MOVSD
0x90 1Op1 NOP
0x6E 1Op1 OUTSB
0x6F 1Op1D OUTSD
0x61 1Op1D POPAD
0x60 1Op1D PUSHAD
0x9D 1Op1D POPFD
0x9C 1Op1D PUSHFD
0xC3 1Op1 RET
0xCB 1Op1 RETF
0x9E 1Op1 SAHF
0xAE 1Op1 SCASB
0xAF 1Op1D SCASD
0xF9 1Op1 STC
0xFD 1Op1 STD
0xFB 1Op1 STI
0xAA 1Op1 STOSB
0xAB 1Op1D STOSD
0x9B 1Op1 WAIT
0xD7 1Op1 XLAT
0xD7 1Op1 XLATB
0xF0 1Op1 LOCK
0xF3 1Op1 REP
0xF3 1Op1 REPE
0xF3 1Op1 REPZ
0xF2 1Op1 REPNE
0xF2 1Op1 REPNZ
0x0AD5 1Op2 AAD
0x0AD4 1Op2 AAM
0x310F 1Op2 RDTSC
0x98 1Op1W CBW
0x99 1Op1W CWD
0xA7 1Op1W CMPSW
0x6D 1Op1W INSW
0xCF 1Op1W IRET
0xAD 1Op1W LODSW
0xA5 1Op1W MOVSW
0x6F 1Op1W OUTSW
0x61 1Op1W POPA
0x60 1Op1W PUSHA
0x9D 1Op1W POPF
0x9C 1Op1W PUSHF
0xAF 1Op1W SCASW
0xAB 1Op1W STOSW
0x14 0 0x15 0 0x80 2 0x81 2 0x83 2 0x10 0 0x11 0 0x12 0 0x13 0 2Op ADC
0x04 0 0x05 0 0x80 0 0x81 0 0x83 0 0x00 0 0x01 0 0x02 0 0x03 0 2Op ADD
0x24 0 0x25 0 0x80 4 0x81 4 0x83 4 0x20 0 0x21 0 0x22 0 0x23 0 2Op AND
0x3C 0 0x3D 0 0x80 7 0x81 7 0x83 7 0x38 0 0x39 0 0x3A 0 0x3B 0 2Op CMP
0x1C 0 0x1D 0 0x80 3 0x81 3 0x83 3 0x18 0 0x19 0 0x1A 0 0x1B 0 2Op SBB
0x2C 0 0x2D 0 0x80 5 0x81 5 0x83 5 0x28 0 0x29 0 0x2A 0 0x2B 0 2Op SUB
0x34 0 0x35 0 0x80 6 0x81 6 0x83 6 0x30 0 0x31 0 0x32 0 0x33 0 2Op XOR
0x0C 0 0x0D 0 0x80 1 0x81 1 0x83 1 0x08 0 0x09 0 0x0A 0 0x0B 0 2Op OR
0xA8 0 0xA9 0 0xF6 0 0xF7 0 0xF7 0 0x84 0 0x85 0 0x84 0 0x85 0 2Op TEST
0xFE 0 0xFF 0 0x40 0 IncDec INC
0xFE 1 0xFF 1 0x48 0 IncDec DEC
0xF6 3 0xF7 3 NegNot NEG
0xF6 2 0xF7 2 NegNot NOT
0xF6 4 0xF7 4 NegNot MUL
0xF6 6 0xF7 6 NegNot DIV
0xF6 7 0xF7 7 NegNot IDIV
0x8D 1 LxS LEA
0xC5 1 LxS LDS
0xC4 1 LxS LES
0xB20F 2 LxS LSS
0xB40F 2 LxS LFS
0xB50F 2 LxS LGS
1 0xFF 4 1 0xE9 1 0xEB Jxx JMP
1 0xFF 2 1 0xE8 0 0 Jxx CALL
0 0 0 0 0 1 0xE3 Jxx JECXZ
0 0 0 0 0 2 0xE367 Jxx JCXZ
0 0 0 0 0 1 0xE2 Jxx LOOP
0 0 0 0 0 1 0xE1 Jxx LOOPZ
0 0 0 0 0 1 0xE0 Jxx LOOPNZ
0 0 0 2 0x840F 1 0x74 Jxx JZ
0 0 0 2 0x800F 1 0x70 Jxx JO
0 0 0 2 0x810F 1 0x71 Jxx JNO
0 0 0 2 0x820F 1 0x72 Jxx JC
0 0 0 2 0x830F 1 0x73 Jxx JNC
0 0 0 2 0x850F 1 0x75 Jxx JNZ
0 0 0 2 0x860F 1 0x76 Jxx JNA
0 0 0 2 0x870F 1 0x77 Jxx JA
0 0 0 2 0x880F 1 0x78 Jxx JS
0 0 0 2 0x890F 1 0x79 Jxx JNS
0 0 0 2 0x8A0F 1 0x7A Jxx JPE
0 0 0 2 0x8B0F 1 0x7B Jxx JPO
0 0 0 2 0x8C0F 1 0x7C Jxx JL
0 0 0 2 0x8D0F 1 0x7D Jxx JGE
0 0 0 2 0x8E0F 1 0x7E Jxx JLE
0 0 0 2 0x8F0F 1 0x7F Jxx JG
: JE JZ ;
: JB JC ;
: JNAE JC ;
: JAE JNC ;
: JNB JNC ;
: JNE JNZ ;
: JBE JNA ;
: JNBE JA ;
: JP JPE ;
: JNP JPO ;
: JNGE JL ;
: JNL JGE ;
: JNG JLE ;
: JNLE JG ;
: LOOPE LOOPZ ;
: LOOPNE LOOPNZ ;
0x02 ShIFt RCL
0x03 ShIFt RCR
0x00 ShIFt ROL
0x01 ShIFt ROR
0x04 ShIFt SAL
0x07 ShIFt SAR
0x04 ShIFt SHL
0x05 ShIFt SHR
0xBE0F 0xBF0F MOVxx MOVSX
0xB60F 0xB70F MOVxx MOVZX
0 0xA30F 4 0xBA0F Bit BT
0 0xBB0F 7 0xBA0F Bit BTC
0 0xB30F 6 0xBA0F Bit BTR
0 0xAB0F 5 0xBA0F Bit BTS
0x970F SETxx SETA
0x930F SETxx SETAE
0x920F SETxx SETC
0x960F SETxx SETNA
0x940F SETxx SETZ
0x9F0F SETxx SETG
0x9D0F SETxx SETGE
0x9C0F SETxx SETL
0x9E0F SETxx SETLE
0x950F SETxx SETNZ
0x900F SETxx SETO
0x910F SETxx SETNO
0x980F SETxx SETS
0x990F SETxx SETNS
0x9A0F SETxx SETP
0x9B0F SETxx SETNP
: SETNBE SETA ;
: SETNB SETAE ;
: SETNC SETAE ;
: SETB SETC ;
: SETNAE SETC ;
: SETBE SETNA ;
: SETE SETZ ;
: SETNLE SETG ;
: SETNL SETGE ;
: SETNGE SETL ;
: SETNG SETLE ;
: SETNE SETNZ ;
: SETPE SETP ;
: SETPO SETNP ;
0xA40F 0xA50F SHxD SHLD
0xAC0F 0xAD0F SHxD SHRD
0xBC0F BSx BSF
0xBD0F BSx BSR
0x73 Rxx RC
0x72 Rxx RNC
0x75 Rxx RZ
0x74 Rxx RNZ
0x76 Rxx RA
0x77 Rxx RNA
0x7A Rxx RPO
0x7B Rxx RPE
0x71 Rxx RO
0x70 Rxx RNO
0x7E Rxx RG
0x7F Rxx RNG
0x7D Rxx RL
0x7C Rxx RNL
: RE RZ ;
: RB RC ;
: RNAE RC ;
: RAE RNC ;
: RNB RNC ;
: RNE RNZ ;
: RBE RNA ;
: RNBE RA ;
: RP RPE ;
: RNP RPO ;
: RNGE RL ;
: RGE RNL ;
: RLE RNG ;
: RNLE RG ;

PREVIOUS

ALSO FORTH DEFINITIONS

:  Code ( -- )
   HEADER  STARTCODE ;

: STARTCODE STARTCODE ;

: USE16 ( --> )
   2 TO *DefDatasz
;
: USE32 ( --> )
   4 TO *DefDatasz
;

S_ASSEM DEFINITIONS

: MOVSD 0xA5  C,  *DefDatasz 2 = IF 0x66 C, THEN ;
: MOVSW 0xA5  C,  *DefDatasz 4 = IF 0x66 C, THEN ;

: SEG SEG ;
: PUBLIC PUBLIC ;
: ENDCODE ENDCODE ;
: PUSH PUSH ;
: POP POP ;
: MOV MOV ;
: INT INT ;
: OUT OUT ;
: IN IN ;
: XCHG XCHG ;
: IMUL IMUL ;
: ALIGN ALIGN ;

PREVIOUS
PREVIOUS DEFINITIONS

\EOF
USE16
Code ZZ
 mov      bp, msg
msg: ; EQU 4444H
 RET
ENDCODE
\EOF

C" DBG" FIND NIP
[IF]

ALSO S_ASSEM
 mov      Ebp, msg

USE16
DBG mov      bp, msg
[THEN]
\S

Code ZZ
     ADD EBX, {KEY}
     ADD EBX, 44H
     ADD EBX, 4444444H
WWW: MOV EAX, WWW+4
     MOV EAX, [EBP+EDX*4]
FOR   0x44444 ,
     JMP SHORT SS1
SS1: JMP       SS2
SS2:
EndCode

' ZZ 20 DUMP
\ 4B22AE 09C0             OR      EAX , EAX
\ http://win32asm.chat.ru/