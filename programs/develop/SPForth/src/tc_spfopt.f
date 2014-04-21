 REQUIRE INCLUDED_L ~mak/listing3.f 

0 VALUE TOUSER-VALUE-CODE
0 VALUE ---CODE

0 VALUE DO-OFF
0 VALUE ?DO-OFF

0 VALUE OFF-LOOP
0 VALUE OFF-+LOOP

0 VALUE 'DUP_V
0 VALUE 'DROP_V

' DUP  TO 'DUP_V
' DROP TO 'DROP_V

:  'DUP  'DUP_V ;
: 'DROP 'DROP_V ;

: M\  POSTPONE \  ; IMMEDIATE
: OS\ ( POSTPONE \) ; IMMEDIATE

: [>T]  ; IMMEDIATE
:  >T   ; IMMEDIATE

TRUE VALUE J_OPT?
: TT ;
S" src/macroopt.f" INCLUDED

: TSET-OPT SET-OPT ;
: TDIS-OPT DIS-OPT ;
: TOMM_SIZE TO MM_SIZE ;

REQUIRE GTYPE ~mak/djgpp/gdis.f

 TRUE TO ?C-JMP
\ 0    TO ?C-JMP

: TC-COMPILE,  \ 94 CORE EXT
\ »нтерпретаци€: семантика не определена.
\ ¬ыполнение: ( xt -- )
\ ƒобавить семантику выполнени€ определени€, представленого xt, к
\ семантике выполнени€ текущего определени€.
    CON>LIT 
    IF  INLINE?
      IF     INLINE,
      ELSE   _COMPILE,
      THEN
    THEN
;

: _DABS ( d -- ud ) \ 94 DOUBLE
\ ud абсолютна€ величина d.
  DUP 0< IF DNEGATE THEN
;

 0xE9 ' COMPILE, C! 
 ' TC-COMPILE, ' COMPILE, - 5 -  ' COMPILE, 1+ !


: DABS ( d -- ud ) \ 94 DOUBLE
\ ud абсолютна€ величина d.
  DUP 0< IF DNEGATE THEN
;

0 VALUE TSAVE_LIMIT

: TSAVE (  ADDR LEN -- )
 H-STDOUT  >R R/W CREATE-FILE  THROW TO H-STDOUT
  UNIX-LINES
 CR ." MUSEROFFS EQU " USER-HERE RESERVE - 2 MAX .
 CR
 HERE >R
 CONTEXT @ @
 BEGIN
 CR ." AHEADER "
 DUP 1- C@ .   ." ,"
 DUP COUNT ATYPE ." ,"
 DUP COUNT GTYPE
 CR
 R> OVER NAME> GDIS
 DUP NAME>C >R
 CDR 
 DUP TSAVE_LIMIT U<
 UNTIL DROP RDROP CR
 H-STDOUT CLOSE-FILE  THROW R> TO H-STDOUT 
;

: RN> CHAR SWAP WordByAddr DROP C! ;

' CR CONSTANT '_CR

