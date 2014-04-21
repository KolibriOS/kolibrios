
~mak\utils.f 

CREATE  GetOp_STR  80 ALLOT
C" SkipDelimiters" FIND NIP 0=
[IF]
: 2+ 2 + ;
: 0! OFF ;
: 1+! incr ;
: 1-! DECR ;
: EndOfChunk ( -- flag )
  >IN @ SOURCE NIP < 0=        \ >IN не меньше, чем длина чанка
;

: CharAddr ( -- c-addr )
  SOURCE DROP >IN @ +
;

: PeekChar ( -- char )
  CharAddr C@       \ символ из текущего значения >IN
;

: GetChar ( -- char flag )
  EndOfChunk
  IF 0 FALSE
  ELSE PeekChar TRUE THEN
;

: IsDelimiter ( char -- flag )
  BL 1+ < ;

: OnDelimiter ( -- flag )
  GetChar SWAP IsDelimiter AND
;

: SkipDelimiters ( -- ) \ пропустить пробельные символы
  BEGIN
    OnDelimiter
  WHILE
    >IN 1+!
  REPEAT
;

: RDROP POSTPONE R>DROP ; IMMEDIATE

[THEN]

: OnNotDelimiter_ ( C -- flag )
  DUP [CHAR] 0 U<  IF DROP FALSE EXIT THEN
  DUP [CHAR] : U<  IF DROP TRUE  EXIT THEN
  DUP [CHAR] @ U<  IF DROP FALSE EXIT THEN
  DUP [CHAR] [ U<  IF DROP TRUE  EXIT THEN
  DUP [CHAR] _  =  IF DROP TRUE  EXIT THEN
  DUP [CHAR] a U<  IF DROP FALSE EXIT THEN
  DUP [CHAR] { U<  IF DROP TRUE  EXIT THEN
                      DROP FALSE
;

: SkipWord_ ( -- ) \ пропустить term символы
  BEGIN
  GetChar  IF  OnNotDelimiter_  THEN
  WHILE
    >IN 1+!
  REPEAT ;

: ParseWord_ ( -- c-addr u )
  CharAddr >IN @
  SkipWord_
  >IN @ - NEGATE ;

C" UPPER" FIND NIP 0=
[IF]

BASE @ HEX
: UPC  ( c -- c' )
   DUP [CHAR] Z U>
   IF  DF AND
   THEN   ;

BASE !

: UPPER ( ADDR LEN -- )
  0 ?DO COUNT UPC OVER 1- C! LOOP DROP ;

[THEN]

: IN>R  POSTPONE >IN
        POSTPONE @
        POSTPONE >R ; IMMEDIATE

: R>IN  POSTPONE R>
        POSTPONE >IN
        POSTPONE !   ; IMMEDIATE

: GetOp_BS ParseWord_ GetOp_STR PLACE  GetOp_STR ;

: non-term 1 GetOp_STR C! PeekChar GetOp_STR 1+ C! >IN 1+! GetOp_STR  ;

: TERM-STR  CharAddr SkipWord_ CharAddr  OVER -
            GetOp_STR PLACE  GetOp_STR DUP COUNT UPPER  ;

\ types:        1 - non-term (comments, etc.)
\               2 - number
\               3 - name
\               4 - "-bracketed string
\               5 - '-bracketed string

CREATE XXX 0 ,
: (GetOp) ( --> string type )
        SkipDelimiters
        GetChar 0= IF DROP XXX FALSE EXIT THEN
        DUP [CHAR] 0 <
        IF DUP [CHAR] "  =
           IF  [CHAR] " GetOp_BS 4 EXIT
           THEN
               [CHAR] '  =
           IF  [CHAR] ' GetOp_BS 5 EXIT
           THEN non-term         1 EXIT
        THEN
        DUP [CHAR] : <
        IF DROP TERM-STR 2 EXIT
        THEN
          OnNotDelimiter_ 
        IF     TERM-STR 3 EXIT
        THEN   non-term 1  ;

: IFNOT POSTPONE 0=
        POSTPONE IF ; IMMEDIATE

1000 ALLOT
HERE CONSTANT LS0 
VARIABLE LSP
LS0  LSP !

:  ADDNUMOBJECT ( name addr type --> )
 -11 LSP +!
     LSP @ C!
  11 LSP @ 1+ W!
     LSP @ 3 + !
     LSP @ 7 + ! ;


: AddStrObject ( name addr type --> )
  ROT
  DUP C@ 1+ NEGATE LSP +!
  COUNT LSP @ PLACE   \ addr type
  -7 LSP +!
     LSP @ C!
     LSP @ 7 + C@ 8 +
     LSP @ 1+ W!
     LSP @ 3 + ! ;

0 
1 FIELD  L_TYPE
2 FIELD  L_SIZE
4 FIELD  L_ADDR
0 FIELD  L_NAME
DROP

: FindStrObject ( name type --> addr true | false )
  LSP @ >R
  BEGIN R@ L_SIZE W@
  WHILE
    DUP R@ L_TYPE C@ =
    IF        OVER R@ L_SIZE W@ 7 -
        R@ L_NAME  R@ L_SIZE W@ 7 - COMPARE 0=
        IF 2DROP   R> L_ADDR @ TRUE EXIT
        THEN
    THEN   R@ L_SIZE W@ R> + >R
  REPEAT   2DROP RDROP FALSE  ;

CREATE NullString 0 ,


: ConvertString ;

: S= ( c-addr1 c-addr2 --> true | c-addr1 false )
  OVER COUNT ROT COUNT
  COMPARE 
  IF    FALSE
  ELSE  DROP TRUE
  THEN ;
: ?S= ( flag n R: >IN --> R: >IN | -->> n true )
  SWAP
  IF  2R> 2DROP TRUE EXIT
  THEN DROP
;


ALSO FORTH DEFINITIONS

: VAL ( ADDR -- UD2 FLAG )
 0 0 ROT COUNT >NUMBER NIP 0= ;

VARIABLE CUR-PAB 
HERE 0 , CUR-PAB !

: ?PABLIC ( CFA -- FLAG )
   CUR-PAB @
   BEGIN 2DUP @ U<
   WHILE @
   REPEAT CELL+ @ = ;

: PABLIC ( -- )
  HERE CUR-PAB @ , LAST @ NAME> ,  CUR-PAB ! ;

: >L
 -4 LSP  +!
    LSP @ ! ;

: L>
   LSP @ @
 4 LSP  +! ;

: ERR_  TRUE ABORT"  " ;

C" 1-!" FIND NIP 0=
[IF] 
: 1-! ( ADDR -- )
  DUP>R @ 1- R> ! ;
[THEN]

C" ON" FIND NIP 0=
[IF] 
: ON ( ADDR -- )
  TRUE SWAP ! ;
[THEN]

C" ?PAIRS" FIND NIP 0=
[IF] 
: ?PAIRS  XOR ABORT" conditionals not paired" ;
[THEN]

\ : 'Alias ' Alias ;
PREVIOUS DEFINITIONS


