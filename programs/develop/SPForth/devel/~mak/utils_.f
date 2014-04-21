CR .( UTILS_.F)
REQUIRE [IF] ~MAK\CompIF.f
\ WINAPI: GetCurrentDirectoryA         KERNEL32.DLL
\ WINAPI: MoveFileA                    KERNEL32.DLL

: DEFER VECT ;

80 CONSTANT MAXSTRING

C" PLACE" FIND NIP 0=
[IF]

255 CONSTANT MAXCOUNTED   \ maximum length of contents of a counted string


: "CLIP"        ( a1 n1 -- a1 n1' )   \ clip a string to between 0 and MAXCOUNTED
                MAXCOUNTED MIN 0 MAX ;

: PLACE         ( addr len dest -- )
                SWAP "CLIP" SWAP
                2DUP 2>R
                CHAR+ SWAP MOVE
                2R> C! ;

: +PLACE        ( addr len dest -- ) \ append string addr,len to counted
                                     \ string dest
                >R "CLIP" MAXCOUNTED  R@ C@ -  MIN R>
                                        \ clip total to MAXCOUNTED string
                2DUP 2>R

                COUNT CHARS + SWAP MOVE
                2R> +! ;

: C+PLACE       ( c1 a1 -- )    \ append char c1 to the counted string at a1
                DUP 1+! COUNT + 1- C! ;
[THEN]
: OFF     0! ;

: BLANK         ( addr len -- )     \ fill addr for len with spaces (blanks)
                BL FILL ;

: START/STOP   ( -- )
                KEY?
                IF KEY  27 = IF ABORT THEN
                THEN ;

: .S            ( -- )
     S0 @ SP@ CELL+ 2DUP =
     IF  ." EMPTY"  2DROP 
     ELSE DO I @ . START/STOP 1 CELLS +LOOP
     THEN ;

C" TUCK" FIND NIP 0=
[IF]
: TUCK       ( n1 n2 -- n2 n1 n2 ) \ copy top data stack to under second item
   SWAP OVER ;
[THEN]


128 CONSTANT SPCS-MAX  ( optimization for SPACES )

CREATE SPCS
       SPCS-MAX ALLOT
 SPCS  SPCS-MAX BLANK

: (D.)          ( d -- addr len )       TUCK DABS  <# #S ROT SIGN #> ;

C" WITHIN" FIND NIP 0=
[IF]
: WITHIN  ( n1 low high -- f1 ) \ f1=true if ((n1 >= low) & (n1 < high))
  OVER - >R - R> U< ;
[THEN]
: BETWEEN 1+ WITHIN ;

80 VALUE COLS

: H.R           ( n1 n2 -- )    \ display n1 as a hex number right
                                \ justified in a field of n2 characters
                BASE @ >R HEX >R
                0 <# #S #> R> OVER - SPACES TYPE
                R> BASE ! ;

: H.N           ( n1 n2 -- )    \ display n1 as a HEX number of n2 digits
                BASE @ >R HEX >R
                0 <# R> 0 ?DO # LOOP #> TYPE
                R> BASE ! ;
: COL ( N -- )
      DROP 9 EMIT ;

: UPC [ CHAR A CHAR a XOR INVERT ] LITERAL AND ;

: 2,  ( D -- )
  HERE 2! 2 CELLS ALLOT ;

: VOC-STATE,
   CONTEXT @ ,
   CONTEXT @ @ ,
  VOC-LIST @ VOC-LIST 2,
   CURRENT @  CURRENT 2,
      LAST @     LAST 2,
  VOC-LIST @
  BEGIN  ?DUP
  WHILE  DUP CELL+ DUP @ SWAP 2, @
  REPEAT
;
: INCLUDE BL WORD COUNT INCLUDED ;

: CELLS+ CELLS + ;

: ? @ . ;
: DEFINED       ( -- str 0 | cfa flag )
                BL WORD FIND ;
                
: [IFUNDEF] DEFINED NIP 0= POSTPONE [IF] ;
\ C" CELL-"  FIND NIP 0=
1
[IF] : CELL- 1 CELLS - ; 
[THEN]

\ C" LCOUNT" FIND NIP 0=
1
[IF] : LCOUNT   CELL+ DUP CELL- @ ; 
[THEN]
: INCR   1 SWAP +! ;
: FIELD+ -- ;
0 [IF]
: CUR_DIR PAD 256 GetCurrentDirectoryA PAD SWAP  ;
CREATE FIRST-PATH-BUF CUR_DIR NIP 1+ ALLOT
CUR_DIR  FIRST-PATH-BUF PLACE
: FIRST-PATH" FIRST-PATH-BUF COUNT ;
: RENAME-FILE ( adr1 len adr2 len -- ior )
   4DUP + DUP @ 2>R + DUP @ 2>R
   4DUP + 0! + 0!
   DROP NIP SWAP MoveFileA
   2R> SWAP !  2R> SWAP !
;

[THEN]
: FILE-APPEND   ( fileid -- ior )
     DUP >R  FILE-SIZE DROP
          R> RESIZE-FILE  ;

C" U>" FIND NIP 0=
[IF]
: U> ( U1 U2  -- FLAG )
     SWAP U< ;
[THEN]

C" FOLLOWER" FIND NIP
[IF]
: 2,  ( D -- )
  HERE 2! 2 CELLS ALLOT ;

: VOC-STATE,
   CONTEXT @ ,
   CONTEXT @ @ ,
  VOC-LIST @ VOC-LIST 2,
   CURRENT @  CURRENT 2,
      LAST @     LAST 2,
  VOC-LIST @
  BEGIN  ?DUP
  WHILE  DUP CELL+ DUP @ SWAP 2, @
  REPEAT
;

: MARKER, ( -- ADDR )
  HERE
  VOC-STATE,
  FOLLOWER @ FOLLOWER 2,
  HERE  4 CELLS + DP 2,  0. 2,
;
: MARKER! ( ADDR -- )
   DUP @ CONTEXT ! CELL+
   DUP @ CONTEXT @ ! CELL+
   BEGIN  DUP 2@  DUP
   WHILE ! 2 CELLS +
   REPEAT 2DROP DROP ;
[ELSE]
: MARKER ( "<spaces>name" -- ) \ 94 CORE EXT
\ Пропустить ведущие пробелы. Выделить name, ограниченное пробелами.
\ Создать определение с семантикой выполнения, описанной ниже.
\ name Выполнение: ( -- )
\ Восстановить распределение памяти словаря и указатели порядка поиска
\ к состоянию, которое они имели перед определением name. Убрать 
\ определение name и все последующие определения. Не требуется 
\ обязательно восстанавливать любые оставшиеся структуры, которые 
\ могут быть связаны с удаленными определениями или освобожденным 
\ пространством данных. Никакая другая контекстуальная информация, 
\ как основание системы счисления, не изменяется.
  HERE
\  [C]HERE , [E]HERE ,
  GET-CURRENT ,
  GET-ORDER DUP , 0 ?DO DUP , @ , LOOP
  CREATE ,
  DOES> @ DUP \ ONLY
\  DUP @ [C]DP ! CELL+
\  DUP @ [E]DP ! CELL+
  DUP @ SET-CURRENT CELL+
  DUP @ >R R@ CELLS 2* + 1 CELLS - R@ 0
  ?DO DUP DUP @ SWAP CELL+ @ OVER ! SWAP 2 CELLS - LOOP
  DROP R> SET-ORDER
  DP !
;

[THEN]

C" BODY>" FIND NIP 0=
[IF] : BODY> 5 - ;
[THEN]

C" >NAME" FIND NIP 0=
[IF] : >NAME  4 - DUP BEGIN 1- 2DUP COUNT + U< 0= UNTIL NIP ;
[THEN]

C" CELL/" FIND NIP 0=
  [IF] : CELL/ ( N - N1 )  2 RSHIFT ;
  [THEN]

C" IMAGE-BEGIN" FIND NIP
[IF]
: ?NAME ( ADDR - FLAG )
        DUP IMAGE-BEGIN U>
        OVER HERE       U< AND
        IF  DUP >NAME COUNT + CELL+ =
        ELSE DROP FALSE
        THEN ;
[THEN]

H-STDOUT CONSTANT FORTH-OUT

: FORTH-IO
   FORTH-OUT H-STDOUT <> 
   IF  H-STDOUT CLOSE-FILE DROP
       FORTH-OUT TO H-STDOUT
   THEN
;
: H. BASE @ HEX SWAP U. BASE ! ;
: 3DROP DROP 2DROP ;
: 4DUP 2OVER 2OVER ;
: 0.0 0 DUP ;
: IS POSTPONE TO ; IMMEDIATE

C" -ROT" FIND NIP 0=
[IF] : -ROT ROT ROT ;
[THEN]


: SCAN ( adr len char -- adr' len' )
\ Scan for char through addr for len, returning addr' and len' of char.
        >R 2DUP R> -ROT
        OVER + SWAP
        ?DO DUP I C@ =
                IF LEAVE
                ELSE >R 1 -1 D+ R>
                THEN
        LOOP DROP ;

: SSKIP ( adr len char -- adr' len' )
\ Skip char through addr for len, returning addr' and len' of char+1.
        >R 2DUP R> -ROT
        OVER + SWAP
        ?DO DUP I C@ <>
                IF LEAVE
                ELSE >R 1 -1 D+ R>
                THEN
        LOOP DROP ;

1 CELLS CONSTANT CELL

C" LSCAN" FIND NIP 0=
[IF]
: LSCAN ( adr len long -- adr' len' )
\ Scan for char through addr for len, returning addr' and len' of char.
        >R 2DUP CELLS R> -ROT   \ adr len long adr len
        OVER + SWAP       \ adr len long adr+len adr
        ?DO DUP I @ =
                IF LEAVE
                ELSE >R 1- >R CELL+ R> R>
                THEN CELL
       +LOOP DROP ;
[THEN]

C" /STRING" FIND NIP 0=
[IF] : /STRING DUP >R - SWAP R> + SWAP ;
[THEN]

: "TO-PATHEND"  ( a1 n1 --- a2 n2 )     \ return a2 and count=n1 of filename
                OVER 1+ C@ [CHAR] : =   \ second char is ':'
                OVER 2 > AND            \ and name is longer than two characters
                IF      2 /STRING       \ then remove first two characters
                THEN                    \ now scan to end of last '\' in filename
                BEGIN   2DUP [CHAR] \ SCAN ?DUP
                WHILE   2SWAP 2DROP 1 /STRING
                REPEAT  DROP ;

: ON TRUE SWAP ! ;
C" -ROT" FIND NIP 0=
[IF] : -ROT ROT ROT ;
[THEN]

C" BOUNDS" FIND NIP 0=
[IF] : BOUNDS OVER + SWAP ;
[THEN]
: >= < INVERT ;
: 4DROP 2DROP 2DROP ;

C" RECURSE" FIND NIP 0=
[IF]
: RECURSE       ( -- )          \ cause current definition to execute itself
                ?COMP  LAST @ NAME> COMPILE, ; IMMEDIATE
[THEN]
C" DUP>R" FIND NIP 0=
[IF] : DUP>R POSTPONE DUP POSTPONE >R ; IMMEDIATE
[THEN]

C" PICK" FIND NIP 0=
[IF]
: PICK ( n -- n' )
  1+ CELLS SP@ + @ ;
[THEN]

C" ROLL" FIND NIP 0=
[IF]

: ROLL          ( n1 n2 .. nk k -- n2 n3 .. nk n1 )
\  Rotate k values on the stack, bringing the deepest to the top.
\   ?DUP IF 1- SWAP >R RECURSE R> SWAP THEN ;
     DUP>R PICK SP@ DUP CELL+ R> 1+ CELLS MOVE DROP  ;
[THEN]

C" AHEAD" FIND NIP 0=
[IF]
: AHEAD POSTPONE FALSE POSTPONE IF ; IMMEDIATE
[THEN]

C" NOT" FIND NIP 0=
[IF] : NOT 0= ;
[THEN]

C" ?EXIT" FIND NIP 0=
[IF]
 : ?EXIT POSTPONE IF
         POSTPONE EXIT
         POSTPONE THEN ; IMMEDIATE
\ : ?EXIT  IF RDROP THEN ;
[THEN]

: BEEP 7 EMIT ;

16 CONSTANT #VOCS 
-1 CELLS CONSTANT -CELL
C" D2*" FIND NIP 0=
[IF] : D2* 2DUP D+ ;
[THEN]
: ,"  [CHAR] " WORD C@ 1+ ALLOT 0 C, ;
: TAB 9 EMIT ;

: (D.)          ( d -- addr len )       TUCK DABS  <# #S ROT SIGN #> ;
: D.R           ( d w -- )              >R (D.) R> OVER - SPACES TYPE ;
: U.R           ( u w -- )              0 SWAP D.R ;
: $  SOURCE TYPE CR ; IMMEDIATE
: +NULL         ( a1 -- )       \ append a NULL just beyond the counted chars
                COUNT + 0 SWAP C! ;

C" CELLS+" FIND NIP 0=
[IF]
: CELLS+  CELLS + ;
[THEN]

C" +CELLS" FIND NIP 0=
[IF]
: +CELLS  SWAP CELLS+ ;
[THEN]
C" PERFORM" FIND NIP 0=
[IF]
: PERFORM @ EXECUTE ;
[THEN]

C" UPPER" FIND NIP 0=
[IF]
: UPPER ( A L -- )
        OVER + SWAP
        ?DO I C@ DUP [CHAR] Z U>
           IF  0xDF AND
           THEN  I C!
        LOOP ;
[THEN]

C" RESET-STACKS" FIND NIP 0=
[IF]
: RESET-STACKS  S0 @ SP! ;
[THEN]
C" D-" FIND NIP 0=
[IF]
: D- ( D1 D2  -- FLAG )
      DNEGATE D+ ;
[THEN]

C" D=" FIND NIP 0=
[IF]
: D= ( D1 D2  -- FLAG )
       D- D0= ;
[THEN]

C" D<>" FIND NIP 0=
[IF]
: D<> ( D1 D2  -- FLAG )
       D= INVERT ;
[THEN]

C" <=" FIND NIP 0=
[IF]
: <= ( D1 D2  -- FLAG )
      > INVERT ;
[THEN]

C" UMAX" FIND NIP 0=
[IF]
: UMAX ( D1 D2  -- FLAG )
   2DUP U< IF NIP ELSE DROP THEN ;
[THEN]

C" D2/" FIND NIP 0=
[IF]
: D2/        ( d1 -- d2 ) \ divide the double number d1 by two
   DUP 1 AND 0x1F RSHIFT ROT 2/ OR SWAP 2/ ;
[THEN]

C" D0<" FIND NIP 0=
[IF]
: D0<        ( d1 -- f1 )
\ Signed compare d1 double number with zero.  If d1 < 0, RETNurn TRUE.
 0< NIP ;
[THEN]
C" \S" FIND NIP 0=
[IF]
: \S            \ comment to end of file
  BEGIN REFILL 0= UNTIL

\     SOURCE-ID FILE-SIZE DROP
\     SOURCE-ID REPOSITION-FILE DROP
     [COMPILE] \ ; IMMEDIATE
[THEN]

\ C" NEEDS" FIND NIP 0=
0
[IF]
: NEEDS
  BL WORD FIND NIP
  BL WORD SWAP 0=
  IF COUNT INCLUDED
  ELSE  DROP
  THEN
;
[THEN]
C" 0MIN" FIND NIP 0=
[IF] : 0MIN 0 MIN ;
[THEN]
C" 0MAX" FIND NIP 0=
[IF] : 0MAX 0 MIN ;
[THEN]

C" H." FIND NIP 0=
[IF] : H. BASE @ SWAP HEX U. BASE ! ;
[THEN]

C" .HS" FIND NIP 0=
[IF]
: .HS ( N -- N1 )
  BASE @ >R HEX .S R> BASE ! ;
[THEN]


C" MS" FIND NIP 0=
[IF]
C" PAUSE" FIND NIP
  [IF] : MS ( N -- ) PAUSE ;
  [THEN]
[THEN]

C" 0>" FIND NIP 0=
[IF]
: 0> ( N -- ) NEGATE 0< ;
[THEN]
C" CS-DUP" FIND NIP 0=
[IF] : CS-DUP 2DUP ;
[THEN]
C" M_WL" FIND NIP 0=
[IF] : M_WL  CS-DUP POSTPONE WHILE ; IMMEDIATE
[THEN]

C" AHEAD" FIND NIP 0=
[IF] : AHEAD  ?COMP HERE BRANCH, >MARK 1 ; IMMEDIATE
[THEN]

C" CS-DUP" FIND NIP 0=
[IF] : CS-DUP 2DUP ;
[THEN]

C" CS-!" FIND NIP 0=
[IF] : CS-! 2! ;
[THEN]

C" CS-@" FIND NIP 0=
[IF] : CS-@ 2@ ;
[THEN]

C" CS-CELLS" FIND NIP 0=
[IF] : CS-CELLS CELLS 2* ;
[THEN]
