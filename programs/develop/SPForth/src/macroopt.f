\ Оптимизирующий макроподстановшик        Максимов М.О.
\ http://wiki.forth.org.ru/optimizer

[UNDEFINED] C>S
[IF]
: C>S ( c -- n )  0xFF AND [ 0x7F INVERT ] LITERAL XOR 0x80 + ;
[THEN]

BASE @ HEX

\ : REL! ( ADDR' ADDR  --  )
\        TUCK - SWAP ! ;

: REL@ ( ADDR -- ADDR' )
         DUP @ + ;

\  FALSE VALUE OPT?
   TRUE VALUE OPT?

  084 VALUE J_COD

: SET-OPT TRUE TO OPT? ;

: DIS-OPT FALSE TO OPT? ;

INLINEVAR
[IF]
0x20 VALUE MM_SIZE

0 VALUE OFF-EBP

0 VALUE OFF-EAX

0 VALUE :-SET

0 VALUE J-SET

0 VALUE LAST-HERE

0x44 CELLS DUP CONSTANT OpBuffSize

CREATE OP0 HERE >T DUP , 0 , SWAP ALLOT

DUP OpBuffSize + CELL- CONSTANT OPLast

CELL+ DUP CONSTANT OP1
CELL+ DUP CONSTANT OP2
CELL+ DUP CONSTANT OP3
CELL+ DUP CONSTANT OP4
CELL+ DUP CONSTANT OP5
CELL+ DUP CONSTANT OP6
CELL+ DUP CONSTANT OP7
CELL+ DUP CONSTANT OP8

DROP

: SetOP ( -- )
 OP0 OP1 OpBuffSize CELL- CMOVE>
 DP @ OP0 !
;

: ToOP0 ( OPn -- )
     OP0 OpBuffSize CELL- CMOVE ;

0x11 CELLS DUP CONSTANT JpBuffSize

CREATE JP0 HERE DUP , OVER ALLOT


DUP ROT ERASE

CELL+ DUP CONSTANT JP1


CELL+ DUP CONSTANT JP2


CELL+ DUP CONSTANT JP3


CELL+ DUP CONSTANT JP4

DROP

[THEN]

: ClearJpBuff JP0 JpBuffSize ERASE ;

:  J@ 1+   REL@    CELL+ ;

: SJ@ 1+ DUP C@ C>S + 1+ ;

: J_@
        DUP C@ F0
           AND 70 = IF   SJ@ ELSE
        DUP C@ EB = IF   SJ@ ELSE
        DUP C@ E9 = IF    J@ ELSE
        DUP W@ F0FF
         AND 800F = IF 1+ J@ ELSE
        HEX U. 1 ." J_@ ERR" ABORT
        THEN  THEN THEN THEN
;

: SetJP ( -- )
 JP0 JpBuffSize + CELL- @  DUP
 IF J_@
 THEN
\  DP @ UMIN
 J-SET UMAX TO J-SET
 JP0 JP1 JpBuffSize CELL- CMOVE>
 DP @ JP0 ! ;

\ : ToJP0 ( OPn -- )
\     JP0 JpBuffSize CELL- CMOVE
\ JP0 JpBuffSize + CELL- 0!  ;

: ?SET DP @
       DUP LAST-HERE <> IF DUP TO :-SET DUP TO J-SET THEN
       DUP    OP0 @ U< IF OP0 0! THEN
       DUP    OP1 @ U< IF OP1 0! THEN
       DUP    JP0 @ U< IF JP0 0! THEN
              JP1 @ U< IF JP1 0! THEN
;

: SHORT? ( n -- -129 < n < 128 )
  0x80 + 0x100 U<
;

M\ VECT DTST

\ \\\\\\\\\\

[DEFINED] [TTO] \ CR .( TTO=) DUP .
[IF]

GET-CURRENT

WORDLIST CONSTANT MACROOPT-WL

\ запишем адрес имени словаря
' MACROOPT-WL EXECUTE CELL+ ( name_addr )
LAST @ SWAP !

ALSO ' MACROOPT-WL EXECUTE CONTEXT ! DEFINITIONS
TC-WL ALSO TC-IMM

[THEN]

\ \\\\\\\\\\\\

: EVEN-EAX OFF-EAX
   IF      M\ 1000 DTST
 SetOP OFF-EAX DUP SHORT?
     IF    0408D W, C,
     ELSE  0808D W,  ,
     THEN   \  LEA   EAX,  OFF-EBP [EAX]
     0 TO OFF-EAX
       M\ 1001 DTST
   THEN
;

: EVEN-EBP OFF-EBP
   IF SetOP  OFF-EBP  06D8D W, C, \   LEA   ebp,  OFF-EBP [EBP]
      0 TO OFF-EBP
   THEN
;

: +>OFF-EBP ( C -- )
   C>S OFF-EBP + TO OFF-EBP ;

: ADD|XOR|OR|AND=  ( W -- FLAG )
   CASE
  DUP 4503 <> IF   \ ADD EAX, X2 [EBP]
  DUP 450B <> IF   \  OR
  DUP 4523 <> IF   \ AND
  DUP 4533 <> IF   \ XOR
             DROP FALSE EXIT
  DUPENDCASE DROP TRUE ;

: DUP3B?[EBP]  ( W -- W FLAG )
   CASE
  DUP 0E7C4 AND 04500 <> IF      \ 010X.X101 00XX.X0XX
\ ADD|OR|ADC|SBB|AND|SUB|XOR|CMP  _L | E_X | X [EBP]  , _L | E_X | X [EBP]

  DUP  E7FD AND  4589 <> IF  \ 010X.X101 1000.10X1
\ MOV  X [EBP], E(ABCD)X | E(ABCD)X , X [EBP]
\   DUP           5503 <> IF \ ADD   EDX , 0 [EBP]
   DUP           758B <> IF \ MOV   ESI , X [EBP]

  DUP  EFFF AND  6DDB <> IF  \ FLD | FSTP     EXTENDED 0 [EBP]

  DUP            45DB <> IF  \ FILD DWORD FC [EBP]
  DUP            65F7 <> IF  \  MUL  X [EBP]
  DUP            6DF7 <> IF  \ IMUL  X [EBP]
  DUP           04587 <> IF  \ XCHG EAX , X [EBP]
             FALSE EXIT
  DUPENDCASE TRUE ;

: DUP3B?      ( W -- W FLAG )
   CASE
\  11XX.X000 1000.0011
   DUP C7FF AND  C083 <> IF \ ADD|OR|ADC|SBB|AND|SUB|XOR|CMP  EAX, # X
   DUP          0478B <> IF \ MOV  EAX, X [EDI]
   DUP          0488B <> IF \ MOV  ECX, X [EAX]
   DUP          0588B <> IF \ MOV  EBX, X [EAX]
   DUP          0508B <> IF \ MOV  EDX, X [EAX]
   DUP          0788B <> IF \ MOV  EDI, X [EAX]
   DUP          0708B <> IF \ MOV  ESI, X [EAX]


   DUP          0588D <> IF \ LEA  EBX, X [EAX]
   DUP          0508D <> IF \ LEA  EDX, X [EAX]
   DUP          05089 <> IF \ MOV X [EAX], EDX
   DUP          05189 <> IF \ MOV X [ECX] , EDX 
   DUP          07589 <> IF \ MOV X [EAX], ESI

   DUP          0F8C1 <> IF \ SAR  EAX, # X
   DUP          0E0C1 <> IF \ SHL  EAX, # X
   DUP          0E1C1 <> IF \ SHL  ECX, # X

   DUP          0E8C1 <> IF \ SHR  EAX, # X
   DUP          0408D <> IF \ LEA  EAX , X [EAX]
   DUP          0408B <> IF \ MOV  EAX , X [EAX]
             FALSE EXIT
  DUPENDCASE TRUE ;

: DUP2B?      ( W -- W FLAG )
   CASE
  DUP 0E4C5 AND 0C001 <> IF
\ ADD|OR|ADC|SBB|AND|SUB|XOR|CMP  E_X , E_X

\  DUP           01801 <> IF  \ ADD  [EAX], EBX

  DUP           01001 <> IF  \ ADD  [EAX], EDX
  DUP            0003 <> IF  \ ADD  EAX, [EAX]
  DUP		F633 <> IF  \ XOR  ESI, ESI
  DUP		108B <> IF  \ MOV     EDX , [EAX]
  DUP 0C0FF AND  C085 <> IF \ TEST E__ , E__

\ 110X.X0XX  1000.10X1
  DUP 0E4FC AND 0C088 <> IF \  MOV    E(ABCD)X , E(ABCD)X | (ABCD)L , (ABCD)L

\ 00XX.X0XX  1000.100X

  DUP 0C4FE AND 00088 <> IF \  MOV  [E(ABCD)X] , E(ABCD)X | (ABCD)(HL)
  DUP           0008B <> IF \ MOV EAX, [EAX]
  DUP            028B <> IF \ MOV EAX, [EDX]
  DUP           0C78B <> IF \ MOV EAX,  EDI
  DUP           0F88B <> IF \ MOV EDI,  EAX
  DUP           0F08B <> IF \ MOV ESI,  EAX
  DUP           0C88B <> IF \ MOV ECX , EAX
  DUP           0D18B <> IF \ MOV EDX , ECX
  DUP           0C68B <> IF \ MOV EAX,  ESI
\ 111X.X0XX  1101.00XX
  DUP 0E4FC AND 0E0D0 <> IF \  S(AH)(LR)  (ABCD)L | E(ABCD)X,  CL | 1
  DUP           0C0DD <> IF \ FFREE ST

  DUP  F0FF AND  C0D9 <> IF \  FLD     ST(X)  | FXCH    ST(X)
\     1100.XXXX.1101.1001

  DUP  FAFF AND  E0D9 <> IF \  FCHS|FABS|FTST|FXAM
\     1110.0X0X.1101.1001
  DUP  E8FF AND  E8D9 <> IF \ FLD1 FLDL2T FLDL2E FLDPI FLDLG2 FLDLN2 FLDZ ???
\     1110.1XXX.1101.1001

  DUP  F0FF AND  F0D9 <> IF   \ F2XM1 FYL2X FPTAN FPATAN
\     1111.XXXX.1101.1001    \ FXTRACT FPREM1  FDECSTP FINCSTP
                             \ FPREM   FYL2XP1 FSQRT   FSINCOS
                             \ FRNDINT FSCALE  FSIN    FCOS
  DUP  E8FF AND  20DB <> IF \  FLD     EXTENDED [E_X]  | FST  EXTENDED [E_X]
\   001X.0XXX.1101.1011

  DUP  E8FF AND  00DD <> IF \  FLD    DOUBLE [E_X]  | FST  DOUBLE [E_X]
\   000X.0XXX.1101.1101

  DUP  F0FF AND  C0DE <> IF \  FADDP   ST(X) | FMULP   ST(X)
  DUP  E0FF AND  E0DE <> IF \  FSUBRP  ST(X) | FSUBP   ST(X)
\     111X.XXXX.1101.1110  \  FDIVRP  ST(X) | FDIVP   ST(X)

  DUP            00FF <> IF \ INC [EAX]
\   DUP 0C0FF <> IF \ INC  EAX
\   DUP 0C3FF <> IF \ INC  EBX
\   DUP 0C8FF <> IF \ DEC  EAX
  DUP           0D0F7 <> IF \ NOT EAX
  DUP           0D1F7 <> IF \ NOT ECX
  DUP           0D8F7 <> IF \ NEG EAX
  DUP           0D9F7 <> IF \ NEG ECX
  DUP           0DAF7 <> IF \ NEG EDX
  DUP           0E9F7 <> IF \ IMUL ECX
  DUP           0F1F7 <> IF \  DIV ECX
  DUP           0F9F7 <> IF \ IDIV ECX
             FALSE EXIT
  DUPENDCASE TRUE ;

: DUP6B?      ( W -- W FLAG )
   CASE
\ X00X.X101 1000.10X1
  DUP   67FD AND 0589 <> IF  \ MOV X {[EBP]}, E(ACDB)X | E(ACDB)X , X {[EBP]}

  DUP   C3FF AND C081 <> IF \ ADD|OR|ADC|SBB|AND|SUB|XOR|CMP  EAX, # X

  DUP           00501 <> IF    \ ADD  X , EAX
  DUP            0503 <> IF \ ADD  EAX,  X
  DUP            053B <> IF \ CMP  EAX,  X
  DUP            873B <> IF \ CMP  EAX,  X [EDI]
  DUP            F281 <> IF \ XOR  EDX , # 80000000
  DUP            928D <> IF \ LEA  EDX , [EDX+80000000H]

\  DUP   0589 <> IF \ MOV X , EAX
\  DUP   058B <> IF \ MOV EAX,  X
  DUP            808B <> IF \ MOV EAX, X [EAX]
  DUP            808D <> IF \ LEA EAX, X [EAX]
  DUP            8703 <> IF \ MOV EAX, X [EDI]

  DUP            878B <> IF \ MOV EAX, X [EDI]
  DUP            878D <> IF \ LEA EAX, X [EDI]
  DUP             0C7 <> IF \ MOV [EAX], # X
  DUP            0081 <> IF \ ADD [EAX], # X
  DUP            05FF <> IF \ INC X

\  DUP  FCFF AND C0C7 <> IF \ MOV EAX|EBX|ECX|EDX, # X
             FALSE EXIT
  DUPENDCASE TRUE ;

: DUP5B?      ( C -- C FLAG )


  DUP   0C7 AND   5 =    \  ADD|OR|ADC|SBB|AND|SUB|XOR|CMP  EAX, # X
  OVER  0FC AND 0B8 = OR \  MOV EAX|EBX|ECX|EDX, # X
  OVER  0FD AND 0A1 = OR \  MOV EAX, X  | X , EAX
;

: DUP7B?      ( N -- N FLAG )
\ XX00.0101 0000.0100 1000.10X1
  DUP  3FFFFD AND 050489 =    \  MOV   X [EAX*_] , EAX  | MOV EAX , X [EAX*_]
\ XX00.0101 0000.0100 1000.1X01
  OVER 3FFFFB AND 050489 = OR \  MOV   X [EAX*_] , EAX  | LEA EAX , X [EAX*_]
  OVER 80B60F = OR \ MOVZX EAX, BYTE PTR [EAX]
  OVER 45048D = OR \ LEA  EAX, X [EAX*2]

;

 0 VALUE TTTT

\ 0 VALUE ZZZZ \ VECT VVV

: ?ChEAX  ( ADDR --  FALSE | TRUE )
   CASE
   DUP C@
   DUP       B8 <> IF   \  MOV  EAX, # X
   DUP       A1 <> IF   \  MOV  EAX,   X
   DROP
   DUP W@
\   DUP ADD|XOR|OR|AND= 0= IF
   DUP     C033 <> IF   \  XOR  EAX, EAX
   DUP     C031 <> IF   \  XOR  EAX, EAX
   DUP     D889 <> IF   \  MOV  EAX, EBX

   38FF AND
   DUP  008B <> IF   \  MOV  EAX, ___
   DUP  008D <> IF   \  LEA  EAX, ___
   DROP
   DUP  @  FFFFFF AND 
   DUP  02048D <> IF   \  LEA  EAX, [EDX] [EAX]

             2DROP TRUE EXIT
  DUPENDCASE 2DROP FALSE ;

: ^?EAX=  ( ADDR --  FALSE | TRUE )
\ FALSE
   DUP ?ChEAX IF DROP TRUE EXIT THEN
   DUP W@
   DUP  00F9
   AND  0089 <> IF 2DROP FALSE EXIT THEN
\ LEA MOV
   CASE
   FF00 AND
   DUP 0100 AND
       0100 <> IF   \  MOV  EAX,

   DUP  C200 <> IF   \  MOV  EAX, EDX
   DUP  D800 <> IF   \  MOV  EAX, EBX
   DROP
   DUP @ FFFF00 AND

   DUP   240400 <> IF   \  MOV  EAX,   [ESP]
   DUP   244400 <> IF   \  MOV  EAX, X [ESP]
   DUP   95048B <> IF   \ 8B0495F0065A00        MOV     EAX , 5A06F0 [EDX*4]
             2DROP TRUE EXIT
  DUPENDCASE 2DROP FALSE ;

\   DUP   8B00 =     \  MOV  EAX, [EAX]

: OP_SIZE ( OP - n )
  DUP IF THEN  DUP CELL- @ SWAP @ -
;

: OPexcise ( OPX -- )
      DUP OP0 = IF @ DP ! OP1 ToOP0 EXIT THEN
      >R
      R@ CELL- @ R@ @  DP @ R@ CELL- @ - CMOVE

      R@ OP_SIZE NEGATE
      R@ OP0 DO DUP I +! CELL +LOOP
      ALLOT
      R@ CELL+ R@ OpBuffSize CELL- R> - OP0 + CMOVE
;
: ?OPlast  ( OPX -- OPX flag )
     DUP OP0 OpBuffSize + CELL- CELL- U> ;

: XX_STEP ( OPX -- OPX+CELL FALSE | { OPX | FALSE } TRUE )
\ Проверка на не изменение  EAX
     ?OPlast
     IF DROP FALSE TRUE EXIT THEN
     DUP @
     DUP  :-SET U< IF 2DROP FALSE TRUE EXIT THEN
     C@
   CASE
     DUP 3D <> IF   \ CMP EAX, # X
     DUP 3B <> IF   \ CMP E_X , X
     DUP A3 <> IF   \ MOV X , EAX
     DUP B9 <> IF   \ MOV ECX , # X
     DUP BA <> IF   \ MOV EDX , # X
     DUP BB <> IF   \ MOV EBX , # X
     DUP 50 <> IF   \ PUSH EAX
     DROP
     DUP @ W@
     DUP 04FF AND 04C7 <> IF \ MOV 4444 , # 5555
     DUP 40FE AND 0088 <> IF \ MOV   [EAX] , DL
     DUP 4D89 <> IF \ MOV 4 [EBP] , ECX
     DUP 558B <> IF \ MOV EDX , FC [EBP]
     DUP 4C8B <> IF \ MOV ECX , FC [E__] [E__]
     DUP 648D <> IF \ LEA  ESP , 8 [ESP]
     DUP 34FF AND 04FF <> IF \ INC 0 [ESP]
     DUP C00B <> IF \ OR  EAX, EAX
     DUP D00B <> IF \ OR  EDX, EAX
     DUP D02B <> IF \ SUB EDX, EAX
   DUP 0401 <> IF \ ADD [E_X]
     DUP 06FD  \ CMP X [EBP], # Z   \ CMP 44444, # 55555
     AND 0481 <> IF
     DUP FFFD
     AND 4589 =
       IF   OVER @ 2+ C@ OP0 @  2+ C@ =
         IF  DROP TRUE
             EXIT
         THEN
         458B = IF DROP FALSE TRUE EXIT THEN
             CELL+ FALSE EXIT
       THEN
     2DROP
     FALSE TRUE EXIT
  DUPENDCASE  DROP CELL+ FALSE ;

: ?EDX_[EBP]   ( OPX -  FLAG )
  DUP @   @ FFFFFF AND
  DUP  24048B =  \  8B0424		MOV     EAX , [ESP]
        IF 2DROP FALSE EXIT THEN
  FFFF AND

  DUP  458B XOR     \  8B4500	      MOV     EAX , 0 [EBP]
  OVER 4589 XOR OR  \ OPX N F MOV     FC [EBP] , EAX
0=      IF DROP
            @ 2+ C@ OP0 @ 2+ C@ =
            EXIT
        THEN
  NIP
  DUP  ADD|XOR|OR|AND=
        IF DROP FALSE EXIT THEN
  DUP  E8C1  =   \  C1E819            SHR     EAX , 19
        IF DROP FALSE EXIT THEN
  DUP  418D  =   \  8D41FF            LEA     EAX , FF [ECX]
        IF DROP FALSE EXIT THEN
  DUP  408D  =   \  8D40FF            LEA     EAX , FF [EAX]
        IF DROP FALSE EXIT THEN
  DUP  878D  =   \  8D873C100000      LEA     EAX , 103C [EDI]
        IF DROP FALSE EXIT THEN
  DUP  D0F7  =   \  F7D0	      NOT     EAX
        IF DROP FALSE EXIT THEN
  DUP  D8F7  =   \  F7D8	      NEG     EAX
        IF DROP FALSE EXIT THEN
  DUP  C123  =   \  23C1              AND     EAX , ECX
        IF DROP FALSE EXIT THEN

  DUP  C033  =    \       XOR     EAX , EAX
        IF DROP FALSE EXIT THEN

  DUP  808D  =   \  8D8024475A00	LEA     EAX , 5A4724  [EAX]
        IF DROP FALSE EXIT THEN

  DUP  088B  =   \  MOV     ECX , [EAX]
        IF DROP FALSE EXIT THEN

  DUP  C00B  =   \  0BC0         OR      EAX , EAX
        IF DROP FALSE EXIT THEN

  DUP  488D  =   \ 8D4820		LEA     ECX , 20 [EAX]
        IF DROP FALSE EXIT THEN

  DUP  E0D3  =   \ D3E0           SHL     EAX , CL
        IF DROP FALSE EXIT THEN

  FF AND
  DUP  A1    =  \  A1047A5800        MOV     EAX , 587A04
        IF DROP FALSE EXIT THEN
  DUP  58    =  \        58         POP     EAX
        IF DROP FALSE EXIT THEN
  DUP  B9    =  \  B907000000 MOV    ECX , # 7
        IF DROP FALSE EXIT THEN
  DROP TRUE
;

: MOV_EDX_[EBP]  ( OPX - OPX' FALSE | FLAG TRUE )
        ?OPlast  IF DROP FALSE TRUE EXIT THEN
  DUP @ :-SET U< IF DROP FALSE TRUE EXIT THEN
  DUP @ ?ChEAX 0= IF CELL+ FALSE EXIT THEN
  DUP  ?EDX_[EBP] 0= IF CELL+ FALSE EXIT THEN
  DUP @ W@

  DUP 448B =              \ MOV     EAX , 4 [EDX] [EAX*4]
        IF DROP CELL+ FALSE EXIT THEN

  DUP 1489 =              \ MOV     59C2E4 [EAX*4] , EDX
        IF DROP CELL+ FALSE EXIT THEN
  DUP 8BCA =              \ MOV     ECX , EDX
        IF DROP CELL+ FALSE EXIT THEN
  DUP FFFD AND 5589 =              \ OPX N F
        IF DROP @ 2+ C@  OP0 @ 2+ C@ = TRUE EXIT
        THEN
  2DROP FALSE  TRUE
;

: OPresize ( OPX n -- )
  DUP >R
  OVER OP0 ?DO DUP I +! CELL +LOOP
  ALLOT
  @ DUP  R> +  DUP DP @ - NEGATE MOVE
;

: OPinsert ( OPX n -- )
  DUP >R
  2DUP OPresize DROP
  DUP
  DUP CELL + OVER OP0 CC -  OpBuffSize CELL- - NEGATE MOVE
  R> SWAP +!
;

VARIABLE ?~EAX
VARIABLE SAVE-?~EAX
: ?~EAX{ ( FLAG -- )
 ?~EAX @  SAVE-?~EAX ! ?~EAX ! ;

: }?~EAX (  -- )
   SAVE-?~EAX @ ?~EAX ! ;


: ?EAX>EBX  ( OPX - OPX' FALSE | FALSE TRUE | OPX' TRUE TRUE )
\     TTTT 0= IF DROP FALSE TRUE EXIT THEN
\  ." $="  DUP @ @ U.
\  TTTT IF HEX DUP @ U. THEN
\ DROP FALSE TRUE EXIT
     ?OPlast     IF DROP FALSE TRUE EXIT THEN
  DUP @ :-SET U< IF DROP FALSE TRUE EXIT THEN
  DUP @ W@
   DUP 4589 =     \ OPX N F  MOV     FC [EBP] , EAX
  OVER 458B = OR  \ OPX N F  MOV     EAX , FC [EBP]
        IF DROP DUP @ 2+ C@  OP0 @ 2+ C@ =
           IF   CELL- TRUE TRUE
           ELSE DUP @ W@ 458B =  IF ?~EAX 0! THEN
                CELL+ FALSE
           THEN      EXIT
        THEN

   CASE
\ ." $="  DUP @ U.
  DUP 083B <> \ 	CMP     ECX , [EAX]
        IF
  DUP C83B <> \ 	CMP     ECX , EAX
        IF
  DUP C13B <> \ 	CMP     EAX , ECX
        IF
  DUP 508B <> \ 	MOV     EDX , 4 [EAX]
        IF
  DUP 088B <> \ 	MOV     ECX , [EAX]
        IF
  DUP 098B <> \ 	MOV     ECX , [ECX]
        IF
  DUP C88B <> \ 	MOV     ECX , EAX
        IF
  DUP 0889 <> \ 	MOV     [EAX] , ECX
        IF
  DUP 1089 <> \ 	MOV     [EAX] , EDX
        IF
  DUP 4889 <> \ 	MOV     4 [EAX] , ECX
        IF
0 ?~EAX{
  DUP 408D <> \		LEA     EAX , 1 [EAX]
        IF
  DUP 808D <> \		LEA     EAX , 1 [EAX]
        IF
  DUP 008B <> \		MOV     EAX , [EAX]
        IF
\  TTTT IF HEX ." M="  DUP  U. THEN

  DUP C18B <> \		MOV     EAX , ECX
        IF
TRUE ?~EAX !
  DUP D8F7 <>       \ NEG     EAX
        IF
}?~EAX
  DROP
  DUP @ @  FFFFFF AND
  DUP 240C8B <>     \ MOV     ECX , [ESP]
        IF
  DUP 8D0489 <>     \ MOV     X [ECX*4] , EAX
        IF
0 ?~EAX{
  DUP 24048B <>     \ MOV     EAX , [ESP]
        IF
TRUE ?~EAX !
  DUP 24442B <> \ 	SUB     EAX , 4 [ESP]
        IF
}?~EAX
  0xFFFF AND
  OVER @ 2+ C@  OP0 @ 2+ C@
\  TTTT IF HEX ." N="  2DUP U. U. THEN
 =
        IF   2DROP FALSE  TRUE EXIT
        THEN
  DUP 4D8B <> \		MOV     ECX , [EBP]
        IF
  DUP 5589 <> \	        MOV     [EBP] , EDX
        IF

  2DROP FALSE  TRUE EXIT
  DUPENDCASE  DROP CELL+ FALSE ;

: EAX>EBX0  ( OPX - OPX' FLAG )


  DUP OP0 = IF  TRUE EXIT THEN
\  TTTT IF 1 EMIT DUP U. DUP @ U. THEN
  DUP @ W@

  DUP 508B =  \ MOV     EDX , [EAX+4]
        IF DROP CELL- FALSE EXIT THEN
  DUP 088B = \ 	MOV     ECX , [EAX]
        IF DROP CELL- FALSE EXIT THEN
  DUP 098B = \ 	MOV     ECX , [ECX]
        IF DROP CELL- FALSE EXIT THEN
  DUP 4D8B = \ 	MOV     ECX , [EBP]
        IF DROP CELL- FALSE EXIT THEN
  DUP 5589 = \  MOV     0 [EBP] , EDX
        IF DROP CELL- FALSE EXIT THEN
  DUP C88B = \ 	MOV     ECX , EAX
        IF DROP CELL- FALSE EXIT THEN
  DUP 0889 = \ 	MOV     [EAX] , ECX
        IF DROP CELL- FALSE EXIT THEN
  DUP 1089 = \ 	MOV     [EAX] , EDX
        IF DROP CELL- FALSE EXIT THEN
  DUP 4889 = \ 	MOV     4 [EAX] , ECX
        IF DROP CELL- FALSE EXIT THEN
  DUP 408D =              \ LEA     EAX , 1 [EAX]
        IF DROP
           58 OVER @ 1+ C!  CELL- TRUE EXIT
        THEN

  DUP 808D =              \ LEA     EAX , 1 [EAX]
        IF DROP
           98 OVER @ 1+ C!  CELL- TRUE EXIT
        THEN
  DUP 008B =     \ MOV     EAX , [EAX]
        IF DROP
           18  OVER @ 1+ C!  CELL- TRUE EXIT
        THEN

  DUP C18B =     \ MOV     EAX , ECX
        IF DROP
           D9  OVER @ 1+ C!  CELL- TRUE EXIT
        THEN

  DUP 458B =       \ MOV     EAX , [EBP+X]
        IF DROP
           5D  OVER @ 1+ C!  CELL- TRUE EXIT
        THEN

  DUP D8F7 =       \ NEG     EAX
        IF DROP
           DB OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DROP
  DUP @ @  FFFFFF AND

  DUP 8D0489 =     \ MOV     X [ECX*4] , EAX
        IF DROP CELL- FALSE EXIT THEN

  DUP 24048B =              \ MOV     EAX , [ESP]
        IF DROP
           1C OVER @ 1+ C!  CELL- TRUE EXIT
        THEN
  DUP 240C8B =              \ MOV     ECX , [ESP]
        IF DROP CELL- FALSE EXIT THEN
\ BASE @ >R HEX DUP U. R> BASE !
  FF AND
  DUP   3B =       \ CMP
        IF DROP CELL- FALSE EXIT THEN
 HEX  U. U. ." EAX>EBX0" ABORT
;

: EAX>EBX  ( OPX - OPX' FLAG )


  DUP OP0 = IF  TRUE EXIT THEN
  DUP @ W@ \ ." #=" DUP U.

  DUP 508B =     \ MOV     EDX , [EAX+4]
        IF DROP
           53  OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP 083B = \ 	CMP     ECX , [EAX]
        IF DROP
           0B  OVER @ 1+ W!  CELL- FALSE EXIT
        THEN

  DUP C83B = \ 	CMP     ECX , EAX
        IF DROP
           CB  OVER @ 1+ W!  CELL- FALSE EXIT
        THEN

  DUP C13B = \ 	CMP     EAX , ECX
        IF DROP
           D9  OVER @ 1+ W!  CELL- FALSE EXIT
        THEN

  DUP 088B = \ 	MOV     ECX , [EAX]
        IF DROP
           0B  OVER @ 1+ C!  CELL- FALSE EXIT
        THEN
  DUP 458B =       \ MOV     EAX , [EBP+X]
        IF DROP
           5D  OVER @ 1+ C!  CELL- FALSE EXIT
        THEN
  DUP 098B = \ 	MOV     ECX , [ECX]
        IF DROP CELL- FALSE EXIT THEN

  DUP 4D8B = \ 	MOV     ECX , [EBP]
        IF DROP CELL- FALSE EXIT THEN

  DUP 5589 = \  MOV     0 [EBP] , EDX
        IF DROP CELL- FALSE EXIT THEN

  DUP C88B = \ 	MOV     ECX , EAX
        IF DROP
           CB  OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP 0889 = \ 	MOV     [EAX] , ECX
        IF DROP
           0B  OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP 1089 = \ 	MOV     [EAX] , EDX
        IF DROP
           13  OVER @ 1+ C!  CELL- FALSE EXIT
        THEN
  DUP 4889 = \ 	MOV     4 [EAX] , ECX
        IF DROP
           4B  OVER @ 1+ C!  CELL- FALSE EXIT
        THEN
  DUP 4589 =     \ OPX N F  MOV     FC [EBP] , EAX
        IF DROP
           5D OVER @ 1+ C!  CELL- FALSE EXIT
        THEN
  DUP 408D =              \ LEA     EAX , 1 [EAX]
        IF DROP
           5B OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP 808D =              \ LEA     EAX , X [EAX]
        IF DROP
           9B OVER @ 1+ C!  CELL- FALSE EXIT
        THEN
  DUP 008B =     \ MOV     EAX , [EAX]
        IF DROP
           1B OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP C18B =     \ MOV     EAX , ECX
        IF DROP
           D9  OVER @ 1+ C!  CELL- FALSE EXIT
        THEN


  DUP D8F7 =       \ NEG     EAX
        IF DROP
           DB OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DROP
  DUP @ @  FFFFFF AND

  DUP 8D0489 =     \ MOV     X [ECX*4] , EAX
        IF DROP
           1C OVER @ 1+ C!  CELL-  FALSE EXIT
        THEN

  DUP 24048B =              \ MOV     EAX , [ESP]
        IF DROP
           1C OVER @ 1+ C!  CELL-  FALSE EXIT
        THEN
  DUP 240C8B =              \ MOV     ECX , [ESP]
        IF DROP CELL- FALSE EXIT THEN
  DUP 24442B =        \ SUB     EAX , [ESP+4]
        IF DROP
           5C OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

HEX  U. U. ." EAX>EBX" ABORT
;

: ?EAX>ECX  ( OPX -   OPX' FALSE | FALSE TRUE | OPX' TRUE TRUE )
\   ZZZZ 0= IF DROP FALSE TRUE EXIT THEN
\ DROP FALSE TRUE EXIT
     ?OPlast     IF DROP FALSE TRUE EXIT THEN
  DUP @ :-SET U< IF DROP FALSE TRUE EXIT THEN
\  TTTT IF HEX DUP @ U. THEN
  DUP @ W@
   DUP 4589 =     \ OPX N F  MOV     FC [EBP] , EAX
  OVER 458B = OR  \ OPX N F  MOV     EAX , FC [EBP]
        IF DROP DUP @ 2+ C@  OP0 @ 2+ C@ =
           IF   CELL- TRUE TRUE
           ELSE  DUP @ W@ 458B = IF ?~EAX 0! THEN
                CELL+ FALSE
           THEN      EXIT
        THEN
  DUP 5589 =      \ OPX N F  MOV     FC [EBP] , EDX
        IF DROP DUP @ 2+ C@  OP0 @ 2+ C@ =
           IF   DROP  FALSE TRUE
           ELSE CELL+ FALSE
           THEN      EXIT
        THEN

   CASE
 0 ?~EAX{

\ 8B00		MOV     EAX , [EAX]
\ 3B05E3745400	CMP     EAX , 5474E3  ( :-SET+5  )
  DUP 3AFF AND 028B <>   \ MOV     EAX , [EDX]
        IF
  DUP 008B <>       \ MOV     EAX , [EAX]
        IF
  DUP 408D <>       \ LEA     EAX , 1 [EAX]
        IF
  DUP 408B <>       \ MOV     EAX , 1 [EAX]
        IF
  DUP 428D <>       \ LEA     EAX , 1 [EDX]
        IF
\  DUP 808D <>       \ LEA     EAX , X [EAX]
\        IF
  DUP C28B <> 	    \ MOV     EAX , EDX
        IF
TRUE ?~EAX !
  DUP C00B <>       \ OR      EAX , EAX
        IF
  DUP C01B <>       \ SBB     EAX , EAX
        IF
  DUP F8D1 <>       \ SAR     EAX , 1
        IF
  DUP E8C1 <>       \ SHR     EAX , 1
        IF
  DUP D8F7 <>       \ NEG     EAX
        IF
  DUP D0F7 <>       \ NOT     EAX
        IF
  DUP D0F7 <>       \ NOT     EAX
        IF
  DUP 4503 <>       \ ADD     EAX , 0 [EBP]
        IF
  DUP 0523 <>       \ AND     EAX , 587744  ( D+5  )
        IF
  DUP E0C1 <> 	    \ SHL     EAX , 7
        IF
  DUP C223 <> 	    \ AND     EAX , EDX
        IF
 }?~EAX
  DUP 508B <>       \ MOV     EDX , [EAX+4]
        IF
  DUP 053B <>       \ CMP     EAX , 5474E3  ( :-SET+5  )
        IF
  DUP 103B <>       \ CMP     EDX , [EAX]
        IF
  DUP 558B <>       \ OPX N F  MOV     F8 [EBP] , EDX
        IF
  DUP 158B <>       \ MOV     EDX , 587A88
        IF
  DUP D08B <>       \ MOV     EDX , EAX
        IF
  DUP 0501 <>       \ MOV     ADD     555555 , EAX
        IF
  DUP 5089 <>       \ MOV     C [EAX] , EDX
        IF
  DUP 1089 <>       \ MOV     [EAX] , EDX
        IF

  DUP 80C6 <>       \ MOV   BYTE [EAX] , # X
        IF
  DUP 45C7 <>       \ MOV     FC [EBP] , # 1
        IF
  FF AND
  DUP 50 <>         \  PUSH EAX
        IF
  DUP A3 <>         \  MOV     1000 , EAX
        IF
0 ?~EAX{
  DUP B8 <>         \  MOV     EAX , # 1000
        IF
  DUP A1 <>         \  MOV     EAX ,   1000
        IF
TRUE ?~EAX !
  DUP 25 <>       \ AND     EAX , # X
        IF

}?~EAX
  DUP BA <>         \  MOV     EDX , # 1000
        IF
  DUP 3D <>         \  CMP     EAX , # 1000
        IF
  DROP
  DUP @ @
0 ?~EAX{
  DUP 0244B60F <>     \ MOVZX   EAX , BYTE PTR 2 [EDX] [EAX]
        IF
  FFFFFF AND
  DUP 85048D <>     \ LEA     EAX , 0 [EAX*4]
        IF
  DUP 85048B <>     \ MOV     EAX , X [EAX*4]
        IF
  DUP 80B60F <>     \ MOVZX   EAX , BYTE PTR X [EAX]
        IF
  DUP 40B70F <>     \ MOVZX   EAX , WORD PTR X [EAX]
        IF
  DUP 00B70F <>     \ MOVZX   EAX , WORD [EAX]
        IF
  DUP 82448B <>     \ MOV     EAX , 4 [EDX] [EAX*4]
        IF
  DUP 24048B <>     \ MOV     EAX , [ESP]
        IF
  DUP 24448B <>     \ MOV     EAX , [ESP+4]
        IF
TRUE ?~EAX !
  DUP 24442B <>     \ SUB     EAX , [ESP+4]
        IF
}?~EAX
  DUP 24043B <>    \ CMP     EAX , [ESP]
        IF
  DUP 24443B <>    \ CMP     EAX , 20 [ESP]
        IF
  DUP 24548B <>    \ MOV     EDX , C [ESP]
        IF
  2DROP FALSE  TRUE EXIT
  DUPENDCASE  DROP CELL+ FALSE ;

: EAX>ECX0  ( OPX - OPX' FLAG )

  DUP OP0 = IF  TRUE EXIT THEN
  DUP @ W@
  DUP EFFF AND 4589 =     \ OPX N F
        IF DROP CELL- FALSE EXIT THEN

  DUP 0501 =       \ ADD  [444444H]  , EAX
        IF DROP CELL- FALSE EXIT THEN

  DUP 5589 =       \ MOV     X [EBP] , EDX
        IF DROP CELL- FALSE EXIT THEN
  DUP 38FF AND 008B =     \ MOV     EAX , ___
        IF DROP
           0800  OVER @  +!  CELL- TRUE EXIT
        THEN
  DUP C28B =  	    \ MOV     EAX , EDX
        IF DROP
           CA OVER @ 1+ C!  CELL- TRUE EXIT
        THEN
  DUP 808D =              \ LEA     EAX , 1 [EAX]
        IF DROP
           88 OVER @ 1+ C!  CELL- TRUE EXIT
        THEN
  DUP 408D =              \ LEA     EAX , 1 [EAX]
        IF DROP
           48 OVER @ 1+ C!  CELL- TRUE EXIT
        THEN

  DUP 408B =              \ MOV     EAX , 1 [EAX]
        IF DROP
           48 OVER @ 1+ C!  CELL- TRUE EXIT
        THEN

  DUP FF AND 8B =         \  MOV   ~EAX , ___
        IF DROP CELL- FALSE EXIT THEN

  DUP 38FF AND 008D =     \  LEA    EAX , ___
        IF DROP
           0800  OVER @  +!  CELL- TRUE EXIT
        THEN
  DUP 428D =       \ LEA     EAX , 1 [EDX]
        IF DROP
           0800  OVER @  +!  CELL- TRUE EXIT
        THEN

  DUP C00B =     \ OR     EAX , EAX
        IF DROP CELL- FALSE EXIT THEN

  DUP 5089 =           \ MOV     C [EAX] , EDX
        IF DROP CELL- FALSE EXIT THEN

  DUP 1089 =           \ MOV      [EAX] , EDX
        IF DROP CELL- FALSE EXIT THEN
  DUP 80C6 =       \ MOV   BYTE [EAX] , # X
        IF DROP CELL- FALSE EXIT THEN

  DUP 45C7 =       \ MOV     FC [EBP] , # 1
        IF DROP CELL- FALSE EXIT THEN
  DROP
  DUP @ @
  DUP 0244B60F =     \ MOVZX   EAX , BYTE PTR 2 [EDX] [EAX]
        IF DROP
           4C OVER @ 2+ C!  CELL- TRUE EXIT
        THEN

 FFFFFF AND

  DUP 80B60F =     \ MOVZX   EAX , BYTE PTR X [EAX]
        IF DROP
           88 OVER @ 2+ C!  CELL- TRUE EXIT
        THEN

  DUP 40B70F =     \ MOVZX   EAX , WORD PTR x [EAX]
        IF DROP
           48 OVER @ 2+ C!  CELL- TRUE EXIT
        THEN

  DUP 00B70F =     \ MOVZX   EAX , WORD PTR [EAX]
        IF DROP
           08 OVER @ 2+ C!  CELL- TRUE EXIT
        THEN

\  DUP 24442B =        \ SUB     EAX , [ESP+4]
\        IF DROP
\           4C OVER @ 1+ C!  CELL- TRUE EXIT
\        THEN

  FF AND
  DUP 50 =          \  PUSH EAX
        IF DROP CELL- FALSE EXIT THEN
  DUP B8 =         \  MOV     EAX , # 1000
        IF DROP
           B9 OVER @  C!  CELL- TRUE EXIT
        THEN
  DUP BA =         \  MOV     EDX , # 1000
        IF DROP CELL- FALSE EXIT THEN
  DUP 3D =       \  CMP     EAX , # 1000
        IF DROP CELL- FALSE EXIT THEN
  DUP 3B =       \  CMP
        IF DROP CELL- FALSE EXIT THEN

  DUP A3 =         \  MOV     1000 , EAX
        IF DROP CELL- FALSE EXIT THEN

  DUP A1 =         \  MOV     EAX , 1000
        IF DROP  DUP 1 OPresize
           0D8B OVER @  W!  CELL- TRUE EXIT
        THEN

HEX  U. DUP @ @ U.  U. ." EAX>ECX0" ABORT
;

: EAX>ECX  ( OPX - OPX' F | T )

  DUP OP0  = IF  TRUE EXIT THEN
  DUP @  W@
\ 8B00		MOV     EAX , [EAX]
\ 3B05E3745400	CMP     EAX , 5474E3  ( :-SET+5  )

  DUP 5589 =       \ MOV     X [EBP] , EDX
        IF DROP CELL- FALSE EXIT THEN

  DUP 3AFF AND 028B =   \ MOV     EAX , [EDX]
        IF DROP
           0800  OVER @ +!  CELL- FALSE EXIT
        THEN
  DUP 458B =       \ MOV     EAX , [EBP+X]
        IF DROP
           4D  OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP 008B =     \ MOV     EAX , [EAX]
        IF DROP
           09  OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP D08B =     \ MOV     EDX , EAX
        IF DROP
           D1  OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP C28B =  	    \ MOV     EAX , EDX
        IF DROP
           CA OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP E0C1 =  	    \ SHL     EAX , 7
        IF DROP
           E1 OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP C223 = 	    \ AND     EAX , EDX
        IF DROP
           CA OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP 0501 =       \ ADD  [444444H]  , EAX
        IF DROP
          0D01  OVER @  W!  CELL- FALSE EXIT
        THEN

  DUP 508B =     \ MOV     EDX , [EAX+4]
        IF DROP
           51  OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP 053B =     \ CMP     EAX , 5474E3  ( :-SET+5  )
        IF DROP
           0D  OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP 103B =     \ CMP     EDX , [EAX]
        IF DROP
           11  OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP C23B =     \ CMP     EAX , EDX
        IF DROP
           CA  OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP C00B =     \ OR     EAX , EAX
        IF DROP
           C9  OVER @ 1+ C!  CELL- FALSE EXIT
        THEN
  DUP C01B =       \ SBB     EAX , EAX
        IF DROP
           C9  OVER @ 1+ C!  CELL- FALSE EXIT
        THEN
  DUP F8D1 =       \ SAR     EAX , 1
        IF DROP
           F9  OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP E8C1 =       \ SAR     EAX , 1
        IF DROP
           E9  OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP 4589 =     \ OPX N F  MOV     FC [EBP] , EAX
        IF DROP
           4D OVER @ 1+ C!  CELL- FALSE EXIT
        THEN
  DUP 5089 =           \ MOV     C [EAX] , EDX
        IF DROP
           51 OVER @ 1+ C!  CELL- FALSE EXIT
        THEN
  DUP 1089 =           \ MOV      [EAX] , EDX
        IF DROP
           11 OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP 80C6 =       \ MOV   BYTE [EAX] , # X
        IF DROP
           81 OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP 408D =              \ LEA     EAX , 1 [EAX]
        IF DROP
           49 OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP 408B =              \ MOV    EAX , 1 [EAX]
        IF DROP
           49 OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP 428D =       \ LEA     EAX , 1 [EDX]
        IF DROP
           0800  OVER @  +!  CELL- FALSE EXIT
        THEN

  DUP 808D =              \ LEA     EAX , 1 [EAX]
        IF DROP
           89 OVER @ 1+ C!  CELL- FALSE EXIT
        THEN
  DUP C28B =  	    \ MOV     EAX , EDX
        IF DROP
           CA OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP D8F7 =       \ NEG     EAX
        IF DROP
           D9 OVER @ 1+ C!  CELL- FALSE EXIT
        THEN
  DUP D0F7 =       \ NOT     EAX
        IF DROP
           D1 OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP D0F7 =       \ NOT     EAX
        IF DROP
           D1 OVER @ 1+ C!  CELL- FALSE EXIT
        THEN
  DUP 4503 =       \ ADD     EAX , 0 [EBP]
        IF DROP
           4D OVER @ 1+ C!  CELL- FALSE EXIT
        THEN
  DUP 0523 =       \ AND     EAX , 587744  ( D+5  )
        IF DROP
           0D OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP 558B =              \ OPX N F  MOV     F8 [EBP] , EDX
        IF DROP CELL- FALSE EXIT THEN
  DUP 158B =       \ MOV     EDX , 587A88
        IF DROP CELL- FALSE EXIT THEN
  DUP 45C7 =       \ MOV     FC [EBP] , # 1
        IF DROP CELL- FALSE EXIT THEN
  DROP
  DUP @ @
  DUP 0244B60F =     \ MOVZX   EAX , BYTE PTR 2 [EDX] [EAX]
        IF DROP
           0A4C OVER @ 2+ W!  CELL- TRUE EXIT
        THEN
 FFFFFF AND
  DUP 85048D =     \ LEA     EAX , 0 [EAX*4]
        IF DROP
           8D0C OVER @ 1+ W!  CELL- FALSE EXIT
        THEN

  DUP 85048B =     \ MOV     EAX , X [EAX*4]
        IF DROP
           8D0C OVER @ 1+ W!  CELL- FALSE EXIT
        THEN
  DUP 80B60F =     \ MOVZX   EAX , BYTE PTR X [EAX]
        IF DROP
           89  OVER @ 2+ C!  CELL- FALSE EXIT
        THEN

  DUP 40B70F =     \ MOVZX   EAX , WORD PTR x [EAX]
        IF DROP
           49 OVER @ 2+ C!  CELL-  FALSE EXIT
        THEN

  DUP 00B70F =     \ MOVZX   EAX , WORD PTR [EAX]
        IF DROP
           09 OVER @ 2+ C!  CELL- FALSE EXIT
        THEN

  DUP 82448B =              \ MOV     EAX , 4 [EDX] [EAX*4]
        IF DROP
           8A4C OVER @ 1+ W!  CELL- FALSE EXIT
        THEN
  DUP 24048B =              \ MOV     EAX , [ESP]
        IF DROP
           0C OVER @ 1+ C!  CELL-  FALSE EXIT
        THEN
  DUP 24448B =              \ MOV     EAX , [ESP+4]
        IF DROP
           4C OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  DUP 24043B =              \ CMP     EAX , [ESP]
        IF DROP
           0C OVER @ 1+ C!  CELL-  FALSE EXIT
        THEN
  DUP 24443B =              \ CMP     EAX , 20 [ESP]
        IF DROP
           4C OVER @ 1+ C!  CELL-  FALSE EXIT
        THEN
  DUP 24548B =    \ MOV     EDX , C [ESP]
        IF DROP CELL- FALSE EXIT THEN
  DUP 24442B =        \ SUB     EAX , [ESP+4]
        IF DROP
           4C OVER @ 1+ C!  CELL- FALSE EXIT
        THEN

  FF AND
  DUP 50 =          \  PUSH EAX
        IF DROP
           51 OVER @  C!  CELL- FALSE EXIT
        THEN
  DUP B8 =         \  MOV     EAX , # 1000
        IF DROP
           B9 OVER @  C!  CELL- FALSE EXIT
        THEN
  DUP A3 =         \  MOV     1000 , EAX
        IF DROP  DUP 1 OPresize
           0D89 OVER @  W!  CELL- FALSE EXIT
        THEN
  DUP A1 =         \  MOV     EAX , # 1000
        IF DROP  DUP 1 OPresize
           0D8B OVER @  W!  CELL- FALSE EXIT
        THEN
  DUP 25 =       \ AND     EAX , # X
        IF DROP  DUP 1 OPresize
           E181 OVER @  W!  CELL- FALSE EXIT
        THEN
  DUP BA =         \  MOV     EDX , # 1000
        IF DROP CELL- FALSE EXIT THEN
  DUP 3D =         \  CMP     EAX , # 1000
        IF DROP  DUP 1 OPresize
           F981 OVER @  W!  CELL- FALSE EXIT
        THEN
  U. DUP @ @ U.  U. ." EAX>ECX" ABORT
;

: -EBPLIT   ( n OPX  -- n OPX' )
   DUP @  :-SET  U> 0= IF EXIT THEN
  BEGIN
          ?OPlast     IF EXIT THEN
     DUP @ W@ 6D8D =  IF EXIT THEN \ LEA     EBP , X [EBP]
     DUP @ C@ E8   =  IF EXIT THEN \ CALL
     DUP @ C@ E9   =  IF EXIT THEN \ JMP
     DUP @ C@ F0
          AND 70   =  IF EXIT THEN \ Jx
     DUP @ C@ EB   =  IF EXIT THEN
     DUP @ W@ F0FF
          AND 800F =  IF EXIT THEN
     2DUP @ 2+ C@  =  IF EXIT THEN
    CELL+   DUP @  :-SET   U> 0=
  UNTIL ;
1 [IF]
:  -EBPCLR   ( FLAG OPX  -- FLAG' )
   DUP @  :-SET  U< IF DROP EXIT THEN
  OFF-EBP CELL- TO OFF-EBP
  BEGIN
          ?OPlast    IF DROP EXIT THEN
     DUP @ W@ 6D8D = IF DROP EXIT THEN
     DUP @ C@ E8   = IF DROP EXIT THEN \ CALL
     DUP @ C@ E9   = IF DROP EXIT THEN \ JMP
     DUP @ C@ F0
          AND 70   = IF DROP EXIT THEN \ Jx
     DUP @ C@ EB   = IF DROP EXIT THEN
     DUP @ W@ F0FF
          AND 800F = IF DROP EXIT THEN
     DUP @ @ FFFFFF AND 85448B = \ MOV  EAX , FC [EBP] [EAX*4]
              IF DROP EXIT THEN

\     DUP @  2+ C@ DUP U. OFF-EBP FF AND DUP U. =
     DUP @ 2+ C@ OFF-EBP FF AND =
     IF   DUP  @ W@  E7FF AND  4589 =    \ MOV X [EBP] , EAX|EDX|EBX|ECX
          OVER @ W@            45C7 = OR \ MOV     F8 [EBP] , # 2710
          IF M\ 20A DTST
\ ." ^"   DUP @ U. :-SET U. \  OPexcise NIP TRUE SWAP CELL-
               DUP  OPexcise NIP TRUE SWAP CELL-
          ELSE DROP EXIT
          THEN
     THEN
    CELL+   DUP @  :-SET  U<
  UNTIL DROP ;

[ELSE]
:  -EBPCLR   ( FLAG OPX  -- FLAG' )
  OFF-EBP CELL-  TO OFF-EBP
   BEGIN
       OFF-EBP  SWAP  -EBPLIT NIP
    ?OPlast    IF DROP EXIT THEN
   :-SET  OVER  @   U<
   WHILE  DUP @ 2+ C@  OFF-EBP FF AND =
          IF  DUP @ W@  E7FF AND  4589  =  \ MOV X [EBP] , EAX|EDX|EBX|ECX
              IF M\ 20A DTST
                DUP  OPexcise NIP TRUE SWAP \ CELL-
              ELSE DROP EXIT
              THEN
          ELSE
    DUP @ @ FFFFFF AND 85448B = \ MOV  EAX , FC [EBP] [EAX*4]
              IF DROP EXIT THEN
              CELL+
                     ?OPlast    IF DROP EXIT THEN
               DUP @  :-SET  U< IF DROP EXIT THEN
          THEN
   REPEAT  DROP
;
[THEN]

:  T?EAX>ECX   (  FALSE | OPX' TRUE -- ... )
   IF  CELL+ CELL+ FALSE
   ELSE FALSE TRUE
   THEN ;

: F?EAX>ECX ( FLAG -- ... )
   IF \   TRUE TRUE
       ?~EAX @
       IF   DROP FALSE
       ELSE TRUE
       THEN TRUE
   ELSE FALSE
   THEN ;
: ECX:=EAX ( OPX -- OPX- )
     DUP 2 OPinsert
     C88B OVER CELL+ @ W! \ MOV     ECX , EAX
\     CELL-
;

: EBX:=EAX ( OPX -- OPX- )
     DUP 2 OPinsert
     D88B OVER CELL+ @ W! \ MOV     EBX , EAX
\     CELL-
;

TRUE VALUE  ~BR-OPT

: ?EAX:=ECX  ( -- )
   ?~EAX @
   IF SetOP 8B C, C1 C, \ MOV     EAX , ECX
   THEN  ;
: DO_EAX>ECX ( -- FLAG )
      OP1
      BEGIN ?EAX>ECX
         DUP  0= \ TTTT AND
         IF   DROP CELL-
              ?OPlast     IF DROP FALSE EXIT THEN
           DUP @ :-SET U< IF DROP FALSE EXIT THEN
              DUP @  W@  4D89 =  \ OPX N F  MOV     FC [EBP] , ECX
              IF   DUP @ 2+ C@
                   OP0 @ 2+ C@ =
                       IF    M\ 410 DTST
                           DROP    ?~EAX 0!
                           OP0 2 OPinsert
                           C18B OP1 @ W! \ MOV     EAX , ECX
                           OP0 TRUE TRUE
                       ELSE DROP  FALSE TRUE
                       THEN
              ELSE CELL+ FALSE
              THEN
         THEN
      UNTIL DUP >R
      IF   M\ 40E DTST
         ?~EAX @
         IF    ECX:=EAX
         ELSE  BEGIN EAX>ECX0 UNTIL
         THEN  BEGIN EAX>ECX  UNTIL
         DROP
         OP1 ToOP0
         -3 ALLOT
         M\ 40F DTST
      THEN R>
;
M\  VARIABLE VPPP : PPPP DUP VPPP @ <> IF 0 @ THEN ;

VECT FPOP

: ?EAX=RULES ( ADDR  -- ADDR' FLAG )

   BEGIN  OP1 @ :-SET
\ TTTT  IF ." T=" 2DUP HEX U. U. THEN
 U< IF TRUE EXIT THEN
          OP1 @ ?ChEAX 0=
          ~BR-OPT AND
\          OP1 @ W@ ADD|XOR|OR|AND= OR
   WHILE   M\ 0 DTST
           OP1 OPexcise
           M\ 1 DTST
   REPEAT
   OP1 @ @ FC458B58 = \ POP     EAX   MOV     EAX , FC [EBP]
   IF      M\ F0 DTST
           0424648D OP1 @ !
           OP1 ToOP0
           FALSE    M\ F1 DTST
           EXIT
   THEN
 M\ PPPP

   OP2 @ :-SET U< 0= IF  \ GOTO OP2>
   OP2 @ C@ B8 =    \  MOV     EAX , # 44444
   IF
     OP1 @ @ FFFFFF AND 240401 =  \ ADD     [ESP] , EAX
     IF   M\ B0E DTST
        OP2 @ 1+ @
        240481  OP2 @ !	  \  ADD  [ESP] , #
        OP2 @ 3 + !
        2 OP1  +!
        OP0 @ 2@ OP1 @ 2!
        OP1 ToOP0
        FALSE  -1 ALLOT M\ B0F DTST
        EXIT
     THEN
 M\ PPPP
     OP1 @ W@ 00C7 =  \ MOV     [EAX] , # X
     IF   M\ C0E DTST
                   OP1 @ 2+ @
        OP2 @ 1+ @
        05C7  OP2 @ W!	  \  MOV  44444 , # X
        OP2 @ 2+ ! OP2 @ 6 + !
        5 OP1  +!
        OP0 @ 2@ OP1 @ 2!
        OP1 ToOP0
        FALSE  -1 ALLOT M\ C0F DTST
        EXIT
     THEN
   THEN
 M\ PPPP

   OP2 @ ?ChEAX
   OP1 @ W@ 5589 XOR        \ 8955FC            MOV     FC [EBP] , EDX
   OP1 @ W@ 1501 <> AND     \ 011544444400      ADD     444444 , EDX
   OP1 @ W@ 1589 <> AND  OR \ 891544444400      MOV     444444 , EDX
\ EAX=
0=   IF  M\ C2 DTST
       OP2 OPexcise
       FALSE  M\ C3 DTST
       EXIT
     THEN
              THEN   \ LAB: OP2>

   OP0 @  W@  458B = \ MOV     EAX , X [EBP]
   IF
 OP1  \ ." $"
      BEGIN  XX_STEP
      UNTIL
      IF  M\ 90E DTST
         OP1 ToOP0 -3 ALLOT FALSE   M\ 90F DTST
         EXIT
      THEN
      DO_EAX>ECX IF FALSE EXIT THEN
      TRUE ?~EAX !
      OP1
      BEGIN ?EAX>EBX
      UNTIL
      IF   M\ 80E DTST
         ?~EAX @
         IF    EBX:=EAX
         ELSE  BEGIN EAX>EBX0 UNTIL
         THEN  BEGIN EAX>EBX  UNTIL
           DROP
           OP1 ToOP0
           FALSE
           -3 ALLOT  M\ 80F DTST
           EXIT
       THEN

        :-SET OP2 @ U< IF   \ GOTO OP2 MOV     EAX , X [EBP]


OP2 @ W@ 5589 XOR    \ 8955FC		MOV     FC [EBP] , EDX
OP1 @ W@  889 XOR OR \ 8908		MOV     [EAX] , ECX
\ MOV     EAX , X [EBP]
OP2 @  2+ C@ OP0 @  2+  C@ XOR OR  \   X2=X0
0=     IF M\ 11E DTST
          C28B  OP0 @ W!          \ MOV     EAX , EDX
          -1 ALLOT
          FALSE M\ 11F DTST
          EXIT
       THEN
\ $ DUP >R
       OP2 @  W@ 4589 XOR           \  MOV     X2 [EBP] , EAX
       OP1 @  C@ 50   XOR       OR  \  PUSH    EAX
\      OP0 @  W@ 458B XOR       OR  \  MOV     EAX , X0 [EBP]
       OP2 @  2+ C@ OP0 @  2+  C@ XOR OR 0=  \   X2=X0
       IF M\ 1E DTST
          50 OP2 @ C!
          OP2 ToOP0
          -6 ALLOT
          FALSE M\ 1F DTST
          EXIT
       THEN

  DUP C@    C3 XOR
OP2 @ C@    BA XOR OR \ MOV     EDX , # 1000
OP1 @ W@  1089 XOR OR \	MOV     [EAX] , EDX
\ MOV     EAX , FC [EBP]
       0= IF  M\ 52 DTST
             OP2 1 OPresize
             00C7 OP2 @ W!  \ MOV     [EAX] , # 1000
             OP1 OPexcise
             FALSE  M\ 53 DTST
             EXIT
          THEN

        :-SET OP4 @ U< IF   \ GOTO OP4 MOV     EAX , X [EBP]

       DUP   C@  0C3 XOR
       \ CR ." XX"
       OP4 @ W@ 4D89 XOR OR \ 567F5A 894DF8		MOV     F8 [EBP] , ECX
       OP3 @ W@ 1089 XOR OR \ 567F5D 8910		MOV     [EAX] , EDX
       OP2 @ W@ 558B XOR OR \ 567F5F 8B55F8		MOV     EDX , F8 [EBP]
       OP1 @ W@ 5089 XOR OR \ 567F62 895004		MOV     4 [EAX] , EDX
\      OP0 @ W@ 458B XOR OR \ 567F65 8B45FC		MOV     EAX , FC [EBP]
       OP4 @ 2+ C@ OP2 @ 2+ C@ XOR OR  \  X0=X2  \ ГРУБО
       0= IF  M\ 232 DTST
             OP2 OPexcise
             4889 OP1 @ W!  \ MOV     [EAX+4] , ECX
             FALSE  M\ 233 DTST
             EXIT
          THEN
   THEN     \ L: OP4 MOV     EAX , X [EBP]
   THEN     \ L: OP2 MOV     EAX , X [EBP]
   THEN
DUP C@ C3 XOR
OP0 @  C@  A1 XOR OR \ MOV     EAX , X
0= IF  OP1
      BEGIN   ?EAX>ECX
         IF  T?EAX>ECX
         ELSE   CELL-  ?OPlast >R
                DUP @ :-SET U< R> OR
           IF   DROP FALSE TRUE
           ELSE DUP >R
                R@ CELL- @  CELL- @
                OP0 @ 1+ @ =
                IF R@  @   C@ FD AND A1 =  \	MOV    { EAX , X } | { X ,EAX }
                     DUP
                     IF  F?EAX>ECX
                     ELSE 2DROP FALSE TRUE
                     THEN
                ELSE  DROP FALSE TRUE
                THEN  RDROP
            THEN
         THEN
      UNTIL
      IF   M\ F0E DTST
           CELL- \ DROP TRUE EXIT
           BEGIN EAX>ECX0 UNTIL
           BEGIN EAX>ECX  UNTIL
           DROP \ ." #"
           OP1 ToOP0
           FALSE
           -5 ALLOT  M\ F0F DTST
           EXIT
       THEN

   THEN
 M\ PPPP
DUP C@ C3 XOR
OP0 @  C@  58 XOR OR \ POP     EAX
0= IF   OP1 \   TTTT IF ."  ?EAX>ECX" THEN
      BEGIN   ?EAX>ECX
         IF  T?EAX>ECX
         ELSE   CELL-
		?OPlast
                OVER @ :-SET U< OR
                OVER @ @ FF00FF AND 24008B = OR \  8B0424   MOV   ___ , [ESP]
           IF   DROP  FALSE TRUE
           ELSE DUP  @   C@  50 XOR    \	PUSH     EAX , X
\           0=  F?EAX>ECX
            0=  IF   TRUE TRUE
                ELSE CELL+ FALSE
                THEN

            THEN
         THEN
      UNTIL
      IF   M\ CA DTST  \ ." $"
           DUP OPexcise
           CELL- \ DROP TRUE EXIT
         ?~EAX @
         IF    ECX:=EAX
         ELSE  BEGIN EAX>ECX0 UNTIL
         THEN  BEGIN EAX>ECX  UNTIL
           DROP
         OP1 ToOP0
           FALSE
           -1 ALLOT
\            ?EAX:=ECX
  M\ CB DTST
           EXIT
       THEN

   THEN

OP1 @ @ 24048B50 = \  50      PUSH    EAX
                   \  8B0424  MOV     EAX , [ESP]
   IF  M\ E2 DTST
       OP0 OPexcise
       FALSE  M\ E3 DTST
       EXIT
   THEN

DUP C@ C3 XOR
OP1 @ W@ D08B XOR OR \ 8BD0              MOV     EDX , EAX
OP0 @ @ FFFFFF AND 95048B XOR OR \ 8B0495F0065A00        MOV     EAX , 5A06F0 [EDX*4]
0= IF  M\ 13C DTST
         OP1 OPexcise
       85  OP0 @ 2+ C!      \ MOV     EAX , 5982D8 [EAX*4]

       FALSE  M\ 13D DTST
       EXIT
   THEN

OP1 @ 2+  @
OP0 @ 1+  @  XOR
OP1 @ W@ 589 XOR OR \  890577770700       MOV     77777 , EAX
OP0 @ C@ A1  XOR OR \  A177770700  MOV     EAX , 77777
0= IF  M\ 150 DTST
       OP0 OPexcise
       FALSE  M\ 151 DTST
       EXIT
   THEN


OP2 @ :-SET U< IF TRUE EXIT THEN M\ PPPP

   OP1 @  C@ A3 =  \ MOV ' VVVV >BODY ,  EAX
   IF   OP2 @  C@ B8 =       \ MOV  EAX, # X
     IF      M\ 20E DTST
        OP2 @ 1+ @    OP1 @ 1+ @
        OP2 @ 2+ !    OP1 @ 1+ !
        05C7 OP2 @ W!
        OP0 @  OP1 !
        FALSE
        OP1  ToOP0  M\ 20F DTST
        EXIT
     THEN
        OP2 @  W@ C033 =  \ XOR     EAX , EAX
     IF      M\ 30E DTST
        3 ALLOT
        OP0 @ @ OP0 @ 3 + !
        05C7 OP2 @ W!
        OP1 @ 1+ @ OP1 @ !
        OP1 @ CELL+ 0!
        OP1 @ 08 + OP1 !
        FALSE
        OP1  ToOP0  M\ 30F DTST
        EXIT
     THEN
           OP3 @ :-SET U< IF TRUE EXIT THEN  M\ PPPP
     OP3 @ 1+ @
     OP1 @ 1+ @   XOR
     OP3 @ C@ 0A1 XOR  OR   \ MOV   EAX , XX \ MOV  XX ,  EAX \ EAX=
0=   IF
      OP2 @ C@ 025 =     \ AND     EAX , # ZZ
       IF
         M\ 530 DTST
          OP3 @ 1+ @
          OP2 @ 1+ @
          OP3 OPexcise
          OP2 OPexcise
          OP1 OPexcise
          OP0 0A OPinsert
           2581  OP1 @ !  OP1 @ 6 + !  OP1 @ 2+ !
          FALSE  M\ 531 DTST
          EXIT
       THEN
       OP2 @ W@ 408D =
       IF
          OP2 @ 2+ C@ FF =  \ LEA     EAX , FF [EAX]
         IF
           M\ 632 DTST
            OP3 01 OPresize
            0DFF OP3 @ W!
            OP2 OPexcise
            OP1 OPexcise
            FALSE M\ 633 DTST
            EXIT
         THEN
       THEN
     THEN   TRUE EXIT
   THEN

 \ $ 4444 TO VVVV

    OP2 @ C@ A1 XOR     \ MOV     EAX , 44444
    OP1 @ C@ 3D XOR OR  \ CMP     EAX , # 55555
0= IF M\ 218 DTST
       OP2 @ 1+ @ OP2 @ 2+ !
       3D81 OP2 @ W!         \ CMP  44444 , # 55555
       OP0 @ OP1 !
       OP1 ToOP0
       FALSE
       EXIT M\ 219 DTST
   THEN

\   OP0 @ C@ 58 =    \ POP EAX
\  IF
   OP1 @  C@ 50 =   \ PUSH EAX
   IF
     \  444 >R
       OP2 @  C@ B8 = \ MOV EAX , # 5
       IF    M\ 1A DTST
             68 OP2 @ C!
             OP0 @ 2@ OP1 @ 2!
             OP1 ToOP0
           FALSE  -1 ALLOT M\ 1B DTST
           EXIT
       THEN
   THEN
   OP1 @  W@ 4589 =  \ MOV X [EBP], EAX   EAX=
   IF
     \    444 555
       OP2 @  C@ B8 =  \ MOV EAX ,  5
       IF    M\ 1C DTST
             OP2 @ 1+ @
                   45C7   OP2 @    W!
              OP1 @ 2+ C@ OP2 @ 2+ C!
                          OP2 @ 3 + !
             2 OP1  +!
             OP0 @ 2@ OP1 @ 2!
             OP1 ToOP0
           FALSE  -1 ALLOT M\ 1D DTST
           EXIT
       THEN
   THEN
\  THEN
M\ PPPP
OP2 @  C@   B8 XOR    \	MOV     EAX , # 789
OP1 @  W@ C88B XOR OR \	MOV     ECX , EAX
\ EAX=
0= IF   M\ 830 DTST
       B9 OP2 @ C!
       OP1 OPexcise
       FALSE  M\ 831 DTST
       EXIT
   THEN

OP2 @  W@ 878D XOR    \ LEA     EAX , 1448 [EDI]
OP1 @  W@ 00FF XOR OR \ INC     [EAX]
\ EAX=
0= IF   M\ 930 DTST
       FF OP2 @ C!
       OP1 OPexcise
       FALSE  M\ 931 DTST
       EXIT
   THEN
OP2 @ @ FFFFFF AND 8D048D XOR \     LEA     EAX , X [ECX*4]
OP1 @ W@ 1089 XOR OR \    MOV     [EAX] , EDX
\ EAX=
0= IF  M\ 5E DTST
       1489 OP2 @ W!  \   MOV   [44444H+ECX*4], EDX
       OP1 OPexcise
       FALSE  M\ 5F DTST
       EXIT
   THEN
M\ PPPP
 DUP C@   0C3 XOR
OP2 @ W@ D08B XOR OR  \     MOV     EDX , EAX
OP1 @ @ FFFFFF AND 8D1489 XOR OR \     MOV   X [ECX*4] , EDX
\ EAX=
0= IF  M\ 60 DTST
       OP2 OPexcise
       0489 OP1 @ W!  \   MOV   [44444H+ECX*4], EAX
       FALSE  M\ 61 DTST
       EXIT
   THEN

OP2 @ 2+ C@
OP0 @ 2+ C@   XOR
OP2 @ W@ 5589 XOR OR \ MOV     F8 [EBP] , EDX
OP1 @ W@ 4589 XOR OR \ MOV     F4 [EBP] , EAX
OP0 @ W@ 458B XOR OR \ MOV     EAX , F8 [EBP]
0= IF  M\ 64 DTST
       C28B  OP0 @ W!  \   MOV     EAX , EDX
       -1 ALLOT
       FALSE  M\ 65 DTST
       EXIT
   THEN
OP2 @ W@ C033 XOR  \ 57DD80 33C0              XOR     EAX , EAX
OP1 @ W@ 4589 XOR OR \ 57DD82 8945F8            MOV     F8 [EBP] , EAX
\ EAX=
0= IF  M\ 6C DTST
       OP2 OPexcise
       OP0 4 OPresize 4 OP0 +!
       45C7  OP1 @ W! OP1 @ 3 + 0!  \  MOV     F8 [EBP] , # 0
       FALSE  M\ 6D DTST
       EXIT
   THEN
OP2 @  @ 45C7D08B XOR  \     MOV     EDX , EAX  MOV     F8 [EBP] , # X
OP0 @ W@     C28B XOR OR \   MOV     EAX , EDX
0= IF  M\ 6E DTST
       OP0 OPexcise
       FALSE  M\ 6F DTST
       EXIT
   THEN

  DUP C@   C3 XOR
OP2 @ C@   BA XOR OR \  MOV     EDX , # 0
OP1 @ W@ 1088 XOR OR \  MOV     [EAX] , DL
\ EAX=
0= IF  M\ 70 DTST
       OP2 @ 1+ C@
       OP2  OPexcise
       OP1 1 OPresize
       00C6  OP1 @ W!    \  MOV   BYTE PTR  [EAX] , 44H
       OP1 @ 2+ C!
       FALSE  M\ 71 DTST
       EXIT
   THEN
OP2 @ W@ 808D XOR  \   LEA     EAX , X [EAX]
OP1 @ W@ 00C6 XOR OR \   MOV     [EAX] , # 0
\ EAX=
0= IF  M\ 72 DTST
       80C6 OP2 @ W!
       OP1 @ 2+ C@ OP1 @ C!
       1 OP1 +!
       OP1 OPexcise
       FALSE  M\ 73 DTST
       EXIT
   THEN

  DUP C@   C3 XOR
OP2 @ W@ D18B XOR OR \ 596C15 8BD1              MOV     EDX , ECX
OP1 @ W@ 1089 XOR OR \ 596C17 8910              MOV     [EAX] , EDX
\ EAX=
0= IF  M\ 72 DTST
       OP2 OPexcise
       0889 OP1 @ W!
       FALSE  M\ 73 DTST
       EXIT
   THEN

OP2 @ @ FFFFFF AND 24448B XOR  \   MOV     EAX , 30 [ESP]
OP1 @ W@  F7FF AND 44FF XOR OR \ FF4424  INC DEC    30 [ESP]
\ EAX=
0= IF  M\ 8A DTST
       OP2 OPexcise
       FALSE  M\ 8B DTST
       EXIT
   THEN

OP2 @ C@ B8 XOR \   MOV     EAX , # 1
OP1 @ @ FFFFFF AND 21C8D XOR OR \         LEA     EBX , [EDX] [EAX]
\ EAX=
0= IF  M\ AA DTST
       OP2 1 OPresize
       9A8D   OP2 @ W!
       OP1 OPexcise
       FALSE  M\ AB DTST
       EXIT
   THEN
OP2 @ C@ BA XOR  \  MOV     EDX , # 7FFFFFEE
OP1 @ W@ 9A8D XOR OR \   LEA     EBX , 1 [EDX]
\ EAX=
0= IF  M\ AC DTST
       OP1 @ 2+ @
       OP1 OPexcise
       OP0 5 OPinsert
       BB OP1 @ C!        \     MOV     EBX , #
       OP2 @ 1+ @ +  OP1 @ 1+ !
       FALSE  M\ AD DTST
       EXIT
   THEN

OP2 @ W@ C28B XOR       \  8BC2        MOV     EAX , EDX
OP1 @ @ C28BC00B XOR OR \  0BC0        OR      EAX , EAX
                        \  8BC2        MOV     EAX , EDX
0= IF  M\ C4 DTST
       OP2 OPexcise
       C28BD20B   OP1 @ !    \  OR      EDX , EDX   MOV     EAX , EDX
       FALSE  M\ C5 DTST
       EXIT
   THEN

 DUP C@    C3 XOR
OP2 @ W@ D18B XOR OR \ 58270A 8BD1              MOV     EDX , ECX
OP1 @ W@ 4589 XOR OR \ 58270C 8945F8            MOV     F8 [EBP] , EAX
OP0 @ W@ C28B XOR OR \ 58270F 8BC2              MOV     EAX , EDX
0= IF  M\ D4 DTST
       OP2 OPexcise
       C18B  OP0 @ W!    \  MOV     EAX , ECX
       FALSE  M\ D5 DTST
       EXIT
   THEN

DUP C@ C3 XOR
                             \ 8BD0            MOV     EDX , EAX
OP2 @ @ 501D08B XOR          \ 010544440400    ADD     44444 , EAX
OP2 @ @ 589D08B <>  AND OR   \ 890544440400    MOV     44444 , EAX
OP0 @ W@ 458B XOR OR         \ 8B4500          MOV     EAX , 0 [EBP]
0= IF  M\ D4 DTST
       OP2 OPexcise
       FALSE  M\ D5 DTST
       EXIT
   THEN

OP2 @ @ 1589D08B = \  8BD0           MOV     EDX , EAX
                   \  891555550500      MOV     55555 , EDX
   IF  M\ E8 DTST
       0589 OP1 @ W!        \  MOV  5800E0  , EAX
       FALSE  M\ E9 DTST
       EXIT
   THEN

OP2 @ W@ 408D XOR       \  8D4020         LEA     EAX , 20 [EAX]
OP1 @ @ 458BC88B XOR OR \  8BC8           MOV     ECX , EAX
                        \  8B45F8         MOV     EAX , F8 [EBP]
0= IF  M\ F2 DTST
       488D OP2 @ W!        \  LEA     ECX , 20 [EAX]
       OP1 OPexcise
       FALSE  M\ F3 DTST
       EXIT
   THEN

  OP2 @ ?ChEAX
  OP2 @ ^?EAX=  AND
OP1 @ C@ B9 XOR OR \ 587AC6 B919000000  MOV     ECX , # 19
\ EAX=
0= IF  M\ FE DTST
       OP2 OPexcise
       FALSE  M\ FF DTST
       EXIT
   THEN

OP2 @ 2+ C@
OP0 @ 2+ C@   XOR
OP2 @ W@ 5589 XOR OR \  8955E8            MOV     E8 [EBP] , EDX

OP1 ?EDX_[EBP] OR

OP0 @ W@ 458B XOR OR \  8B45E8            MOV     EAX , E8 [EBP]
0= IF  M\ BC DTST
       C28B   OP0 @ W!    \  MOV     EAX , EDX
       -1 ALLOT
       FALSE  M\ BD DTST
       EXIT
   THEN

DUP C@ C3 XOR
OP2 @ @ 9788D08B XOR OR \  8BD0           MOV     EDX , EAX
                        \  88973C100000      MOV     103C [EDI] , DL
\ EAX=
0= IF  M\ 112 DTST
       OP2 OPexcise
       8788   OP1 @ W!      \ MOV     103C [EDI] , AL
       FALSE  M\ 113 DTST
       EXIT
   THEN

OP2 @ C@ 50 XOR      \  50          PUSH    EAX
OP1 @ W@ 4589 XOR OR \  8945FC            MOV     FC [EBP] , EAX
OP0 @ W@ 48B XOR OR  \  8B0424             MOV     EAX , [ESP]
0= IF  M\ 164 DTST
       OP0 OPexcise
       FALSE  M\ 165 DTST
       EXIT
   THEN

M\ PPPP
OP3 @ :-SET U< IF TRUE EXIT THEN

\ $ SWAP 4444
   OP2 @ W@ C28B =    \ MOV     EAX , EDX
   IF  OP1 @ W@ 4589 =   \ MOV   X [EBP] , EAX
     IF   M\ 60E DTST
        1000  OP1 @ +!	  \  MOV    X [EBP] , EDX
        FALSE  OP2 OPexcise  M\ 60F DTST
          EXIT
     THEN
   THEN

OP3 @  @ 408DC48B XOR  \     MOV     EAX , ESP   LEA     EAX , 8 [EAX]
OP1 @ W@ E08B XOR OR   \     MOV     ESP , EAX
0= IF  M\ 8A DTST
       -1 OP2 +!
       24648D00  OP2 @ 1- !  \ LEA     ESP , 8 [ESP]
       OP3 OPexcise
       OP1 OPexcise
       FALSE  M\ 8B DTST
       EXIT
   THEN

OP3 @ W@ C88B = \ 581B54 8BC8              MOV     ECX , EAX
IF
      DUP C@ C3 XOR
\      OP3 @ W@ C88B XOR OR \ 587733 8BC8              MOV     ECX , EAX
      OP2 @ W@ 428D XOR OR \ 587735 8D4201            LEA     EAX , 1 [EDX]
      OP1 @ W@ 4589 XOR OR \ 587738 894500            MOV     0 [EBP] , EAX
      OP0 @ W@ C18B XOR OR \ 58773B 8BC1              MOV     EAX , ECX
      0= IF   M\ 142 DTST
             OP3 OPexcise
             OP0 OPexcise
             528D OP1 @ W!  \  LEA     EDX , 1 [EDX]
             5589 OP0 @ W!  \  MOV     0 [EBP] , EDX
             FALSE   M\ 143 DTST
             EXIT
         THEN
      OP5 @ J-SET  U< 0= IF   \ GOTO 5SET
      OP5 @ W@ C18B XOR    \ 581B4C 8BC1              MOV     EAX , ECX
      OP4 @ W@ 840F XOR OR \ 581B4E 0F8400000000      JE      581B54  ( zz+14  )
      0= IF  M\ B0 DTST
             OP3 OPexcise
             FALSE M\ B1 DTST
             EXIT
      THEN               THEN  \ 5SET:
THEN

 DUP C@  0C3 XOR
OP3 @ @ C103D8F7 XOR OR \  F7D8     NEG     EAX
                        \ 03C1      ADD     EAX , ECX
OP1 @ W@ 4589 XOR OR    \  894500      MOV     0 [EBP] , EAX
OP0 @ W@ C28B XOR OR    \  8BC2	MOV     EAX , EDX
0= IF   M\ CE DTST
       OP3 OPexcise
       C82B  OP2 @ W!  \   SUB    ECX , EAX
       4D89  OP1 @ W! \      MOV     0 [EBP] , ECX
       FALSE   M\ CF DTST
       EXIT
   THEN

OP3 @ @ C82BD08B XOR    \ 8BD0     MOV     EDX , EAX
                        \ 2BC8     SUB     ECX , EAX
OP1 @ W@ 4D89 XOR OR    \ 894D00   MOV     0 [EBP] , ECX
OP0 @ W@ C28B XOR OR    \ 8BC2     MOV     EAX , EDX
0= IF   M\ D0 DTST
         DUP C@  0C3 =
       IF  OP3 OPexcise THEN
           OP0 OPexcise
       FALSE   M\ D1 DTST
       EXIT
   THEN

DUP C@    C3 XOR
OP3 @ @ 418DD08B XOR OR \ 8BD0          MOV     EDX , EAX
                        \ 8D41FF        LEA     EAX , FF [ECX]
OP1 @ W@ 4589 XOR OR    \ 8945F8        MOV     F8 [EBP] , EAX
OP0 @ W@ C28B XOR OR    \ 8BC2          MOV     EAX , EDX
0= IF   M\ EA DTST
       OP3 OPexcise
       OP0 OPexcise
       498D  OP1 @ W!  \        LEA     ECX , FF [ECX]
       4D89  OP0 @ W!  \        MOV     F8 [EBP] , ECX
       FALSE   M\ EB DTST
       EXIT
   THEN

OP3 @ 2+ C@
OP0 @ 2+ C@   XOR
OP3 @ W@ 5589 XOR OR \  8955E8            MOV     E8 [EBP] , EDX

OP2 ?EDX_[EBP] OR
OP1 ?EDX_[EBP] OR

OP0 @ W@ 458B XOR OR \  8B45E8            MOV     EAX , E8 [EBP]
0= IF  M\ BC DTST
       C28B   OP0 @ W!    \  MOV     EAX , EDX
       -1 ALLOT
       FALSE  M\ BD DTST
       EXIT
   THEN

DUP C@ C3 XOR
OP2 @ C@ BA XOR OR      \ BA2C000000  MOV     EDX , # 2C
OP1 @ @ 458B1001 XOR OR \ 0110        ADD     [EAX] , EDX
                        \ 8B45FC      MOV     EAX , FC [EBP]
0= IF  M\ 14C DTST
       OP1 OPexcise
       OP1 1 OPresize
       0081   OP1 @ W!    \  ADD     [EAX] , # 2C
       FALSE  M\ 14D DTST
       EXIT
   THEN
OP2 @ C@ B8 XOR  \ 57DDE3 B844440400  MOV     EAX , # 44444
OP1 @ W@ 81 XOR OR \ 57DDE8 81002C000000        ADD     [EAX] , # 2C
OP0 @ W@ 458B XOR OR \ 57DDEE 8B45FC            MOV     EAX , FC [EBP]
0= IF  M\ 14E DTST
       OP2 @ 1+ @ OP2 @ 2+ !
       OP1 @ 2+ @ OP1 @ 1+ !
       0581   OP2 @ W!    \  ADD     44444 , # 2C
       5 OP1 +!
       OP1 OPexcise
       FALSE  M\ 14F DTST
       EXIT
   THEN

OP2 @ 2+ C@
OP0 @ 2+ C@     XOR
OP2 @ W@ 45C7	XOR OR	\    MOV     F8 [EBP] , # 3
OP1 @ C@ B9	XOR OR	\    MOV     ECX , # 4
OP0 @ W@ 458B	XOR OR	\    MOV     EAX , F8 [EBP]
0= IF  M\ 3E DTST
	OP2 @ 3 + @ OP2 @ 1+ !
	B8  OP2 @ C!
	OP1 0 OPinsert  -2 OP2 +!
	OP2 OPexcise		\    MOV     EAX , # 3
	4589 OP0 @ W!		\    MOV     F8 [EBP] , EAX

       FALSE  M\ 3F DTST
       EXIT
   THEN


OP3 @ :-SET U< IF TRUE EXIT THEN

OP3 @ C@ A1 XOR  \ 57E2D7 A108E15700  MOV     EAX , 57E108
OP2 @ W@ 4589 XOR OR \ 57E2DC 8945EC            MOV     EC [EBP] , EAX
OP1 @ W@ D08B XOR OR \ 57E2DF 8BD0              MOV     EDX , EAX
\ EAX=
0= IF  M\ 152 DTST
       OP3 1 OPresize
       158B   OP3 @ W!    \  MOV     EDX , 57E108
       5589   OP2 @ W!    \  MOV     EC [EBP] , EDX
       OP1 OPexcise
       FALSE  M\ 153 DTST
       EXIT
   THEN


OP3 @ C@ 58 XOR  \ 	POP     EAX
OP2 @ @ FFFFFF AND FF408D XOR OR \  LEA     EAX , FF [EAX]
OP1 @ C@ 50 XOR OR \ 	PUSH    EAX
\ EAX=
0=  IF   M\ D30 DTST
\       OP2 1 OPresize
       240CFF OP2 @ !  \  DEC DWORD PTR  [ESP]
       OP3 OPexcise
       OP1 OPexcise
       FALSE   M\ D31 DTST
       EXIT
    THEN

DUP C@ C3 XOR
OP3 @ @ 458BD08B XOR OR \  8BD0           MOV     EDX , EAX  MOV     EAX , 0 [EBP]
OP1 @ W@ 4589 XOR OR \  8945FC            MOV     FC [EBP] , EAX
OP0 @ W@ C28B XOR OR \  8BC2              MOV     EAX , EDX
0=  IF  M\ 166 DTST
       OP3 OPexcise
       OP0 OPexcise
	558B OP1 @ W! \  MOV     EDX , 0 [EBP]
	5589 OP0 @ W! \  MOV     FC [EBP] , EAX
       FALSE  M\ 167 DTST
       EXIT
   THEN

DUP C@ C3 XOR
OP3 @ W@ D08B XOR OR \ 5822F8 8BD0              MOV     EDX , EAX
OP2 @ W@ 458B XOR OR \ 5822FA 8B4508            MOV     EAX , 8 [EBP]
OP1 @ W@ 1089 XOR OR \ 5822FD 8910              MOV     [EAX] , EDX
OP0 @ W@ 458B XOR OR \ 5822FF 8B450C            MOV     EAX , C [EBP]
0= IF  M\ 156 DTST
       OP3 OPexcise
       558B  OP2 @ W!     \ MOV     EDX , 8 [EBP]
       0289  OP1 @ W!     \ MOV     [EDX] , EAX
       FALSE  M\ 157 DTST
       EXIT
   THEN



OP4 @ :-SET U< IF TRUE EXIT THEN
OP5 @ :-SET U< IF TRUE EXIT THEN

OP5 @ C@ B8 XOR  \ 596994 B812000000  MOV     EAX , # 12
OP4 @ W@ 4589 XOR OR \ 596999 8945F4            MOV     F4 [EBP] , EAX
OP3 @ W@ 4589 XOR OR \ 59699C 8945F0            MOV     F0 [EBP] , EAX
OP2 @ C@ BA XOR OR \ 59699F BA00000080  MOV     EDX , # 80000000
OP1 @ W@ D02B XOR OR \ 5969A4 2BD0              SUB     EDX , EAX
\ EAX=
0=  IF  M\ A6 DTST
       OP5 @ 1+ @ NEGATE OP2 @ 1+ +!
       OP1 OPexcise
       FALSE  M\ A7 DTST
       EXIT
   THEN

  TRUE
;


\ : SSSSSS ;
\ TRUE VALUE ?C-JMP
 FALSE VALUE ?C-JMP

\ $ - указывает на фрагмент исходного текста, оптимизируемый
\ данным методом

: OPT-RULES  ( ADDR  -- ADDR' FLAG )

   BEGIN M\ -1 DTST
   OP0 @ :-SET U< IF TRUE EXIT THEN

   OP0 @  W@  408D =  \  LEA   EAX,  X [EAX]
   WHILE  M\ 2 DTST
       OP0 @ 2+ C@ C>S OFF-EAX + TO OFF-EAX
       OP1 ToOP0
       -3 ALLOT M\ 3 DTST
   REPEAT M\  DUP VPPP !
   OP0 @ C@ 05  = ~BR-OPT AND   \ ADD  EAX, # X
   IF  M\ 4 DTST
       OP0 @ 1+ @ OFF-EAX + TO OFF-EAX
       OP1 ToOP0
       FALSE  -5 ALLOT M\ 5 DTST
       EXIT
   THEN
M\ PPPP
   OP0 @  W@  808D =  \  LEA   EAX,  X [EAX]
   IF  M\ 6 DTST
       OP0 @ 2+ @  OFF-EAX + TO OFF-EAX
       OP1 ToOP0
       -6 ALLOT FALSE M\ 7 DTST
       EXIT
   THEN
   OFF-EAX
   OP0 @  W@  C033 = AND \  XOR     EAX , EAX
   IF  M\ 4A DTST
        B8 OP0 @ C!
        OFF-EAX  OP0 @ 1+ !
        0 TO OFF-EAX
        3 ALLOT FALSE M\ 4B DTST
       EXIT
   THEN
M\ PPPP
   OP0 @ @ 3FFFFF AND
   05048D =      \ LEA  EAX, X [EAX*_]
   IF

\ $ 4444 CELLS

      OP1 @ :-SET U< 0=
      IF
           OP1 @ C@ B8 XOR     \ MOV EAX, # X1
           OP0 @ 3 + @         \ X=0
           OR 0=
           IF  M\ 8 DTST
               OP1 @ 1+ @   OP0 @ @ C00000 AND 16 RSHIFT
               LSHIFT
               OP1 @ 1+ !
               OP1 ToOP0       \ FIX
               FALSE  -7 ALLOT M\ 9 DTST
               EXIT
           THEN
      THEN

\ $ CELLS 4444 +

       OFF-EAX OP0 @ 3 + +!
       0 TO OFF-EAX

   THEN
M\ PPPP

   OP0 @  C@ B8 =    \ MOV  EAX, # X
   IF  OFF-EAX OP0 @ 1+ +!
       0 TO OFF-EAX
       OP0 @ 1+ @ 0=         \ MOV  EAX, # 0
       IF   M\ A DTST
            C033 OP0 @  W!    \ XOR EAX, EAX
           -3 ALLOT
\           FALSE   EXIT
           M\ B DTST
       THEN
       OP1 @ :-SET U< IF TRUE EXIT THEN
       DUP C@  0C3   XOR
       OP1 @  W@ 45C7 XOR OR 0=   \ MOV     F8 [EBP] , # 5
       IF  OP1 @ 2+ C@ C>S OFF-EBP U<
         IF
           M\ 10A DTST
             OP1 OPexcise
\            FALSE   EXIT
            M\ 10B DTST
         THEN
       THEN
   THEN
M\ PPPP

   DUP C@ C3 =
   IF OFF-EBP >R
       5555 DP @ !
       FALSE   OP0 -EBPCLR OP0 -EBPCLR OP0 -EBPCLR OP0 -EBPCLR
       IF   M\ 20B DTST
            R> TO OFF-EBP FALSE EXIT
       THEN R> TO OFF-EBP
   THEN

   OFF-EAX
   OP0 @  W@ C28B =         \  MOV     EAX , EDX
   OP0 @  W@ C18B = OR AND  \  MOV     EAX , ECX
   IF  M\ D6 DTST
       OFF-EAX DUP SHORT?
       IF    0418D C18B - OP0 @ +! C,
       ELSE  0828D C28B - OP0 @ +!  ,
       THEN   \  LEA   EAX,  OFF-EBP [EDX]
       0 TO OFF-EAX
       FALSE  M\ D7 DTST
       EXIT
   THEN

OP0 @ 2+ @ 5  XOR
OP0 @ W@ C069 XOR OR \ 59A095 69C005000000      IMUL    EAX , EAX , # 5
0= IF M\ 8E DTST
        80048D OP0 @ !    \ MOV     EAX , [EAX*4+EAX]
       -3 ALLOT
       FALSE  M\ 8F DTST
       EXIT
   THEN

OP0 @ W@ D28B =  \   MOV EDX, EDX  \ C-DO
IF  OP0 OPexcise TRUE EXIT
THEN

OP0 @ @ 1004583 = \ 587890 83450001                ADD     0 [EBP] , # 1
IF M\ 146 DTST
        45FF OP0 @ W!    \ INC     0 [EBP]
       -1 ALLOT
       FALSE  M\ 147 DTST
       EXIT
THEN

M\ PPPP
OP1 @ :-SET U< IF TRUE EXIT THEN
   ?~EAX 0!

OP1 @ @ 00FFFFFD AND 244489 XOR \ MOV     8 [ESP] , EAX
OP0 @ @ FFFFFFFD AND
OP1 @ @ FFFFFFFD AND  XOR OR \ MOV     EAX , 8 [ESP]
0= IF M\ 1F0 DTST
       OP1 ToOP0
       -4 ALLOT
       FALSE  M\ 1F1 DTST
       EXIT
   THEN

OP1 @ 2+  C@
OP0 @ 2+  C@  XOR
OP1 @ W@ 5089 XOR OR \ MOV     C [EAX] , EDX
OP0 @ W@ 408B XOR OR \ MOV     EAX , C [EAX]
0= IF M\ 2F0 DTST
        C28B OP0 @ W!    \ MOV     EAX , EDX
       -1 ALLOT
       FALSE  M\ 2F1 DTST
       EXIT
   THEN
M\ PPPP
OP1 @ @ 008B0889 = \ MOV     [EAX] , ECX 	MOV     EAX , [EAX]
   IF      M\ F0 DTST
           C18B OP0 @ W!
           FALSE    M\ F1 DTST
           EXIT
   THEN

   OP0 @ C@ 58 XOR   \ POP EAX
   OP0 @ ^?EAX= AND 0=
\   OP0 @ ^?EAX=  0=
   IF  ?EAX=RULES EXIT
   THEN

OP1 @ @ 4503C033 = \ XOR     EAX , EAX 	ADD     EAX , F8 [EBP]
   IF  M\ 10C DTST
       OP1 OPexcise
       8B OP0 @ C!  \  MOV     EAX , F8 [EBP]
       FALSE  M\ 10D DTST
       EXIT
   THEN
M\ PPPP
OP1 @ @ 048DC033 = \  XOR     EAX , EAX  LEA  EAX , 57B680 [EAX*4]
   IF   OP0 @ 2+ C@ 85 =
      IF  M\ 62 DTST
         OP1 OPexcise
         B8 OP0 @ C!  \  MOV     EAX , # 57B680
         OP0 @ 3 + @ OP0 @ 1+ !
         -2 ALLOT
         FALSE  M\ 63 DTST
         EXIT
      THEN
   THEN


   ?C-JMP
   IF     OP0 @ C@ C3 XOR
          OP1 @ C@ E8 XOR OR 0=    \ CALL X   RET
          IF M\ 0C DTST
             OP1 @ 1+!
             OP1 ToOP0
             FALSE  -6 ALLOT
             SetJP   5 ALLOT  M\ 0D DTST
             EXIT
          THEN
   THEN

\ TRUE EXIT

 \ $ CELLS 444 + @
   OP1 @  W@ 38FF AND 008D = \ LEA EAX , ____
   IF  OP0 @  W@ 008B   =   \ MOV EAX , [EAX]
      IF
        M\ 0E DTST
        8B OP1 @ C!
        OP1 ToOP0
        FALSE  -2 ALLOT M\ 0F DTST
        EXIT
      THEN
      OP0 @ @ FFFEFF AND 00B60F = \ MOVZX   EAX , BYTE|WORD PTR [EAX]
                                  \ LEA EAX , ____
      IF
       M\ 10E DTST
        OP0 @ W@   OP1 @ 2@ ( 2DUP  U. U. ) OP1 @ 1+ ( 0 IF THEN ) 2!
        OP1 @ W!
        OP1 ToOP0
        FALSE  -2 ALLOT M\ 10F DTST
        EXIT
      THEN
   THEN
   OP0 @  W@ 4589  XOR  \  MOV  X [EBP], EAX
   OP0 @ 2+ C@
   OP1 @ 2+ C@     XOR OR 0=   \  (FALG &( X1=X ))
    IF
          OP1 @    W@ 458B =   \  MOV  EAX, 1X [EBP] \ $ DROP DUP
          IF   M\ 10 DTST
               OP1 ToOP0
               FALSE  -3 ALLOT M\ 11 DTST
               EXIT
          THEN
    THEN

    OP1 @  C@ B8 =              \ MOV  EAX, # X \ eax=
    IF  OP0 @  W@
          DUP   D8F7   =   \ NEG EAX
       IF DROP M\ 14 DTST
          OP1 @ 1+ @ NEGATE OP1 @ 1+ !   \  MOV EAX, # -X
          OP1 ToOP0
          FALSE -2 ALLOT M\ 15 DTST
          EXIT
       THEN
          DUP   D0F7   =   \ MOV  EAX, # X   NOT EAX
       IF DROP M\ 16 DTST
          OP1 @ 1+ @ INVERT OP1 @ 1+ !   \  MOV EAX, # ~X
          OP1 ToOP0
          FALSE -2 ALLOT M\ 17  DTST
          EXIT
       THEN
          DUP    008B   =   \  MOV  EAX, # X  MOV EAX, [EAX]
       IF DROP M\ 18 DTST
          A1 OP1 @ C!   \  MOV EAX, X
          OP1 ToOP0
          FALSE -2 ALLOT M\ 19  DTST
          EXIT
       THEN
         DROP
       OP0 @  @ FFFFFF AND  85448B = \  MOV  EAX, # X  MOV EAX , X1 [EBP] [EAX*4]
       IF M\ 518 DTST
          OP0 @ 3 + C@
          OP1 @ 1+   @ CELLS +
          458B  OP1 @ W!   \  MOV     EAX , X [EBP]
                OP1 @ 2+ C!
          OP1 ToOP0
          FALSE -6 ALLOT M\ 519  DTST
          EXIT
       THEN
    THEN

    OP1 @ W@ 5589 XOR        \ MOV     X1 [EBP] , EDX
    OP0 @ W@ 4503 XOR  OR    \ ADD     EAX , X0 [EBP]
    OP1 @  2+ C@ OP0 @  2+  C@ XOR OR  \   X1=X0
0=  IF M\ 418 DTST
          02048D OP0 @ !         \  lea EAX, [EAX+EDX]
        FALSE  M\ 419  DTST
        EXIT
    THEN

    OP1 @ W@ 4D89 XOR        \ MOV     4 [EBP] , ECX
    OP0 @ W@ 558B XOR  OR    \ MOV     EDX , 4 [EBP]
    OP1 @  2+ C@ OP0 @  2+  C@ XOR OR  \   X1=X0
0= IF M\ 418 DTST
          D18B OP0 @ W!         \  MOV     EDX , ECX
        -1 ALLOT
        FALSE  M\ 419  DTST
        EXIT
   THEN
M\ PPPP
   OP1 @  W@ 408D   XOR       \ LEA     EAX , Y [EAX]
   OP0 @   @ 85048D FFFFFF AND XOR OR 0= \ LEA     EAX , X [EAX*4]
   IF M\ 30E DTST
       OP1 @ 2+ C@ C>S CELLS OP0 @ 3 + +!
        FALSE
        OP1 OPexcise   M\ 30F DTST
       EXIT
   THEN

   OP1 @     @ FFFFFF AND 85048D  XOR \ LEA     EAX , X [EAX*4]
   OP0 @     @ FFFFFF AND 02048D  XOR    OR    \ MOV     EAX , [EDX] [EAX]
   OP1 @ 3 + @ 80 + FFFFFF00 AND OR   0=
   IF M\ 50E DTST
        8244 OP1 @ 1+ W!
        OP1 ToOP0
        FALSE
        -6 ALLOT  M\ 50F DTST
       EXIT
   THEN

   OP0 @  W@ FFFD AND 5589 =    \ MOV     EDX , X [EBP]
   IF   OP1
        BEGIN MOV_EDX_[EBP]
        UNTIL
        IF   M\ A0E DTST
            OP1 ToOP0
            FALSE
            -3 ALLOT  M\ A0F DTST
            EXIT
        THEN
   THEN

DUP    C@  0C3 XOR
OP1 @  W@ 4589 XOR OR \ 5695B0 8945FC		MOV     FC [EBP] , EAX
OP0 @  W@ 6DF7 XOR OR \ 5695B3 F76DFC		IMUL    FC [EBP]
OP1 @ 2+ C@
OP0 @ 2+ C@    XOR OR
OP0 @ 2+ C@ C>S  OFF-EBP > OR
0= IF  M\ 32A DTST
       E8F7 OP1 @ W!
       OP1 ToOP0
       FALSE -4 ALLOT M\ 32B DTST
       EXIT
   THEN
M\ PPPP
   OP0 @ W@  E7FF AND 4589 = \ TTTT AND \ MOV X [EBP] , EAX|EDX|EBX|ECX
   IF OFF-EBP >R   OP0 @ 2+ C@ C>S CELL+ TO OFF-EBP
       FALSE   OP1 -EBPCLR
       IF     R> TO OFF-EBP FALSE EXIT THEN
              R> TO OFF-EBP
   THEN

OP1 @ W@ CA8B XOR               \  8BCA		MOV     ECX , EDX
OP0 @ @ FFFFFF AND 88048D XOR OR \ 8D0488	LEA     EAX , [EAX] [ECX*4]
0= IF  M\ 42A DTST
       90048D  OP0 @ !
       FALSE  M\ 42B DTST
       EXIT
   THEN

DUP C@    0C3 XOR
OP1 @ W@ CA8B XOR    OR           \  8BCA		MOV     ECX , EDX
OP0 @ @ FFFFFF AND 90048D XOR OR \ 8D0488	LEA     EAX , [EAX] [EDX*4]
0= IF  M\ 62A DTST
        OP1 OPexcise
       FALSE  M\ 62B DTST
       EXIT
   THEN

OP1 @ @ FFFFFF AND 8D0C8D XOR \  8D0C8D00000000 LEA     ECX , 0 [ECX*4]
OP1 @ 3 + @  XOR
OP0 @ @ FFFFFF AND 1048D XOR OR \  8D0401	   LEA     EAX , [ECX] [EAX]
0= IF  M\ 52A DTST
       88048D  OP1 @ !
       OP1 ToOP0
       FALSE -7 ALLOT M\ 52B DTST
       EXIT
   THEN
OP1 @ C@ B8  XOR   \ MOV     EAX , # 58DDE8
OP0 @ @ FFFFFF AND 90048D XOR OR  \ LEA     EAX , [EAX] [EDX*4]
0= IF  M\ 4C DTST
       OP1 @ 1+ @
       OP1 OPexcise
       95 OP0 @ 2+ C!  ,
       FALSE M\ 4D DTST
       EXIT
   THEN

DUP   C@  0C3 XOR
OP1 @  @ 048DD9F7 XOR OR \     NEG     ECX     LEA     EAX , [ECX] [EAX]
0= IF  M\ 90 DTST
       C12B  OP1 @ W!
       OP1 ToOP0
       FALSE -3 ALLOT M\ 91 DTST
       EXIT
   THEN

DUP   C@  0C3 XOR
OP1 @ @ FFFFFF AND 244C8B XOR OR \   MOV     ECX , 20 [ESP]
OP0 @ W@ C12B XOR OR \ 59A13C 2BC1              SUB     EAX , ECX
0= IF  M\ 92 DTST
       442B  OP1 @ W!            \   SUB     EAX , 20 [ESP]
       OP1 ToOP0
       FALSE -2 ALLOT M\ 93 DTST
       EXIT
   THEN

DUP   C@  0C3 XOR
OP1 @ W@  D8B XOR OR  \  MOV     ECX , 599825
OP0 @ W@ C12B XOR OR \  SUB     EAX , ECX
0= IF  M\ 92 DTST
       052B  OP1 @ W!            \   SUB     EAX , 599825
       OP1 ToOP0
       FALSE -2 ALLOT M\ 93 DTST
       EXIT
   THEN

DUP   C@  0C3 XOR
OP1 @ C@ B8 XOR OR \ 59A227 B841000000  MOV     EAX , # 1000
OP0 @ C@ A3 XOR OR \ 59A22C A375995900  MOV     599975  ( Char1Glob+5  ) ,  EAX
0=  IF   M\ 98 DTST
            OP1 @ 1+ @ OP0 @ 1+ @
            OP1 @ 2+ ! OP0 @ 1+ !
            05C7  OP1 @ W! \  MOV   599975 ,  # 1000
            OP1 ToOP0
            FALSE M\ 99 DTST
            EXIT
    THEN
DUP   C@  0C3 XOR
OP1 @ C@ B8 XOR OR               \   MOV     EAX , # 2000
OP0 @ @ FFFFFF AND 244489 XOR OR \    MOV     1C [ESP] , EAX
0=  IF   M\ A8 DTST
            OP1 @ 1+ @ ,
            OP1 OPexcise
            C7  OP0 @ C! \  MOV   1C [ESP] ,  # 2000
            FALSE M\ A9 DTST
            EXIT
    THEN

OP1 @ C@ B8 XOR      \  B8E0005800  MOV     EAX , # 5800E0
OP0 @ W@ 1001 XOR OR \  0110              ADD     [EAX] , EDX
0=  IF  M\ E4 DTST
       OP1 @ 1+ @
       1501 OP0 @ W! ,       \  ADD  5800E0  , EDX
       FALSE  M\ E5 DTST
       EXIT
   THEN
OP1 @ C@ B8 XOR  \ 580185 B855550500  MOV     EAX , # 55555
OP0 @ W@ 1089 XOR OR \ 58018A 8910              MOV     [EAX] , EDX
0=  IF  M\ EE DTST
       OP1 @ 1+ @
       1589 OP0 @ W! ,       \  MOV  5800E0  , EDX
       FALSE  M\ EF DTST
       EXIT
   THEN

OP1 @ @ 1501D08B = \  8BD0           MOV     EDX , EAX
                   \  011560015800   ADD     580160 , EDX
   IF  M\ E6 DTST
       0501 OP0 @ W!        \  ADD  5800E0  , EAX
       FALSE  M\ E7 DTST
       EXIT
   THEN

DUP   C@  0C3 XOR
OP1 @ @ 0A4CB60F XOR OR \     MOVZX   ECX , BYTE PTR 2 [EDX] [ECX]
OP0 @ W@ C133 XOR OR   \     XOR     EAX , ECX
0=  IF   M\ A0 DTST
            OP0 OPexcise
            0A443200   OP0 @ !
            OP0 @ 1+ @ OP0 @ !  \ XOR   AL , BYTE PTR [EAX+EDX+44H]
            -1 ALLOT
            FALSE M\ A1 DTST
            EXIT
    THEN

DUP   C@  0C3 XOR
OP1 @ @ FFFFFF AND 850C8D XOR OR \    LEA     ECX , 58E188  ( rson+5  ) [EAX*4]
OP0 @ W@ 1189 XOR OR \ 596738 8911              MOV     [ECX] , EDX
0=  IF   M\ A2 DTST
            OP0 OPexcise
            1489  OP0 @ W!
            FALSE M\ A3 DTST
            EXIT
    THEN

OP1 @ 2+ C@
OP0 @ 2+ C@   XOR
OP1 @ W@ 5589 XOR OR \  8955F8            MOV     F8 [EBP] , EDX
0=  IF
    OP0 @ W@ 450B = \  0B45F8            OR      EAX , F8 [EBP]
         IF   M\ B2 DTST
              C20B  OP0 @ W!      \    OR      EAX , EDX
              -1 ALLOT
              FALSE M\ B3 DTST
              EXIT
         THEN
    OP0 @ W@ 453B = \  3B45F8            CMP     EAX , F8 [EBP]  !!!
         IF   M\ BE DTST
              C23B  OP0 @ W!      \    CMP      EAX , EDX
              -1 ALLOT
              FALSE M\ BF DTST
              EXIT
         THEN
    THEN

OP1 @ W@ C28B XOR    \ 8BC2              MOV     EAX , EDX
OP0 @ W@ 4589 XOR OR \ 8945FC            MOV     FC [EBP] , EAX
0=  IF   M\ C0 DTST
            5589  OP0 @ W!      \ 8955FC            MOV     FC [EBP] , EDX
            FALSE M\ C1 DTST
            EXIT
    THEN

 DUP C@    C3 XOR
OP1 @ W@ D08B XOR OR \ 8BD0              MOV     EDX , EAX
OP0 @ W@ 45C7 XOR OR \ C7450001000000    MOV     0 [EBP] , # 1
0=  IF   M\ C6 DTST
            OP1 OPexcise
            FALSE M\ C7 DTST
            EXIT
    THEN

OP1 @ 2+ C@
OP0 @ 2+ C@   XOR
OP1 @ W@ 4589 XOR OR \ 58251C 894500            MOV     0 [EBP] , EAX
OP0 @ W@ 45C7 XOR OR \ 58251F C7450001000000    MOV     0 [EBP] , # 1
0=  IF   M\ C6 DTST
            OP1 OPexcise
            FALSE M\ C7 DTST
            EXIT
    THEN

DUP C@ C3 XOR
OP1 @ @ 0501D08B XOR OR \  8BD0           MOV     EDX , EAX
                        \  011510025800      ADD     580210  , EAX
0=  IF  M\ E8 DTST
       OP1 OPexcise
       FALSE  M\ E9 DTST
       EXIT
   THEN

DUP C@ C3 XOR
OP1 @ C@ B9 XOR OR \   B94C025800  MOV     ECX , # 58024C
OP0 @ W@ 1C7 XOR OR \  C70189ABCDEF       MOV     [ECX] , # EFCDAB89
0= IF  M\ EC DTST
         OP0 @ 2+ @ OP1 @ 1+ @
         OP1 @ 2+ ! OP0 @ 1+ !
         05C7  OP1 @ W! \  MOV   58024C ,  # EFCDAB89
         OP1 ToOP0
         -1 ALLOT
       FALSE  M\ ED DTST
       EXIT
   THEN

OP1 @ C@ B8 XOR    \ 587294 B801000000  MOV     EAX , # 1
OP0 @ W@ C88B XOR OR \ 587299 8BC8              MOV     ECX , EAX
0= IF  M\ F6 DTST
         OP1 @ 1+ @
         OP0 @ 1+ !
         B9  OP0 @ C! \  MOV     ECX , # 1
         3 ALLOT
       FALSE  M\ F7 DTST
       EXIT
   THEN

OP1 @ C@ B9 XOR      \  B901000000  MOV     ECX , # 1
OP0 @ W@ D9F7 XOR OR \  F7D9              NEG     ECX
0= IF  M\ F8 DTST
         OP1 @ 1+ @ NEGATE
         OP1 @ 1+ !
         OP1 ToOP0
         -2 ALLOT
       FALSE  M\ F9 DTST
       EXIT
   THEN

DUP C@ C3 XOR
OP1 @ C@ B9 XOR OR   \  B919000000  MOV     ECX , # 19
OP0 @ W@ F7FF AND E0D3 XOR OR \  D3E8              SHR|SHL     EAX , CL
0= IF  M\ 100 DTST
         OP1 @ 1+ @
         OP1 OPexcise
         C1 OP0 @ C! C,
       FALSE  M\ 101 DTST
       EXIT
   THEN

OP1 @ C@ B8 XOR \ B801000000  MOV     EAX , # 1
OP0 @ @ FFFFFF AND 85048B XOR OR \ 8B04851CDE5700        MOV     EAX , 57DE1C [EAX*4]
0= IF  M\ 148 DTST
       A1  OP1 @  C!      \ MOV     EAX , 1
       OP0 @ 3 + @
       OP1 @ 1+  @ CELLS +  OP1 @ 1+ !
       OP0 OPexcise
       FALSE  M\ 149 DTST
       EXIT
   THEN

OP1 @ W@ C033 XOR \ 33C0		XOR     EAX , EAX
OP0 @ @ FFFFFF AND 85048B XOR OR \ 8B04851CDE5700        MOV     EAX , 57DE1C [EAX*4]
0= IF  M\ 348 DTST
       2 OP0 +!
       A1  OP0 @ C!      \ MOV     EAX , 57DE1C
       OP1 OPexcise
       FALSE  M\ 349 DTST
       EXIT
   THEN

OP1 @ C@ B8 XOR		\ B803000000  MOV     EAX , # 3
OP0 @ W@ F7FF AND E0C1 XOR OR	\ C1E804            SH[R|L]     EAX , 4
0= IF  M\ 48 DTST
       OP1 @ 1+ @
       OP0 @ 2+ C@ OP0 @ W@ 0x800 AND IF RSHIFT ELSE LSHIFT THEN
       OP1 @ 1+ !
       OP0 OPexcise
       FALSE  M\ 49 DTST
       EXIT
   THEN
DUP C@ C3 XOR
OP1 @ W@ CA8B XOR OR \ 582148 8BCA              MOV     ECX , EDX
OP0 @ W@ C12B XOR OR \ 58214A 2BC1              SUB     EAX , ECX
0= IF  M\ 180 DTST
       C22B  OP1 @ W! \    SUB     EAX , ECX
       OP0 OPexcise
       FALSE  M\ 181 DTST
       EXIT
   THEN

M\ PPPP
OP2 @ :-SET
\ TTTT  IF ." T=" 2DUP HEX U. U. THEN
 U< IF TRUE EXIT THEN

\ $ - DUP
    OP2 @ W@ D8F7 XOR
    OP1 @ W@ 4501 XOR OR 0=
    IF M\ 118 DTST
       OP2 OPexcise
       29 OP1 @ C!
       FALSE
       EXIT M\ 119 DTST
    THEN

\ $ - -
   OP2 @  @  4503D8F7 XOR           \  NEG EAX  ADD EAX, X [EBP]
   OP0 @  W@ D8F7     XOR    OR 0=  \ NEG EAX
     IF  M\ 20 DTST
         OP1 @ @ 452B OR OP2 @ ! \  SUB EAX, X [EBP]
         OP2 ToOP0
         -4 ALLOT
         FALSE M\ 21 DTST
         EXIT
     THEN
M\ PPPP
 OP0 @ W@  ADD|XOR|OR|AND=   \ $  4444  OR
    IF
      OP2 @ W@  4589  XOR \ MOV X1 [EBP], EAX
      OP2 @ 2+  C@
      OP0 @ 2+  C@    XOR OR 0=
      IF
          OP1 @ W@
          DUP  0878B =     \ MOV EAX, X [EDI]
               IF  DROP M\ 22 DTST
                   OP0 @ C@ 8700 + OP2 @ W!
                   OP1 @ 2+ @ OP2 @ 2+ !
                   OP2 ToOP0
                   FALSE -6 ALLOT M\ 23 DTST
                  EXIT
               THEN
                          \ MOV X1 [EBP], EAX
          DUP  0458B =    \  MOV     EAX , X [EBP]
                          \  ADD|XOR|OR|AND=
               IF  DROP  M\ 122 DTST
                   OP0 @ C@ 4500 +  OP1 @ W!
                   OP1 ToOP0
                   FALSE -3 ALLOT M\ 123 DTST
                   EXIT
               THEN
          FF AND
                          \ MOV X1 [EBP], EAX
          DUP  0B8 =      \ MOV EAX, # X
                          \  ADD|XOR|OR|AND=
               IF  DROP M\ 24 DTST
                   OP0 @ C@ 2+ OP1 @ C!
                   OP1 ToOP0
                   FALSE -3 ALLOT M\ 25 DTST
                   EXIT
               THEN

                          \ MOV X1 [EBP], EAX
          DUP  0A1 =      \ MOV EAX,  X
                          \  ADD|XOR|OR|AND=
               IF  DROP  M\ 26 DTST
                   OP0 @ C@ 500 +
                   OP1 @ 1+ @ OP1 @ 2+ !
                    OP1 @ W!
                   OP1 ToOP0
                   FALSE -2 ALLOT M\ 27 DTST
                   EXIT
               THEN
          DROP
      THEN
      OP2 @ W@    45C7 XOR  \ MOV X1 [EBP], # 4444 \ $ 4444 5555 OR
      OP1 @ C@      B8 XOR OR 0=   \ MOV     EAX , # 4
                          \  ADD|XOR|OR|AND=
      IF  M\ 124 DTST
          OP2 @ 3 + @ OP1 @ 1+ @
          C300  OP0 @ 2+ ! OP0 @ EXECUTE
          OP1 @ 1+ ! DROP
          OP1 ToOP0
          FALSE -3 ALLOT M\ 125 DTST
          EXIT
      THEN
      OP1 @ W@   4D89  XOR   \ MOV     F4 [EBP] , ECX
      OP1 @ 2+  C@
      OP0 @ 2+  C@    XOR OR 0=  \  ADD|XOR|OR|AND=
      IF  M\ 12 DTST
          C1 OP0 @ 1+ C!    \ XOR     EAX , ECX
          FALSE -1 ALLOT M\ 13 DTST
          EXIT
      THEN

                          \  ADD|XOR|OR|AND=
      OP0 @ 2+  C@ OP1 -EBPLIT NIP
      DUP @ W@ 45C7 =      \ MOV     F8 [EBP] , # 2710
      OVER @ 2+  C@ OP0 @ 2+  C@  = AND
      IF  OP0 @ W@ 4503 =  \  ADD     EAX , F8 [EBP]
          IF M\ 46 DTST
             @ 3 + @ OFF-EAX + TO OFF-EAX
             OP1 ToOP0
             FALSE -3 ALLOT  M\ 47 DTST
             EXIT
          THEN
      THEN
      DROP

 OP0 @ 2+ C@
 OP1 @ 2+ C@  XOR
OP1 @ W@ 45C7 XOR OR  \ 599B95 C745F401000000    MOV     F4 [EBP] , # 1
\ ADD|XOR|OR|AND=
0=     IF M\ 8C DTST
         OP0 @ C@ 2+ OP0 @ C!
         2 ALLOT
         OP1 @ 3 + @ OP0 @ 1+ !
         FALSE M\ 8D DTST
         EXIT
       THEN

    THEN
M\ PPPP

\ OP0 @ W@ 4D8B =
 OP0 @ W@ 558B =    \ MOV     EDX , X [EBP]
    IF                     \ OP-1  MOV     EAX , FC [EBP]
      OP0 @ 2+  C@ OP1 -EBPLIT NIP
      DUP @ W@ 45C7 =      \ MOV     F8 [EBP] , # 2710
      OVER @ 2+  C@ OP0 @ 2+  C@  = AND
      IF M\ 50 DTST
             -2 ALLOT
             BA OP0 @ C!
             @ 3 + @ ,
             FALSE  M\ 51 DTST
             EXIT
      THEN
      DROP
    THEN

    OP0 @ W@   4539   =      \ CMP     X [EBP] , EAX
    IF
         OP1 @ W@   5589 XOR             \ MOV     X [EBP] , EDX
         OP1 @ 2+ C@
         OP0 @ 2+ C@    XOR OR 0=        \
         IF  M\ 128 DTST
             D03B  OP0 @ W!
             FALSE -1 ALLOT M\ 129 DTST
             EXIT
         THEN

         OP1 @ C@   0B8 XOR      \ MOV EAX, # X
         OP2 @ W@  4589 XOR OR   \ MOV X1 [EBP], EAX
         OP2 @ 2+ C@
         OP0 @ 2+ C@    XOR OR 0=
         IF  M\ 28 DTST
             3D OP2 @ C!
             OP1 @ 1+ @ OP2 @ 1+ !
             OP2 ToOP0
             FALSE -6 ALLOT M\ 29 DTST
             EXIT
         THEN
\ $  4444 @ U<
         OP1 @ C@   0A1 XOR      \ MOV EAX, # X
         OP2 @ W@  4589 XOR OR   \ MOV X1 [EBP], EAX
         OP2 @ 2+ C@
         OP0 @ 2+ C@    XOR OR 0=
         IF  M\ 2A DTST
             053B OP2 @ W!
             OP1 @ 1+ @ OP2 @ 2+ !
             OP2 ToOP0
             FALSE -5 ALLOT M\ 2B DTST
             EXIT
         THEN

    THEN
   DUP   W@ 458B XOR           \ MOV     EAX , FC [EBP]
   OP0 @ W@ 1089 XOR OR        \ MOV     [EAX] , EDX
   OP1 @ W@ 558B XOR OR 0=     \ MOV     EDX , X [EBP] \ !?
    IF                     \ OP-1  MOV     EAX , FC [EBP]
          OP2 @ W@ 408D =  \ LEA     EAX , 4 [EAX]  \ $ CELL+ !
          IF  M\ 430 DTST
              OP2 @ 2+ C@
              OP2 OPexcise
              5089 OP0 @ W! C,
              FALSE M\ 431 DTST
              EXIT
          THEN

          OP3 @ :-SET U< IF TRUE EXIT THEN
\ $  44444 !
          OP3 @ 2+ C@
          OP1 @ 2+ C@    =
          IF   OP3 @ W@ 4589 =   \  MOV     X [EBP] , EAX
            IF OP2 @ C@ B8   =  \  MOV     EAX, X
               IF M\ 30 DTST
                  A3  OP3 @ C!
                  OP2 @ 1+ @  OP3 @ 1+ !
                  OP3 ToOP0
                  FALSE -8 ALLOT M\ 31 DTST
                  EXIT
               THEN
               OP2 @ W@ 878D =     \ LEA      EAX,  X [EDI]
               IF  M\ 230 DTST  \ ???
                  OP3 OPexcise
                  8789 OP2 @ W!    \ MOV     X [EDI] , EAX
                  OP2 ToOP0
                  FALSE  -5 ALLOT M\ 231 DTST
                  EXIT
               THEN
            THEN
            OP3 @ W@  45C7 =   \  MOV     X [EBP] , # 44444
            IF OP2 @ C@ B8   = \ TTTT AND  \  MOV     EAX, X
               IF  M\ 130 DTST
                   -4000 OP3 @ +!
                   OP3 @ 3 + @   OP2 @ 1+ @ OP3 @ 2+  !
                   OP3 @ 6 + !

                   OP3 ToOP0
                   FALSE -7 ALLOT M\ 131 DTST
                   EXIT
               THEN

               OP2 @ W@  878D =  \ LEA      EAX,  X [EDI]
               IF  M\ 330 DTST        \ ???
                    87C7 OP3 @ W!
                    OP3 @ 3 + @   OP2 @ 2+ @ OP3 @ 2+  !
                    OP3 @ 6 + !

                    OP3 ToOP0
                    FALSE -8 ALLOT M\ 331 DTST
                    EXIT
               THEN
            THEN
          THEN
    THEN

M\ PPPP
    OP2 @ W@ 5589 XOR     \ MOV     F8 [EBP] , EDX
    OP1 @  @ FFFFFF AND 85048D XOR OR  \  LEA     EAX , Y [EAX*X] грубо
    OP0 @ W@ 4503   XOR  OR
    OP2 @  2+ C@ OP0 @  2+  C@ XOR OR 0= \   X2=X0
    IF M\ 318 DTST
       048D OP0 @ W! 2 OP0 @ 2+ C!
       FALSE
       EXIT M\ 319 DTST
    THEN

    OP2 @ W@ 04D89 XOR        \ MOV     X [EBP] , ECX
    OP1 @ ?ChEAX              \ MOV     EAX , ____
    OP1 @ W@ 5589 <> AND OR   \ MOV     FC [EBP] , EDX
    OP0 @ W@ 558B    XOR OR   \ MOV     EDX , X [EBP]
    OP2 @ 2+ C@  OP0 @ 2+ C@  XOR OR 0=
    IF   M\ 630 DTST
       D1 OP0 @ 1+ C!
       FALSE -1 ALLOT  M\ 631 DTST
       EXIT
    THEN
    OP1 @ @ 008BC28B = \ MOV     EAX , EDX   MOV     EAX , [EAX]
    IF   M\ 730 DTST
       028B OP1 @ W!
            OP1 ToOP0
       FALSE -2 ALLOT  M\ 731 DTST
       EXIT
    THEN

   DUP C@ C3 =
   IF
       OP1 @  W@ D18B   XOR        \ MOV     EDX , ECX
       OP0 @  W@ 1089   XOR OR 0=  \ MOV     [EAX] , EDX
       IF M\ 30E DTST
          0889 OP1 @ W!
            OP1 ToOP0
            FALSE
            -2 ALLOT  M\ 40F DTST
           EXIT
       THEN
   THEN
M\ PPPP

DUP   C@  0C3 XOR
OP1 @ C@   B8 XOR OR \  MOV     EAX , # 400
OP0 @ W@ 6DF7 XOR OR \  IMUL    F8 [EBP]
OP2 @ 2+ C@
OP0 @ 2+ C@    XOR OR
0=  IF
       OP2 @ W@ 45C7 = \ MOV     F8 [EBP] , # C8
       IF     M\ E30 DTST
            OP2 @ 3 + @
            OP1 @ 1+  @ * OP1 @ 1+ !
            OP1 ToOP0
            FALSE -3 ALLOT    M\ E31 DTST
            EXIT
       THEN

       OP2 @ W@ 4589 = \ MOV     F8 [EBP] , EAX
          IF  M\ D0E DTST
              OP1 @ 1+ @
              OP1 OPexcise
              -3 ALLOT
              69 C, C0 C, ,  \  IMUL EAX,EAX,44444H
              FALSE  M\ D0F DTST
              EXIT
          THEN
    THEN

DUP    C@  0C3 XOR
OP1 @  C@   A1 XOR OR \	MOV     EAX , 569598  ( X+5  )
OP0 @  W@ 6DF7 XOR OR \ IMUL    FC [EBP]
OP2 @ 2+ C@
OP0 @ 2+ C@    XOR OR
\ OP0 @ 2+ C@ C>S  OFF-EBP > OR
0=      IF
           OP2 @  W@ 4589 = \ MOV     FC [EBP] , EAX
           IF  M\ 12A DTST
               2DF7 OP2 @ W!
               OP1 @ 1+ @  OP2 @ 2+ !
               OP2 ToOP0
               FALSE -5 ALLOT M\ 12B DTST
               EXIT
            THEN

           OP2 @  W@ 45C7 = \ MOV     F8 [EBP] , # 123
           IF  M\ 22A DTST
               B8 OP2 @ C!
               OP2 @ 3 + @  OP2 @ 1+ !
               -2 OP1 +!
               2DF7 OP1 @ W!
               OP1 @ 3 + @  OP1 @ 2+ !
               OP1 ToOP0
               FALSE -4 ALLOT M\ 22B DTST
               EXIT
            THEN
         THEN

OP2 @  W@ 4589 XOR       \ MOV     FC [EBP] , EAX
OP1 @  @ 4503008B XOR OR \ MOV     EAX , [EAX]   ADD     EAX , FC [EBP]
OP2 @ 2+ C@
OP0 @ 2+ C@    XOR OR
0=    IF   M\ 830 DTST
       03 OP1 @ C!
       OP1 ToOP0
       FALSE -3 ALLOT  M\ 831 DTST
       EXIT
    THEN
M\ PPPP

DUP    C@ 0C3 XOR
OP2 @  W@ 558B XOR OR \		MOV     EDX , 0 [EBP]
OP1 @  W@ CA8B XOR OR \		MOV     ECX , EDX
OP0 @  C@ 51   XOR OR \		PUSH    ECX
0=  IF   M\ A30 DTST
       75FF OP2 @ W!
       OP2 ToOP0
       FALSE -3 ALLOT  M\ A31 DTST
       EXIT
    THEN

OP2 @  W@ 4589 XOR  \ 		MOV     0 [EBP] , EAX
OP1 @  @ FFBFFF AND 24048B XOR OR \ 	MOV     EAX , [ESP(+4)]
OP0 @  W@ 4503 XOR OR \		ADD     EAX , 0 [EBP]
OP2 @ 2+ C@
OP0 @ 2+ C@    XOR OR
0=  IF   M\ B30 DTST
       03 OP1 @ C!
       OP1 ToOP0
       FALSE -3 ALLOT  M\ C31 DTST
       EXIT
    THEN
OP2 @ W@ 5589 XOR    \  MOV     F8 [EBP] , EDX
OP1 @    ?ChEAX   OR \ 	MOV     EAX , [EAX]
OP0 @ W@ 558B XOR OR \  MOV     EDX , F8 [EBP]
OP2 @ 2+ C@
OP0 @ 2+ C@    XOR OR
0=  IF   M\ C30 DTST
       OP1 ToOP0
       FALSE -3 ALLOT  M\ C31 DTST
       EXIT
    THEN

OP2 @ 2+ C@
OP0 @ 2+ C@    XOR
OP2 @ W@ 4589 XOR OR \ 579DE0 8945FC            MOV     FC [EBP] , EAX
OP1 @ C@   25 XOR OR \ 579DE3 2555555555  AND     EAX , # 55555555
OP0 @ W@ 558B XOR OR \ 579DE8 8B55FC            MOV     EDX , FC [EBP]
0= IF   M\ C32 DTST
	OP2 OPexcise
	OP1 02 OPinsert
	D08B OP2 @ W!    \ MOV     EDX , EAX
       OP1 ToOP0
       FALSE -3 ALLOT  M\ C33 DTST
       EXIT
    THEN

M\ PPPP
OP0 @  W@ ADD|XOR|OR|AND=
   IF
      TRUE ?~EAX !
      OP1
      BEGIN ?EAX>ECX
      UNTIL
\ DUP IF  ?~EAX @ IF 2DROP FALSE THEN
\                THEN
      IF   M\ AE DTST
         ?~EAX @
         IF    ECX:=EAX
         ELSE  BEGIN EAX>ECX0 UNTIL
         THEN  BEGIN EAX>ECX  UNTIL
           DROP
           OP0 @  W@  4503 = \ TTTT AND  \ ADD     EAX , X [EBP]
           IF   01048D OP0 @ !            ELSE   \ LEA EAX, [EAX+ECX]
                0C1  OP0 @ 1+ C! -1 ALLOT        \  OR EAX,ECX
           THEN FALSE  M\ AF DTST
           EXIT
      THEN   M\ PPPP

      TRUE ?~EAX !
      OP1   \  TTTT IF ."  ?EAX>EBX" THEN
      BEGIN ?EAX>EBX  \ TTTT IF ." F=" DUP . THEN
      UNTIL
\ DUP IF  ?~EAX @ IF 2DROP FALSE THEN
\                THEN
      IF   M\ AE DTST
         ?~EAX @  \ ." {#}"
         IF    EBX:=EAX
         ELSE  BEGIN EAX>EBX0 UNTIL
         THEN  BEGIN EAX>EBX  UNTIL
           DROP
           OP0 @  W@  4503 = \ TTTT AND  \ ADD     EAX , X [EBP]
           IF   03048D OP0 @ !            ELSE   \ LEA EAX, [EAX+EBX]
           OP0 @  W@ 4539  =                     \ CMP F8 [EBP] , EAX
           IF   C33B OP0 @    W! -1 ALLOT ELSE   \ CMP EAX , EBX
           OP0 @  W@ 453B  =                     \ CMP EAX , F8 [EBP]
           IF   D83B OP0 @    W! -1 ALLOT ELSE   \ CMP EBX , EAX
                0C3  OP0 @ 1+ C! -1 ALLOT        \  OR EAX,EBX
           THEN THEN THEN FALSE  M\ AF DTST
           EXIT
      THEN   M\ PPPP


      OP1 @   @ 4503D8F7 = \  NEG     EAX  \	ADD     EAX , 4 [EBP]
      IF  OP2 @ W@ 458B =     \	MOV     EAX , X [EBP]
          IF  M\ D0E DTST
              OP1 OPexcise
              OP1 @ 2+ C@  OP0 @ 2+ C@
              OP1 @ 2+ C!  OP0 @ 2+ C!
              2B OP0 @ C!
              FALSE  M\ D0F DTST
              EXIT
          THEN
          OP2 @ C@ A1 =     \	MOV     EAX , X
          IF  M\ D0E DTST
              OP2 @ 1+ @
              OP2 OPexcise
              OP1 OPexcise
              8B OP0 @ C!
              SetOP 2B C, 5 C, ,
              FALSE  M\ D0F DTST
              EXIT
          THEN
      THEN
   THEN
M\ PPPP

OP2 @ W@ FFFD AND 4589 XOR    \  MOV     F8 [EBP] , EAX
OP1 @ W@ FFFD AND 4589 XOR OR \  MOV     EAX , F8 [EBP]
OP2 @ 2+ C@
OP1 @ 2+ C@    XOR OR
0=  IF   M\ E30 DTST
       OP1 OPexcise
       FALSE  M\ E31 DTST
       EXIT
    THEN

OP2 @  @  458BC28B =   \ MOV     EAX , EDX  MOV     EAX , 0 [EBP]
   IF   M\ 44 DTST
       OP2 OPexcise
       FALSE M\ 45 DTST
       EXIT
   THEN
DUP C@    0C3 XOR
OP1 @ C@  B9 - FE AND  OR   \ MOV     EDX , # 1000 MOV     ECX , # 1000
OP0 @ W@ 1189 XOR      OR   \ MOV     [ECX] , EDX
0= IF  M\ 54 DTST
       OP1 @ C@ BA =
       IF   01C7     \ MOV     [ECX] , # 1000
       ELSE 1589     \ MOV     1000  , [EDX]
       THEN
       OP1 1 OPresize
        OP1 @ W!
       OP0 OPexcise
       FALSE  M\ 55 DTST
       EXIT
   THEN

  DUP C@    C3 XOR
OP1 @ C@    B9 XOR OR  \ MOV     ECX , # 1000
OP0 @ W@  4D89 XOR OR \	MOV    X [EBP] , ECX
0= IF  M\ 58 DTST
        OP1 @ 1+ @
        OP1 OPexcise
        45C7 OP0 @ W! ,   \ MOV     FC [EBP] , # 1000
        FALSE  M\ 59 DTST
        EXIT
   THEN

DUP C@    0C3 XOR
OP1 @ @ FFFFFF AND 8D0C8D XOR OR \ LEA     ECX , 5898B8  ( rson+5  ) [ECX*4]
OP0 @ W@  01C7 XOR    OR   \ MOV     [ECX] , # 1000
0= IF  M\ 56 DTST
             04C7 OP1 @ W!   \ MOV  X [ECX*4] , # 1000
             OP0 @ 2+ @
             OP0 OPexcise ,
             FALSE  M\ 57 DTST
             EXIT
       EXIT
   THEN
DUP C@    0C3 XOR
OP1 @ W@ C18B XOR OR \   MOV     EAX , ECX
OP0 @ @ FFFFFF AND 85048D XOR OR \   LEA     EAX , X [EAX*4]
0= IF  M\ 5A DTST
             OP1 OPexcise
             8D OP0 @ 2+ C! \   LEA     EAX , X [ECX*4]
             FALSE  M\ 5B DTST
             EXIT
       EXIT
   THEN

 OP2 @ 2+ C@
 OP0 @ 2+ C@    XOR
OP2 @ W@ 4589 XOR OR \     MOV     F8 [EBP] , EAX
OP1 @ @ FFFFFF AND 8D048D XOR \   LEA     EAX , X [ECX*4]
OP1 @ C@ B8  <> AND  OR \ B801000000  MOV     EAX , # 1
OP0 @ W@ 558B XOR OR \     MOV     EDX , F8 [EBP]
0= IF  M\ 5C DTST
             OP1 2 OPinsert
             D08B  OP2 @ W! \  MOV     EDX , EAX
             OP0 OPexcise
             FALSE  M\ 5D DTST
             EXIT
       EXIT
   THEN

OP0 @ 2+ C@
OP2 @ 2+ C@  XOR
OP2 @ W@ 4589 XOR OR \   MOV     F8 [EBP] , EAX
OP1 @ @ FFFFFF AND 24448B XOR OR \   MOV     EAX , 18 [ESP]
OP0 @ W@ 558B XOR OR \    MOV     EDX , F8 [EBP]
0=  IF   M\ 9C DTST
            OP1 2 OPinsert
            D08B  OP2 @ W! \  MOV     EDX , EAX
            OP0   OPexcise
            FALSE M\ 9D DTST
            EXIT
    THEN

DUP C@   0C3 XOR
OP2 @ @ D9F7CA8B XOR OR \    MOV     ECX , EDX    NEG     ECX
OP0 @ 1- @ 01048DD9 XOR OR \    LEA     EAX , [ECX] [EAX]
0= IF  M\ 66 DTST
       OP2 OPexcise
       02048DDA OP1 @ 1+ !         \  NEG     EDX lea EAX, [EAX+EDX]
       FALSE  M\ 67 DTST
       EXIT
   THEN

OP2 @  @ 45C7D08B XOR \        MOV     EDX , EAX    MOV     F8 [EBP] , # X
OP0 @ C@ BA XOR OR \   MOV     EDX , # 0
0= IF  M\ 6A DTST
       OP2 OPexcise
       FALSE  M\ 6B DTST
       EXIT
   THEN

  DUP C@  0C3 XOR
OP2 @ C@   BA XOR OR \   MOV     EDX , # 4
OP1 @ W@ 5501 XOR OR \   ADD     0 [EBP] , EDX
OP0 @ W@ 4513 XOR OR \   ADC     EAX , FC [EBP]
0= IF  M\ 82 DTST
       OP0 4 OPresize  4 OP0 +!
       OP2 @  1+ @   OP2 OPexcise
       OP1 @ 3 + !  4581 OP1 @ W! \ ADD     0 [EBP] , # 4
       FALSE  M\ 83 DTST
       EXIT
   THEN

 OP2 @ 3 + C@
 OP0 @ 3 + C@    XOR
OP2 @ @ FFFFFF AND 24448B XOR OR \  MOV     EAX , 30 [ESP]
OP0 @ @ FFFFFF AND 244489 XOR OR \  MOV     30 [ESP] , EAX
0= IF    OP1 @ @ 8901408D = \  8D4001         LEA     EAX , 1 [EAX]
     IF  M\ 88 DTST
         OP1 OPexcise
         FF  OP0 @  C!
         FALSE  M\ 89 DTST
         EXIT
     THEN
    OP1 @ @ 89FF408D = \  8D4001         LEA     EAX , -1 [EAX]
     IF  M\ 94 DTST
         OP1 OPexcise
         4CFF  OP0 @  W!
         FALSE  M\ 95 DTST
         EXIT
     THEN
   THEN

  DUP C@   C3 XOR
OP2 @ W@ 408D XOR OR \  LEA     EAX , 1 [EAX]
OP1 @ @ FFFFFF AND 244403 XOR OR \  ADD     EAX , 8 [ESP]
OP0 @ @ FFFFFF AND B60F XOR OR \     MOVZX   EAX , BYTE PTR [EAX]
0= IF  M\ 9E DTST
       OP2 @ 2+ C@   OP2 OPexcise
       548B OP1 @ W!              \  MOV     EDX , 8 [ESP]
       -3 ALLOT
       0244B60F , C,         \   MOVZX   EAX , BYTE PTR 2 [EDX] [EAX]
       FALSE  M\ 9F DTST
       EXIT
   THEN

OP2 @ 2+ C@
OP0 @ 2+ C@   XOR
OP2 @ W@ 5589 XOR OR     \ 8955F8            MOV     F8 [EBP] , EDX

OP1 @ W@ D8F7 XOR        \ F7D8              NEG     EAX
OP1 @ W@ D0F7 <> AND    \ F7D0              NOT     EAX
OP1 @ C@ 58   <> AND    \  58          POP     EAX
OP1 @ W@ C123 <> AND OR \  23C1           AND     EAX , ECX

OP0 @ W@ ADD|XOR|OR|AND= 0=
OP0 @ W@ 458B  <> AND OR
\ OP0 @ W@ 4503 XOR OR \ 0345F8            ADD     EAX , F8 [EBP]
0=  IF  M\ 1B6 DTST
       C2 OP0 @ 1+ C!  \  ADD     EAX , EDX
       -1 ALLOT
       FALSE  M\ 1B7 DTST
       EXIT
   THEN

DUP   C@   C3 XOR
OP2 @ W@ 4D8B XOR OR \  8B4D00            MOV     ECX , 0 [EBP]
OP1 @ @ 4D89C82B XOR OR \  2BC8           SUB     ECX , EAX
                        \  894D00        MOV     0 [EBP] , ECX
0= IF  M\ B6 DTST

       4529 OP2 @ W!  \  SUB     0 [EBP] , EAX
       OP2 ToOP0
       -5 ALLOT
       FALSE  M\ B7 DTST
       EXIT
   THEN

OP2 @ 2+ C@
OP0 @ 2+ C@   XOR
OP2 @ W@ 458B XOR OR \ 58266F 8B4500            MOV     EAX , 0 [EBP]
OP1 @ W@ 558B XOR OR \ 582672 8B5504            MOV     EDX , 4 [EBP]
OP0 @ W@ 4589 XOR OR \ 582675 894500            MOV     0 [EBP] , EAX
0= IF  M\ D2 DTST
       OP0 OPexcise
       FALSE  M\ D3 DTST
       EXIT
   THEN


OP2 @ W@ 458B XOR    \  8B4500            MOV     EAX , 0 [EBP]
OP1 @ W@ 5589 XOR OR \  895500            MOV     0 [EBP] , EDX
OP0 @ W@ C88B XOR OR \  8BC8              MOV     ECX , EAX
0= IF  M\ F4 DTST
       4D8B   OP2 @ W!      \     MOV     ECX , 0 [EBP]
       C18B   OP0 @ W!      \     MOV     EAX , ECX
       FALSE M\ F5 DTST
       EXIT
   THEN

OP2 @ C@ B8 XOR      \  B801000000  MOV     EAX , # 1
OP2 @ 1+ @ SHORT? 0= OR
OP1 @ W@ 558B XOR OR \  8B5500      MOV     EDX , 0 [EBP]
OP0 @ W@ 48D XOR OR  \  8D0402      LEA     EAX , [EDX] [EAX]
0= IF  M\ FC DTST
       OP2 @ 1+ @
       OP2 OPexcise
       428D  OP0 @ W!        \  LEA     EAX , 1 [EDX]
       OP0 @ 2+ C!
       FALSE  M\ FD DTST
       EXIT
   THEN

OP2 @ W@ 558B XOR    \ 5876BB 8B5500            MOV     EDX , 0 [EBP]
OP1 @ W@ 428D XOR OR \ 5876BE 8D4201            LEA     EAX , 1 [EDX]
OP0 @ W@ 4589 XOR OR \ 5876C1 894500            MOV     0 [EBP] , EAX
0= IF  M\ FC DTST
       OP1 @ 2+ C@
       OP1 OPexcise
       4583   OP0 @ W!    C,    \  ADD     EAX , 1 [EDX]
       FALSE  M\ FD DTST
       EXIT
   THEN

OP2 @ 2+ C@
OP0 @ 2+ C@   XOR
OP2 @ W@ 4589 XOR OR     \  8945EC            MOV     EC [EBP] , EAX

OP1 ?EDX_[EBP]  OR

OP0 @ W@ 558B XOR OR     \  8B55EC            MOV     EDX , EC [EBP]
0= IF  M\ 104 DTST
       OP0 OPexcise
       OP0 02 OPinsert
       D08B   OP1 @ W!      \ MOV     EDX , EAX
       FALSE  M\ 105 DTST
       EXIT
   THEN

OP2 @ W@ D08B XOR  \ 5A5408 8BD0              MOV     EDX , EAX

OP1 @ @ FFFFFF AND 95048D XOR  \  8D0495D8825900    LEA     EAX , 5982D8 [EDX*4]
OP1 @ @ FFFFFF AND 95048B <> AND OR \  8B0495D8825900    MOV     EAX , 5982D8 [EDX*4]

OP0 @ W@ 558B XOR OR \ 5A5411 8B55F8            MOV     EDX , F8 [EBP]
0= IF  M\ 138 DTST
       OP2 OPexcise
       85  OP1 @ 2+ C!      \ LEA     EAX , 5982D8 [EAX*4]
       FALSE  M\ 139 DTST
       EXIT
   THEN

DUP C@ C3 XOR
OP2 @ @ E1C1CA8B XOR OR \  8BCA           MOV     ECX , EDX
                        \  C1E107         SHL     ECX , 7
OP0 @ W@ C10B XOR OR    \  0BC1           OR      EAX , ECX
0= IF  M\ 108 DTST
       OP2 OPexcise
       E2C1   OP1 @ W!      \ SHL     EDX , 7
       C20B   OP0 @ W!      \ OR      EAX , EDX
       FALSE  M\ 109 DTST
       EXIT
   THEN

OP2 @ @ 878DD08B XOR \  8BD0           MOV     EDX , EAX
                     \  8D873C100000      LEA     EAX , 103C [EDI]
OP0 @ W@ 1088 XOR OR \  8810              MOV     [EAX] , DL
0= IF  M\ 110 DTST
       OP1 @ 2+ @
       9788    OP1 @ W!      \ MOV     103C [EDI] , DL
       878D   OP0 @ W!      \ LEA     EAX , 103C [EDI]
       ,
       FALSE  M\ 111 DTST
       EXIT
   THEN

DUP C@ C3 XOR
OP2 @ @ 5589D98B XOR OR \ 58809A 8BD9           MOV     EBX , ECX
                        \ 58809C 8955EC            MOV     EC [EBP] , EDX
OP0 @ W@ C323 XOR OR    \ 58809F 23C3              AND     EAX , EBX
0= IF  M\ 116 DTST
       OP2 OPexcise
       C123     OP0 @ W!      \ AND     EAX , ECX
       FALSE  M\ 117 DTST
       EXIT
   THEN


DUP C@ C3 XOR
OP2 @ W@ 4D8B XOR OR    \ 8B4DF0            MOV     ECX , F0 [EBP]
OP1 @ @ 23F05589 XOR OR \ 8955F0         MOV     F0 [EBP] , EDX
                        \ 23C1              AND     EAX , ECX
0= IF  M\ 11E DTST
       OP0 OPexcise
        4523  OP1 @ W!      \    AND     EAX , F0 [EBP]
       FALSE  M\ 11D DTST
       EXIT
   THEN

DUP C@ C3 XOR
OP2 @ W@ 558B XOR OR \ 5881D2 8B55EC            MOV     EDX , EC [EBP]
OP1 @ @ BF04523 XOR OR \ 5881D5 2345F0          AND     EAX , F0 [EBP]
OP0 @ W@ C20B XOR OR \ 5881D8 0BC2              OR      EAX , EDX
0= IF  M\ 120 DTST
       OP2 @ 2+ C@
       OP2 OPexcise
        450B   OP0 @ W!      \    OR     EAX , EC [EBP]
         C,
       FALSE  M\ 11D DTST
       EXIT
   THEN

OP2 @ 2+ C@
OP1 @ 2+ C@   XOR
OP2 @ W@ 5589 XOR OR \  8955F0            MOV     F0 [EBP] , EDX
OP1 @ W@ 4523 XOR OR \  2345F0            AND     EAX , F0 [EBP]
0= IF  M\ 126 DTST
       OP1 OPexcise
       OP0 2 OPinsert
        C223   OP1 @ W!      \    AND     EAX , EDX
       FALSE  M\ 127 DTST
       EXIT
   THEN

OP2 @ 2+ C@
OP1 @ 2+ C@   XOR
OP2 @ W@ 4D89 XOR OR \  894DFC            MOV     FC [EBP] , ECX
OP1 @ W@ 4523 XOR OR \  2345FC            AND     EAX , FC [EBP]
0= IF  M\ 13E DTST
       OP1 OPexcise
       OP0 2 OPinsert
        C123   OP1 @ W!      \    AND     EAX , ECX
       FALSE  M\ 13F DTST
       EXIT
   THEN


DUP C@ C3 XOR
OP2 @ 2+ C@ C>S OFF-EBP U< 0= OR
OP2 @ W@ 558B XOR OR    \ 58838C 8955F8            MOV    EDX , F4 [EBP]
OP1 @ @ C20BC123 XOR OR \ 58838F 23C1           AND     EAX , ECX
                        \ 588391 0BC2              OR      EAX , EDX
0= IF  M\ 12C DTST
       OP2 @ 2+ C@
       OP2 OPexcise
        450B   OP0 @ W!      \    OR     EAX ,  F8 [EBP]
       C,
       FALSE  M\ 12D DTST
       EXIT
   THEN

DUP C@ C3 XOR
OP2 @ @ C123CA8B XOR OR \  8BCA           MOV     ECX , EDX
                        \  23C1              AND     EAX , ECX
OP0 @ W@ 450B XOR OR    \  0B45F4            OR      EAX , F4 [EBP]
0= IF  M\ 1F2 DTST
       OP2 OPexcise
       C223    OP1 @ W!     \ AND     EAX , EDX
       FALSE  M\ 1F3 DTST
       EXIT
   THEN

DUP C@ C3 XOR
OP2 @ W@ 4D8B XOR OR \ 5884C1 8B4DF8            MOV     ECX , F8 [EBP]
OP1 @ @ C10BCA23 XOR OR \ 5884C4 23CA           AND     ECX , EDX
OP0 @ W@ C10B XOR OR \ 5884C6 0BC1              OR      EAX , ECX
0= IF  M\ 1F4 DTST
       OP1 OPexcise
       5523  OP1 @ W!     \ AND     EDX , F8 [EBP]
       C20B  OP0 @ W!      \    OR      EAX , EDX
       FALSE  M\ 1F5 DTST
       EXIT
   THEN

DUP C@ C3 XOR
OP2 @ W@ 558B XOR OR \ 587860 8B5500            MOV     EDX , 0 [EBP]
OP1 @ W@ 528D XOR OR \ 587863 8D5201            LEA     EDX , 1 [EDX]
OP0 @ W@ 5589 XOR OR \ 587866 895500            MOV     0 [EBP] , EDX
0= IF  M\ 144 DTST
       OP1 @ 2+ C@
         OP2 OPexcise
         OP1 OPexcise
       4583  OP0 @ W!      \ ADD     0 [EBP] , # 1
         C,
       FALSE  M\ 145 DTST
       EXIT
   THEN

DUP C@ C3 XOR
\ OP2 @ W@ 5589 XOR OR       \ 8B5500         EDX , MOV     0 [EBP]
OP2 @ W@ 558B XOR OR       \ 8B5500         EDX , MOV     0 [EBP]
 OP1 @ W@ E0D3 XOR OR        \ D3E0           SHL     EAX , CL
\ OP1 ?EDX_[EBP]  OR         \ D3E0           SHL     EAX , CL
OP0 @ W@ C20B XOR OR  \ 0BC2           OR      EAX , EDX
0= IF  M\ 14A DTST
       OP2 @ 2+ C@
       OP2 OPexcise
       45 OP0 @ 1+  C!      \ OR      EAX , 0 [EBP]
       C,
       FALSE  M\ 14B DTST
       EXIT
   THEN

OP2 @ 2+ C@
OP0 @ 2+ C@    XOR
OP2 @ W@ 4589 XOR OR \ 8945FC            MOV     FC [EBP] , EAX
OP1 @ W@ 458B XOR OR \ 8B4500            MOV     EAX , 0 [EBP]
OP0 @ W@ 558B XOR OR \ 8B55FC            MOV     EDX , FC [EBP]
0= IF  M\ 162 DTST
	OP1 ToOP0 -3 ALLOT
	OP0 2 OPinsert
	D08B OP1 @ W!
       FALSE  M\ 163 DTST
       EXIT
   THEN

DUP C@ C3 XOR
OP2 @ W@ 4D8B XOR OR \ 582055 8B4D04            MOV     ECX , 4 [EBP]
OP1 @ W@ 5589 XOR OR \ 582058 895504            MOV     4 [EBP] , EDX
OP0 @ @ FFFFFF AND 1048D XOR OR \ 58205B 8D0401         LEA     EAX , [ECX] [EAX]
0= IF  M\ 168 DTST
	4503 OP2 @ W! \ ADD     EAX , 4 [EBP]
       OP0 OPexcise
       FALSE  M\ 169 DTST
       EXIT
   THEN

DUP C@ C3 XOR
OP2 @ W@ 558B XOR OR \ 582202 8B5500            MOV     EDX , 0 [EBP]
OP1 @ W@ 4503 XOR OR \ 582205 034504            ADD     EAX , 4 [EBP]
OP0 @ W@ C22B XOR OR \ 582208 2BC2              SUB     EAX , EDX
0= IF  M\ 170 DTST
	OP2 @ 2+ C@
       OP2 OPexcise
	452B OP0 @ W! C, \  SUB     EAX , 0 [EBP]
       FALSE  M\ 171 DTST
       EXIT
   THEN

 DUP C@ C3 XOR
 OP2 @ 2+ C@
 OP1 @ 2+ C@  = OR
OP2 @ W@ 558B XOR OR \ 8B5500            MOV     EDX , 0 [EBP]
OP1 @ W@ 4589 XOR OR \ 8945FC            MOV     FC [EBP] , EAX
OP0 @ @ FFFFFF AND 2048D XOR OR \ 8D0402         LEA     EAX , [EDX] [EAX]
0= IF  M\ 158 DTST
	OP2 @ 2+ C@
       OP2 OPexcise
	4503 OP0 @ W!     \          ADD     EAX , 0 [EBP]
	OP0 @ 2+  C!
       FALSE  M\ 159 DTST
       EXIT
   THEN

DUP C@ C3 XOR
OP2 @ 2+ C@
OP0 @ 2+ C@  XOR OR
OP2 @ 2+ C@
OP1 @ 2+ C@  = OR
OP2 @ W@ 4589 XOR OR \ 571DD0 8945FC            MOV     FC [EBP] , EAX
OP1 @ W@ 4503 XOR OR \ 571DD3 034500            ADD     EAX , 0 [EBP]
OP0 @ W@ 6DF7 XOR OR \ 571DD6 F76DFC            IMUL    FC [EBP]
0= IF  M\ 160 DTST
       OP2 OPexcise
	OP1 2 OPinsert
        C88B OP2 @ W!	\	MOV     ECX , EAX
        E9F7 OP0 @ W!	\	IMUL ECX	
       FALSE -1 ALLOT M\ 161 DTST
       EXIT
   THEN
DUP C@ C3 XOR
OP2 @ 2+ C@
OP0 @ 2+ C@  XOR OR

OP2 @ W@ 4589 XOR OR \  894500            MOV     0 [EBP] , EAX
OP1 @ W@ AF0F XOR OR \  0FAFC2            IMUL    EAX , EDX
OP0 @ W@ 4503 XOR OR \  034500            ADD     EAX , 0 [EBP]

0= IF  M\ 248 DTST
       OP2 OPexcise
	OP1 2 OPinsert
	C88B OP2 @  W! \	MOV     ECX , EAX
        C103 OP0 @  W! \	ADD     EAX , ECX

       FALSE -1 ALLOT M\ 141 DTST
       EXIT
   THEN

OP1 @ 2+ C@
OP0 @ 2+ C@  XOR
OP2 @ W@ C28B XOR OR \ 571EED 8BC2              MOV     EAX , EDX
OP1 @ W@ 5589 XOR OR \ 571EEF 8955FC            MOV     FC [EBP] , EDX
OP0 @ W@ 6DF7 XOR OR \ 571EF2 F76DFC            IMUL    FC [EBP]
0= IF  M\ 182 DTST
       EAF7  OP0 @ W! \ 		IMUL    EDX 	
       FALSE -1 ALLOT  M\ 183 DTST
       EXIT
   THEN

( !!!!!!!!!!
DUP C@ C3 XOR
OP1 @ @ EAF7C28B XOR OR	\  8BC2       MOV     EAX , EDX
			 \ F7EA       IMUL    EDX
0= IF  M\ 184 DTST
       OP1 OPexcise
       C2AF0F	OP0 @ ! \ 		IMUL    EAX , EDX
       FALSE 1 ALLOT  M\ 185 DTST
       EXIT
   THEN
)


OP3 @ :-SET U< IF TRUE EXIT THEN
M\ PPPP

OP3 @  @  D08BC28B =   \ MOV     EAX , EDX  MOV     EDX , EAX
   IF   M\ 42 DTST
       OP2 OPexcise
       FALSE M\ 43 DTST
       EXIT
   THEN

    OP1 @ W@   453B   =      \ CMP   EAX , X [EBP]
    IF OP0 @ @ FFFCFF AND C09C0F =  \ SETLE   AL
      IF
\ $  4444  <
         OP2 @ C@   0B8 XOR      \ MOV EAX, # X
         OP3 @ W@  4589 XOR OR   \ MOV X1 [EBP], EAX
         OP3 @ 2+ C@
         OP1 @ 2+ C@    XOR OR 0=
         IF  M\ 2C DTST
             3D OP3 @ C!
             OP2 @ 1+ @ OP3 @ 1+ !
             2 OP2 +!
             OP0 @ @ 300 XOR  OP2 @ !     \  SETGE   AL
             OP2 ToOP0
             FALSE -6 ALLOT M\ 2D DTST
             EXIT
         THEN
\ $  4444 @ <
         OP2 @ C@   0A1 XOR      \ MOV EAX, # X
         OP3 @ 2+ C@
         OP1 @ 2+ C@    XOR OR 0=
         IF
         OP3 @ W@  4589 =   \ MOV X1 [EBP], EAX
           IF M\ 2E DTST
              053B OP3 @ W!
              OP2 @ 1+ @ OP3 @ 2+ !
              3 OP2 +!
              OP0 @ @ 300 XOR  OP2 @ !     \  SETGE   AL
              OP2 ToOP0
              FALSE -5 ALLOT M\ 2F DTST
              EXIT
            THEN

         OP3 @ W@  45C7 =   \
           IF M\ 12E DTST
              3D81 OP3 @ W!
              OP3 @ 3 + @
              OP2 @ 1+ @ OP3 @ 2+ !  OP3 @ 6 + !
              3 OP2 +!
              OP0 @ @   OP2 @ !
              OP2 ToOP0
              FALSE -5 ALLOT M\ 12F DTST
              EXIT
            THEN

         THEN
       THEN
    THEN
    DUP      @ 458B1089 XOR       \ MOV  [EAX], EDX   MOV EAX , X [EBP]
    OVER 5 + @ C3086D8D XOR OR    \ LEA  EBP, 8 [EBP]   RET
    OP0  @  W@     D18B XOR OR 0= \ MOV  EDX , ECX
    IF   M\ 51E DTST
         0889 OP0 @ W!    \ MOV     [EAX] , ECX
         2+ FALSE  M\ 51F DTST
        EXIT
    THEN
M\ PPPP

OP3 @ W@ 4589 XOR  \ 8945FC		MOV     FC [EBP] , EAX
OP2 @ C@ B8 XOR OR \ B8189D5700	MOV     EAX , # 579D18
OP1 @ W@ 558B XOR OR \  8B55FC		MOV     EDX , FC [EBP]
OP0 @ @ FFFFFF AND 90048D XOR OR \ 8D0490	LEA     EAX , [EAX] [EDX*4]
    OP3 @ 2+ C@ OP1 @ 2+ C@ XOR OR  \  X0=X2
0= IF   M\ 840 DTST
       OP2 @ 1+  @   85048D OP2 @ !
       OP2 @ 3 + !
       OP2 ToOP0
       FALSE  -4 ALLOT M\ 841 DTST
       EXIT
   THEN

OP3 @ W@ 558B XOR    \	MOV     EDX , FC [EBP]
OP2 @ W@ 4589 XOR OR \	MOV     FC [EBP] , EAX
OP1 @ W@ C28B XOR OR \  MOV     EAX , EDX
OP0 @ W@ 558B XOR OR \ 	MOV     EDX , FC [EBP]
OP3 @ 2+ C@  OP2 @ 2+ C@ XOR OR
OP2 @ 2+ C@  OP0 @ 2+ C@ XOR OR
0= IF   M\ F30 DTST
       OP3 OPexcise
       OP2 OPexcise
       D08B  OP1 @ W! \ MOV     EDX , EAX
       458B  OP0 @ W! \ MOV     EAX , FC [EBP]
       DO_EAX>ECX DROP
       SetOP 89 C, 55 C,  OP0 @ 2+ C@ C, \  MOV     FC [EBP] , EDX
       FALSE   M\ F31 DTST
       EXIT
   THEN
OP3 @ 2+ C@
OP0 @ 2+ C@ XOR
OP3 @ W@ 4589 XOR OR \ 57BF16 8945FC            MOV     FC [EBP] , EAX
OP2 @ W@ C033 XOR OR \ 57BF19 33C0              XOR     EAX , EAX
OP1 @ W@ 4581 XOR OR \ 57BF1B 814500BA040000    ADD     0 [EBP] , # 4BA
OP0 @ W@ 4513 XOR OR \ 57BF22 1345FC            ADC     EAX , FC [EBP]
0= IF   M\ 84 DTST
       OP2 OPexcise
       D083  OP0 @ ! \ ADC     EAX , # 0
       FALSE   M\ 85 DTST
       EXIT
   THEN

OP3 @ 2+ C@
OP0 @ 2+ C@ XOR
OP3 @ W@ 4589 XOR OR \ 59690C 8945F0            MOV     F0 [EBP] , EAX
OP2 @ C@ B8 XOR OR \ 59690F B801000000  MOV     EAX , # 1
OP1 @ C@ BA XOR OR \ 596914 BA00000080  MOV     EDX , # 80000000
OP0 @ W@ 552B XOR OR \ 596919 2B55F0            SUB     EDX , F0 [EBP]
0= IF   M\ A4 DTST
       OP2 @ 1+ @
       OP2 OPexcise
       D02B  OP0 @ ! \ SUB     EDX , EAX
       -1 ALLOT
       SetOP  B8 C, ,
       FALSE   M\ A5 DTST
       EXIT
   THEN

OP3 @ 2+ C@
OP0 @ 2+ C@ XOR
OP3 @ W@ 4589 XOR  OR  \  8945F8            MOV     F8 [EBP] , EAX
OP2 ?EDX_[EBP] OR
OP1 ?EDX_[EBP] OR
OP0 @ W@ 558B XOR OR \  8B55F8            MOV     EDX , F8 [EBP]
0= IF   M\ EC DTST
       OP2 2 OPinsert
       D08B  OP3 @ W! \  MOV     EDX , EAX
       OP0 OPexcise
       FALSE   M\ ED DTST
       EXIT
   THEN

OP3 @ 2+ C@
OP0 @ 2+ C@ XOR
OP3 @ W@ 4589 XOR OR \  8945FC            MOV     FC [EBP] , EAX
OP2 @ C@ B8 XOR OR   \  B801000000        MOV     EAX , # 1
OP1 @ C@ B9 XOR OR   \  B9FFFFFFFF        MOV     ECX , # FFFFFFFF
OP0 @ W@ 4D03 XOR OR \  034DFC            ADD     ECX , FC [EBP]
0= IF   M\ FA DTST
       OP2 @ 1+ @
       OP2 OPexcise
       C803  OP0 @ W! \  ADD     ECX , EAX
       -1 ALLOT
       SetOP   B8 C, ,
       FALSE   M\ FB DTST
       EXIT
   THEN

OP3 @ C@ A1 XOR     \ 587C37 A1D8795800  MOV     EAX , 5879D8  ( C+5  )
OP2 @ W@ D08B XOR OR \ 587C3C 8BD0              MOV     EDX , EAX
OP1 @ C@ A1 XOR OR \ 587C3E A1E4795800  MOV     EAX , 5879E4  ( D+5  )
OP0 @ W@ 4589 XOR OR \ 587C43 8945EC            MOV     EC [EBP] , EAX
0= IF   M\ 114 DTST
       OP2 OPexcise
       OP2 1 OPresize
       158B OP2 @ W!
       FALSE   M\ 115 DTST
       EXIT
   THEN

OP3 @ 2+ C@
OP1 @ 2+ C@ XOR
OP1 @ 2+ C@
OP0 @ 2+ C@ XOR OR
OP3 @ W@ 4589 XOR OR \  894500            MOV     0 [EBP] , EAX
OP2 @ W@ 428D XOR OR \  8D4201            LEA     EAX , 1 [EDX]
OP1 @ W@ 558B XOR OR \  8B5500            MOV     EDX , 0 [EBP]
OP0 @ W@ 4589 XOR OR \  894500            MOV     0 [EBP] , EAX
0= IF  M\ 140 DTST
       OP3 OPexcise
       OP2 2 OPinsert
       C88B  OP3 @ W!     \ MOV     ECX , EAX
       OP1 OPexcise
       OP0 2 OPinsert
       D18B  OP1 @ W!     \ MOV     EAX , ECX
       FALSE  M\ 141 DTST
       EXIT
   THEN



OP4 @ :-SET U< IF TRUE EXIT THEN

\ $ 10 LSHIFT
    OP4 @ W@ 4589     XOR      \ MOV X0 [EBP] , EAX
    OP3 @ C@ B8       XOR OR   \ MOV EAX, # 10
    OP2 @  @ 458BC88B XOR OR   \ MOV ECX, EAX \ !? MOV EAX, X2 [EBP]
    OP0 @ W@ F7FF AND E0D3  XOR OR  \  SHL|SHR    EAX , CL
    OP4 @ 2+ C@ OP1 @ 2+ C@ XOR OR  \  X0=X2
    0=
    IF  M\ 32 DTST
        OP0 @ W@ 0012 - OP4 @ W!
        OP3 @ 1+ @ OP4 @ 2+ C!
        OP4 ToOP0
        FALSE -C ALLOT M\ 33 DTST
        EXIT
    THEN

    OP1 @ @ FFFFFF AND 82448D XOR  \ LEA     EAX , 0 [EDX] [EAX*4]
    OP0 @ W@ 0889             XOR OR  \ MOV     [EAX] , ECX
    0=
    IF  M\ 132 DTST
        4C89  OP1 @ W!
        OP1 ToOP0
        FALSE -2 ALLOT M\ 133 DTST
        EXIT
    THEN


OP4 @ 2+ C@
OP3 @ 2+ C@ XOR
OP3 @ 2+ C@
OP0 @ 2+ C@ XOR OR
OP4 @ W@ 558B XOR OR \ 8B5500      MOV     EDX , 0 [EBP]
OP3 @ W@ 4589 XOR OR \  894500     MOV     0 [EBP] , EAX
OP2 @ @ C203D8F7 XOR OR \  F7D8    NEG     EAX
                        \  03C2    ADD     EAX , EDX
OP0 @ W@ 558B XOR OR \ 8B5500      MOV     EDX , 0 [EBP]
0=  IF  M\ C8 DTST
        4D   OP4 @ 1+ C!         \  MOV     ECX , 0 [EBP]
                                 \  MOV     0 [EBP] , EAX
        D08B OP2 @    W!         \  MOV     EDX , EAX
    C103D8F7 OP1 @     !         \  NEG     EAX
                                 \  ADD     EAX , ECX
        FALSE -1 ALLOT M\ C9 DTST
        EXIT
    THEN

OP4 @ W@ 5589 XOR    \  895500            MOV     0 [EBP] , EDX
OP3 @ W@ 558B XOR OR \  8B55FC            MOV     EDX , FC [EBP]
OP2 @ W@ 4D8B XOR OR \  8B4D00            MOV     ECX , 0 [EBP]
0=  IF M\ C8 DTST
       OP3 2 OPinsert
       CA8B  OP4 @ W! \  MOV    ECX , EDX
       OP2 OPexcise
        FALSE M\ C9 DTST
        EXIT
    THEN

DUP C@ C3 XOR
OP3 @ 2+ C@
OP0 @ 2+ C@  XOR OR
OP3 @ W@ 4589 XOR OR \ 571FEA 894500            MOV     0 [EBP] , EAX
OP2 @ W@ C28B XOR OR \ 571FED 8BC2              MOV     EAX , EDX
OP1 @ W@ EAF7 XOR OR \ 571FEF F7EA              IMUL    EDX
OP0 @ W@ 4503 XOR OR \ 571FF1 034500            ADD     EAX , 0 [EBP]
0=  IF M\ 1C8 DTST

       OP2 OPexcise
       OP1 OPexcise

       OP0 3 OPinsert
	AF0F OP1 @ W! 
       D2    OP1 @ 2+ C!  \     IMUL    EDX , EDX

       C203  OP0 @ W! \  ADD    EAX , EDX 
        FALSE
 -1 ALLOT
  M\ 1C9 DTST
        EXIT
    THEN


OP5 @ :-SET U< IF TRUE EXIT THEN

M\ PPPP

 TRUE
;

: -EVEN-EBP
     OP0 @ :-SET U< IF EXIT THEN
     OP0 @ W@ 06D8D =  \  LEA   ebp,  OFF-EBP [EBP]
     IF  OP0 @ 2+ C@ +>OFF-EBP
         OP1 ToOP0
        -3 ALLOT EXIT
     THEN ;


: OPT_  ( -- )
  BEGIN
\   M\ -D DTST
 OPT-RULES UNTIL
\   M\ -F DTST
  EVEN-EAX  ;

: DO_OPT   ( ADDR -- ADDR' )
  OPT? IF OPT_ THEN ;

: INLINE?  ( CFA -- CFA FLAG )
  DUP         BEGIN
  2DUP
  MM_SIZE -   U> 0= IF  DROP FALSE  EXIT THEN
  DUP C@      \  CFA CFA+OFF N'
  DUP   0C3    = IF 2DROP  TRUE EXIT THEN  \ RET
  DUP5B?         M_WL DROP 5 + REPEAT
\ 0100.X0XX
  DUP   0F4
  AND    40    = M_WL DROP 1+  REPEAT \ INC|DEC  E(ACDB)X

  DUP   099    = M_WL DROP 1+  REPEAT  \ CDQ
\ 1110.11XX
OS\ DUP FC
OS\ AND EC    = M_WL DROP 1+  REPEAT  \ IN|OUT  EAX AL, DX | DX, EAX EL
OS\ DUP CD    = M_WL DROP 2+  REPEAT  \ INTX
  DROP
  DUP W@      \  CFA CFA+OFF N'

  DUP3B?[EBP]    M_WL DROP 3 +  REPEAT
  DUP3B?         M_WL DROP 3 +  REPEAT
  DUP 06D8D    = M_WL DROP 3 +  REPEAT \ LEA  EBP, OFF-EBP [EBP]
  DUP 0C583    = M_WL DROP 3 +  REPEAT \ ADD  EBP, # OFF-EBP
  DUP 0ED83    = M_WL DROP 3 +  REPEAT \ SUB  EBP, # X
  DUP2B?         M_WL DROP 2+   REPEAT
  DUP 0EC87    = M_WL DROP 2+   REPEAT \ XCHG    EBP , ESP

  DUP  C58B    = M_WL DROP 2+   REPEAT \ MOV EAX,  EBP

\  DUP 0E3FF   = M_WL DROP 2+   REPEAT   \ JMP  EBX
  DUP6B?         M_WL DROP 6 +  REPEAT
  DUP 45C7  =   M_WL  DROP 7 +  REPEAT  \ MOV     X [EBP] , # X
  DUP 05C7  =   M_WL  DROP A +  REPEAT  \ MOV     X  , # Y
  DUP 0581  =   M_WL  DROP A +  REPEAT  \ ADD     X  , # Y

  DROP
  DUP @
  DUP 0424448B = M_WL DROP 4 + REPEAT
  DUP 0424448D = M_WL DROP 4 + REPEAT
  DUP 8BE08B5B =
   IF OVER 3 + 2@         \ 1 TO TTTT
            E3FF046D 8D00458B  D=   \ RP!
           IF 2DROP TRUE EXIT THEN
   THEN
  FFFFFF AND
  DUP   C09D0F = M_WL DROP 3 + REPEAT \ SETGE  AL
  DUP   C09E0F = M_WL DROP 3 + REPEAT \ SETLE  AL
  DUP   90048D = M_WL DROP 3 + REPEAT \ LEA  EAX , [EAX] [EDX*4]
  DUP   02048D = M_WL DROP 3 + REPEAT \ LEA  EAX, [EDX] [EAX]
  DUP   40B70F = M_WL DROP 4 + REPEAT \ MOVZX   EAX , WORD PTR FE [EAX] 


\ CMPXCHG [EAX] , AL| EAX
\ LSS     EAX , [EAX]
\ BTR     [EAX] , EAX
\ LFS     EAX , [EAX]
\ LGS     EAX , [EAX]
\ MOVZX   EAX , BYTE|WORD  PTR [EAX]
\ 0000.0000 1011.1XXX 0000.1111
  DUP FFF8FF
  AND 00B00F = M_WL DROP 3 + REPEAT \ MOVZX  EAX, WORD PTR [EAX]

  DUP 20488B = M_WL DROP 3 +  REPEAT \ MOV  ECX , [EAX+20H]
  DUP 20488D = M_WL DROP 3 +  REPEAT \ LEA  ECX , [EAX+20H]

  DUP 85448B = M_WL DROP 4 + REPEAT \ MOV    EAX, X [EBP] [EAX*4]
  DUP 02048D = M_WL DROP 4 + REPEAT \ LEA     EAX , [EDX] [EAX]
\ XX00.0101 0000.0100 1000.1XX1
  DUP7B?       WHILE DROP 7 + REPEAT
 2DROP  FALSE
;

: MACRO? INLINE? ;

\  МАКРОПОДСТАНОВЩИК

: +EBP DUP C@ C>S OFF-EBP + C,    1+ ;

: 1_,_STEP SetOP DROP DUP C@ C,    1+ ;

: 2_,_STEP SetOP DROP DUP W@ W,    2+ ;

: 3_,_STEP  2_,_STEP DUP C@ C,    1+ ;

: 4_,_STEP_           DUP @  , CELL+ ;

: 4_,_STEP SetOP DROP 4_,_STEP_ ;

: 5_,_STEP  1_,_STEP 4_,_STEP_ ;

: 6_,_STEP  2_,_STEP 4_,_STEP_ ;

: 7_,_STEP  3_,_STEP 4_,_STEP_ ;

: A_,_STEP  2_,_STEP 4_,_STEP_ 4_,_STEP_ ;

: 0A_,_STEP DUP DUP @ + DP @ - , CELL+ ;

: 1A_,_STEP 1_,_STEP 0A_,_STEP ;

: 2A_,_STEP 2_,_STEP 0A_,_STEP ;

: _INLINE,  (  CFA  --  )
\  ." ^" DUP H.
              BEGIN
  DO_OPT

   DUP @      \  CFA  N'
  DUP 8BE08B5B = DUP
  IF DROP OVER 3 + 2@ E3FF046D 8D00458B  D=  \ RP!
  THEN          M_WL DROP   SetOP
                    8B C, E0 C,          \  MOV  ESP,    EAX
                    DROP 'DROP
                 REPEAT
  FF AND      \  CFA  N'
  DUP   0C3 = IF 2DROP     EXIT THEN  \ RET

  DUP5B?      M_WL  5_,_STEP  REPEAT \ ADD EAX, # X

\ 010X.XXXX
  DUP E0 AND 40 = M_WL 1_,_STEP REPEAT  \ INC|DEC|PUSH|POP  E_X

\ FS: GS: D16: A16: INSB INSD OUTSB OUTSD
\ 0110.X1XX
  DUP   F4 AND 64 = M_WL  1_,_STEP      REPEAT

  DUP   099 = M_WL  1_,_STEP      REPEAT  \ CDQ

OS\ DUP FC
OS\ AND EC = M_WL   1_,_STEP  REPEAT  \ IN|OUT  EAX AL, DX | DX, EAX EL
OS\ DUP CD = M_WL   2_,_STEP  REPEAT  \ INTX

\  DUP   0BB = M_WL  5_,_STEP     REPEAT  \ MOV EBX, # X
\  JO JNO JB JAE JE JNE JBE JA JS JNS JP JNP JL JGE JLE JG
\ 0111.XXXX
  DUP  F0 AND 70 = M_WL  2_,_STEP     REPEAT

  DUP   0E8 = M_WL 1A_,_STEP      REPEAT  \  CALL
  DUP   0E9 = M_WL 1A_,_STEP      REPEAT  \  JMP
  DROP
  DUP W@
  DUP3B?[EBP]  M_WL  2_,_STEP +EBP REPEAT
  DUP3B?       M_WL  3_,_STEP      REPEAT
  DUP2B?       M_WL  2_,_STEP      REPEAT
  DUP 0C48B =  M_WL  2_,_STEP   REPEAT  \ MOV EAX , ESP
  DUP  C58B =  M_WL EVEN-EBP 2_,_STEP   REPEAT \ MOV EAX,  EBP
  DUP 0EC87 =  M_WL EVEN-EBP 2_,_STEP   REPEAT \ XCHG    EBP , ESP

  DUP 06D8D = M_WL DROP DUP 2 + C@ +>OFF-EBP
                     3 + REPEAT  \ LEA  EBP, OFF-EBP [EBP]

  DUP 0C583 = M_WL DROP DUP 2 + C@ +>OFF-EBP
                     3 + REPEAT  \ ADD  EBP, # OFF-EBP

  DUP 0ED83 = M_WL DROP DUP 2 + C@ C>S NEGATE +>OFF-EBP
                     3 + REPEAT  \ SUB  EBP, # OFF-EBP

  DUP 0C483 = M_WL  3_,_STEP     REPEAT  \ ADD  ESP, # X
  DUP6B?      M_WL  6_,_STEP      REPEAT  \ MOV  EAX,  # X
  DUP 0E3FF = M_WL EVEN-EBP 2_,_STEP REPEAT  \ JMP  EBX
\  DUP 0D3FF = M_WL EVEN-EBP 2_,_STEP REPEAT  \ CALL EBX

\ DUP 0E2FF = M_WL EVEN-EBP 2_,_STEP REPEAT  \ JMP  EDX
  DUP 0D2FF = M_WL EVEN-EBP 2_,_STEP REPEAT  \ CALL EDX
  DUP 810F  = M_WL 2A_,_STEP      REPEAT  \ JO [ESP]
  DUP 45C7  = M_WL 2_,_STEP +EBP 4_,_STEP_  REPEAT  \ MOV     X [EBP] , # X
  DUP 05C7  = M_WL A_,_STEP  REPEAT  \ MOV     X  , # Y
  DUP 0581  = M_WL A_,_STEP  REPEAT  \ ADD     X  , # Y

  DROP
  DUP @
  DUP 0424448B = M_WL DROP   SetOP       \  MOV  EAX, 4 [ESP]
                    8B C, 04 C, 24 C,    \  MOV  EAX,   [ESP]
                    4 +
                 REPEAT

  DUP 0424448D = M_WL DROP   SetOP       \  LEA  EAX, 4 [ESP]
                    8B C, C4 C,          \  MOV  EAX,    ESP
                    4 +
                 REPEAT

  FFFFFF AND

  DUP C09D0F = M_WL 3_,_STEP      REPEAT \ SETGE  AL
  DUP C09E0F = M_WL 3_,_STEP      REPEAT \ SETLE  AL
  DUP 90048D = M_WL 3_,_STEP      REPEAT \ LEA  EAX , [EAX] [EDX*4]
  DUP 02048D = M_WL 3_,_STEP      REPEAT \ LEA  EAX, [EDX] [EAX]
  DUP 40B70F = M_WL 4_,_STEP      REPEAT \ MOVZX   EAX , WORD PTR FE [EAX] 

  DUP 240401 = M_WL 3_,_STEP      REPEAT \ ADD [ESP] , EAX

\ CMPXCHG [EAX] , AL| EAX
\ LSS     EAX , [EAX]
\ BTR     [EAX] , EAX
\ LFS     EAX , [EAX]
\ LGS     EAX , [EAX]
\ MOVZX   EAX , BYTE|WORD  PTR [EAX]
\ 0000.0000 1011.1XXX 0000.1111
  DUP FFF8FF
  AND 00B00F = M_WL 3_,_STEP      REPEAT
  DUP 244403 = M_WL 4_,_STEP      REPEAT \ ADD  EAX, X [ESP]
  DUP 24442B = M_WL 4_,_STEP      REPEAT \ SUB  EAX, X [ESP]
  DUP 24448B = M_WL 4_,_STEP      REPEAT \ MOV  EAX, X [ESP]
  DUP 85448B = M_WL 3_,_STEP +EBP REPEAT \ MOV  EAX, X [EBP] [EAX*4]
  DUP 24048B = M_WL 3_,_STEP      REPEAT \ MOV  EAX, 0 [ESP]
  DUP 20488B = M_WL 3_,_STEP      REPEAT \ MOV  ECX , [EAX+20H]
  DUP 20488D = M_WL 3_,_STEP      REPEAT \ LEA  ECX , [EAX+20H]
  DUP 021C8D = M_WL 3_,_STEP      REPEAT \ LEA  EBX, [EDX] [EAX]
  DUP 85048D = M_WL 7_,_STEP      REPEAT \ LEA  EAX, X [EAX*4]
  DUP 2404FF = M_WL 3_,_STEP      REPEAT \ INC [ESP]
  DUP 18B60F = M_WL 3_,_STEP      REPEAT \ MOVZX EBX, BYTE PTR [EAX]

  DUP7B?      WHILE 7_,_STEP      REPEAT
  HEX U. ." @COD, ERROR" ABORT
;


: XC_J
    J_COD FE AND 4 = IF EXIT THEN
    J_COD DUP 0x8 AND
    IF  3
    ELSE 5
    THEN  XOR TO J_COD ;

[DEFINED] [TTO]
[IF]
PREVIOUS PREVIOUS SET-CURRENT
[THEN]

: OPT_CLOSE
   EVEN-EBP DP @ TO LAST-HERE ;

:  OPT_INIT   ?SET -EVEN-EBP  ;

: INLINE, ( CFA --  )   OPT_INIT  _INLINE, OPT_CLOSE ;

: MACRO, INLINE, ;


:  ?BR-OPT-RULES ( cfa -- cfa' flag )
\ ZZZZ IF ." z=" OP1 @ @ U. THEN
   OP0 @ :-SET U< IF TRUE EXIT THEN
   M\ -3 DTST

   OP0 @ C@ 05 =    \ ADD  EAX, # X
   IF  M\ 404 DTST
       OP0 @ 1+ @ OFF-EAX + TO OFF-EAX
       OP1 ToOP0
       FALSE  -5 ALLOT M\ 405 DTST
       EXIT
   THEN

   DUP 'DROP XOR
   OFF-EAX   OR
   OP0 @  W@  4589 XOR OR 0= \ MOV X [EBP] , EAX
   IF   M\ 436 DTST
       DP @ TO LAST-HERE INLINE,
        M\ 437 DTST
         ['] NOOP FALSE EXIT
   THEN

   DUP 'DROP =
   OFF-EAX 0=
   OP0 @ C@  A1 =  AND AND  \  MOV     EAX , 44444
   IF M\ 434 DTST
      0 W,
      OP0 @ 1+ @ OP0 @ 2+ !
      3D83  OP0 @ W!     \  CMP 44444, # 0
      FALSE  M\ 435 DTST
      EXIT
   THEN

   OFF-EAX 0=
   OP0 @  @  FFFFFF AND 24048B = AND \ MOV     EAX , [ESP]
   IF M\ 534 DTST
      1 ALLOT
      243C83   OP0 @ !     \    CMP  DWORD PTR [ESP], 0
      FALSE  M\ 535 DTST
      EXIT
   THEN

   DUP 'DROP XOR
   OP0 @ W@  458B XOR OR 0= \  MOV     EAX , 0 [EBP]
   IF M\ 334 DTST
      7D83  OP0 @ W!     \  CMP X [EBP], # Z
      FALSE 0 C, M\ 335 DTST
      EXIT
   THEN
   DUP 'DROP XOR
   OP0 @ W@ C20B XOR OR \  0BC2              OR      EAX , EDX
0= IF M\ B4 DTST
      D00B  OP0 @ W!     \  OR      EDX , EAX
      FALSE  M\ B5 DTST
      EXIT
   THEN

   OP0 @ W@  408D =  \  LEA   EAX,  X [EAX]
   IF M\ 234 DTST
      C083  OP0 @ W!     \  ADD  EAX, # X
      FALSE M\ 235 DTST
      EXIT
   THEN
   OP0 @ W@  808D =  \  LEA   EAX,  X [EAX]
   IF M\ 34 DTST
      05 OP0 @ C!
      OP0 @ 2+ @ OP0 @ 1+ !   \  ADD  EAX, # X
      TRUE  -1 ALLOT M\ 35 DTST
      EXIT
   THEN

   DUP 'DROP XOR
   OP0 @ @ FFFFFF AND 1FF8C1 XOR OR 0= \  SAR  EAX , 1F
   IF  M\ 13A DTST
            OP1 ToOP0
            8D J_COD 1 AND XOR  TO J_COD
            FALSE  -3 ALLOT M\ 13B DTST
            EXIT
   THEN
   DUP 'DROP XOR
   OP0 @ C@ 35  XOR OR 0= \   XOR  EAX, # X
   IF M\ 134 DTST
      3D OP0 @ C!   \  CMP  EAX, # X
      FALSE M\ 135 DTST
      EXIT
   THEN

   DUP 'DROP XOR
 OP0 @ @ FFFFFF AND 240433 XOR OR 0= \ XOR     EAX , [ESP]
   IF M\ 134 DTST
      3B OP0 @ C!   \  CMP     EAX , [ESP]
      FALSE  M\ 135 DTST
      EXIT
   THEN

 OP0 @ W@  4539 XOR     \ CMP    X [EBP] , EAX
0= IF M\ 76 DTST
      3B OP0 @ C!   \  CMP  EAX,  X [EBP]
      XC_J
      FALSE  M\ 77 DTST
      EXIT
   THEN

   OP1 @ :-SET U< IF TRUE EXIT THEN
\ $       0<> IF
   DUP 'DROP XOR
   OP1 @ @  C01BD8F7  XOR OR 0= \   NEG  EAX  \  SBB  EAX, EAX
        IF   M\ 36 DTST
             OP2 ToOP0
\            084 TO J_COD
             FALSE  -4 ALLOT M\ 37 DTST
             EXIT
        THEN


\ $       0= IF
        DUP 'DROP XOR
        OP1 @  @  1B01E883 XOR OR     \ SUB EAX , # 1
        OP0 @ W@      C01B XOR OR 0=  \ SBB EAX , EAX
        IF  M\ 38 DTST
\            OP2 @ W@ U.
            OP2 ToOP0
            J_COD 1 XOR TO J_COD
            FALSE  -5 ALLOT M\ 39 DTST
            EXIT
        THEN


 \ $  U< IF
        DUP 'DROP XOR
        OP1 @  C@  3D <>
        OP1 @  C@  3B <>   AND
\        OP1 @  W@ 053B <> AND
\        OP1 @  W@ D03B <> AND       \ CMP     EDX , EAX
        OP1 @   @ FFFD AND
                  4539 <> AND      \ CMP    X [EBP] , EAX
        OP0 @ W@  C01B XOR OR OR 0=   \ SBB    EAX , EAX
        IF  M\ 3A DTST
            OP1 ToOP0
            83 J_COD 1 AND XOR  TO J_COD
            FALSE -2 ALLOT M\ 3B DTST
            EXIT
        THEN

 OP1 @ @ FFFFFF AND 240C8B XOR \ MOV     ECX , [ESP]
 OP0 @ W@ C13B XOR OR 0= \	CMP     EAX , ECX
        IF  M\ 33A DTST
            043B  OP1 @ W!
            OP1 ToOP0
            FALSE -2 ALLOT M\ 33B DTST
            EXIT
        THEN

 OP1 @ @ FFFFFF AND 240C8B XOR \ MOV     ECX , [ESP]
 OP0 @ W@ C133 XOR OR 0=  \	XOR     EAX , ECX
        IF  M\ 33A DTST
            0433  OP1 @ W! \ XOR     ECX , [ESP]
            OP1 ToOP0
            FALSE -2 ALLOT M\ 33B DTST
            EXIT
        THEN

  OP1 @ @  C13BCA8B = \	MOV     ECX , EDX    CMP   EAX , ECX
        IF  M\ 33A DTST
            C23B  OP1 @ W!    \	CMP     EAX , EDX
            OP1 ToOP0
            FALSE -2 ALLOT M\ 33B DTST
            EXIT
        THEN

 OP1 @ W@ 558B XOR       \ MOV     EDX , 0 [EBP]
 OP0 @ W@ C23B XOR OR 0= \ CMP     EAX , EDX
        IF  M\ 43A DTST
            453B  OP1 @ W! \ CMP     EAX , 0 [EBP]
            OP1 ToOP0
            FALSE -2 ALLOT M\ 43B DTST
            EXIT
        THEN


      DUP 'DROP XOR
  OP1 @ @ C83B008B XOR OR   \	MOV     EAX , [EAX]   CMP     ECX , EAX
0=  IF   M\ 43A DTST
            083B  OP1 @ W! \ CMP     ECX ,  [EAX]
            OP1 ToOP0
            FALSE -2 ALLOT M\ 43B DTST
            EXIT
    THEN

      DUP 'DROP XOR
  OP1 @ @ D03B008B XOR OR \ MOV     EAX , [EAX]   CMP     EDX , EAX
0=  IF   M\ 43A DTST
            103B  OP1 @ W! \ CMP     EDX ,  [EAX]
            OP1 ToOP0
            FALSE -2 ALLOT M\ 43B DTST
            EXIT
    THEN
      DUP 'DROP XOR
OP1 @ @ C13B008B XOR OR   \  MOV     EAX , [EAX]   CMP     EAX , ECX
0=  IF   M\ 80 DTST
            083B  OP1 @ W! \ CMP     ECX ,  [EAX]
            XC_J
            OP1 ToOP0
            FALSE -2 ALLOT M\ 81 DTST
            EXIT
    THEN

OP1 @ C@   B9 XOR    \  MOV     ECX , # 3
OP0 @ W@ C133 XOR OR \  XOR     EAX , ECX
0= IF   M\ 40 DTST
       3D  OP1 @ C!  \  CMP     EAX , # 1
       OP1 ToOP0
       FALSE  -2 ALLOT M\ 41 DTST
       EXIT
   THEN
\ ZZZZ IF ." Z=" OP1 @ @ U. OP1 @ W@ U. OP0 @ W@ U. THEN
OP1 @ W@ 558B XOR \      MOV     EDX , 0 [EBP]
OP0 @ W@ C23B XOR OR \   CMP     EAX , EDX
0= IF   M\ 68 DTST
       453B  OP1 @ C!  \   CMP     EAX , 0 [EBP]
       OP1 ToOP0
       FALSE  -2 ALLOT M\ 69 DTST
       EXIT
   THEN
OP1 @ C@ B9 XOR  \ 57B523 B901000000  MOV     ECX , # 1
OP0 @ W@ C83B XOR OR \ 57B528 3BC8              CMP     ECX , EAX
0= IF   M\ 74 DTST
       3D  OP1 @ C!  \   CMP     EAX , # 1
       XC_J
       OP1 ToOP0
       FALSE  -2 ALLOT M\ 75 DTST
       EXIT
   THEN

OP1 @  @ C23B008B = \   MOV     EAX , [EAX]           CMP     EAX , EDX
    IF   M\ 78 DTST
            103B  OP1 @ W! \ CMP     EDX , [EAX]
            XC_J
            OP1 ToOP0 -2 ALLOT
            FALSE M\ 79 DTST
            EXIT
    THEN

      DUP 'DROP XOR
OP1 @ @ FFFFFF AND 24448B XOR OR \        MOV     EAX , 10 [ESP]
OP0 @ C@ 3D XOR OR \ 597707 3D02000000  CMP     EAX , # 2
0=  IF   M\ 7A DTST
            7C81  OP1 @ W! \ CMP     10 [ESP] , # 2
            OP0 @ 1+ @ OP0 @ !
            OP1 ToOP0 -1 ALLOT
            FALSE M\ 7B DTST
            EXIT
    THEN

OP1 @ @ FFFFFF AND 244C8B XOR \        MOV     ECX , 20 [ESP]
OP0 @ W@ C83B XOR OR \              CMP     ECX , EAX
0=  IF   M\ 7C DTST
            443B  OP1 @ W! \ CMP   EAX ,  20 [ESP]
            OP0 @ 1+ @ OP0 @ !
            XC_J
            OP1 ToOP0 -2 ALLOT
            FALSE M\ 7D DTST
            EXIT
    THEN

OP1 @ W@ 0D8B XOR \        MOV     ECX , 44444
OP0 @ W@ C83B XOR OR \     CMP     ECX , EAX
0=  IF   M\ 7C DTST
            053B  OP1 @ W! \ CMP   EAX ,  44444
            OP0 @ 1+ @ OP0 @ !
            XC_J
            OP1 ToOP0 -2 ALLOT
            FALSE M\ 7D DTST
            EXIT
    THEN

 DUP 'DROP XOR
OP1 @ @ FFFFFF AND 85048B XOR OR \   MOV     EAX , X [EAX*4]
OP0 @ C@ 3D XOR OR \ 5960F8 3500100000  CMP     EAX , # 1000
0=  IF   M\ 86 DTST
            3C81  OP1 @ W! \  CMP   X [EAX*4] ,  # 1000
            OP0 @ 1+ @ OP0 @ !
            OP1 ToOP0 -1 ALLOT
            FALSE M\ 87 DTST
            EXIT
    THEN

 DUP 'DROP XOR
OP1 @ C@ A1 XOR OR \ 5978EF A135755900  MOV     EAX , 597535
OP0 @ C@ 3D XOR OR \ 5978F4 3D41000000  CMP     EAX , # 1000
0=  IF   M\ 96 DTST
            OP1 @ 1+ @ OP1 @ 2+ !
            3D81  OP1 @ W! \  CMP   597535 ,  # 1000
            OP1 ToOP0
            FALSE M\ 97 DTST
            EXIT
    THEN

 DUP 'DROP XOR
OP1 @ W@ D8F7 XOR OR \ 581E16 F7D8              NEG     EAX
OP0 @ W@ C203 XOR OR \ 581E18 03C2              ADD     EAX , EDX
0=  IF   M\ B8 DTST
         D02B  OP1 @ W! \  SUB     EDX , EAX
         OP1 ToOP0 -2 ALLOT
         FALSE M\ B9 DTST
            EXIT
    THEN

OP1 @ @ C90BCA8B XOR	\  8BCA           MOV     ECX , EDX
			\  0BC9           OR      ECX , ECX
0=  IF   M\ BA DTST
         D20B  OP1 @ W! \  OR      EDX , EDX
         OP1 ToOP0 -2 ALLOT
         FALSE M\ BB DTST
            EXIT
    THEN

OP1 @ @  3DFF408D XOR
OP0 @ 1+ @ OR	\ 	LEA     EAX , FF [EAX] 	CMP     EAX , # 0 
0= IF   M\ 1BA DTST
         48 OP1 @ C!	\	DEC     EAX
         OP1 ToOP0 -7 ALLOT
         FALSE M\ 1BB DTST
            EXIT
    THEN

   OP2 @ :-SET U< IF TRUE EXIT THEN

OP2 @ W@  4589 XOR         \  MOV     F8 [EBP] , EAX
OP1 @  @ 453BC033 XOR OR   \ XOR     EAX , EAX CMP     EAX , F8 [EBP]
OP0 @ 2+ C@
OP2 @ 2+ C@   XOR OR  \  (FALG &( X1=X ))
0=  IF   M\ 4E DTST
            3D  OP1 @ C! OP1 @ 1+ 0! \ CMP     EAX ,  # 0
            XC_J
            OP1 ToOP0
            FALSE M\ 4F DTST
            EXIT
    THEN

OP2 @ W@ 5589 XOR    \    MOV     F8 [EBP] , EDX
OP1 @  @ 453B008B XOR OR \   MOV     EAX , [EAX]    CMP     EAX , F8 [EBP]
OP0 @ 2+ C@
OP2 @ 2+ C@   XOR OR  \  (FALG &( X1=X ))
\ OP0 @ 2+ C@ C>S  OFF-EBP > OR   !!!!
0=  IF   M\ 23A DTST
         C23B OP0 @ W!  \    CMP     EAX , EDX
         FALSE -1 ALLOT M\ 23B DTST
         EXIT
    THEN

OP2 @ W@ 4D89 XOR  \      MOV     F8 [EBP] , ECX
OP1 @  @ 453B008B XOR OR \   MOV     EAX , [EAX]    CMP     EAX , F8 [EBP]
OP0 @ 2+ C@
OP2 @ 2+ C@   XOR OR  \  (FALG &( X1=X ))
0=  IF   M\ 7E DTST
         C13B OP0 @ W!  \    CMP     EAX , ECX
         FALSE -1 ALLOT M\  7F DTST
         EXIT
    THEN


OP1 @ W@   8B XOR    \    MOV     EAX , [EAX]
OP0 @ W@ 4539 XOR OR \    CMP     F8 [EBP] , EAX
OP0 @ 2+ C@
OP2 @ 2+ C@   XOR OR  \  (FALG &( X1=X ))
\ OP0 @ 2+ C@ C>S  OFF-EBP > OR
0=  IF
      OP2 @ W@ 4D89 =    \    MOV     F8 [EBP] , ECX
      IF  M\ 23A DTST
         C83B OP0 @ W!  \  CMP     ECX , EAX
         FALSE -1 ALLOT M\ 23B DTST
         EXIT
      THEN
      OP2 @ W@ 5589 =    \    MOV     F8 [EBP] , EDX
       \ MOV     EAX , [EAX]    CMP     F8 [EBP] , EAX
      IF  M\ 33A DTST
         D03B OP0 @ W!  \  CMP     EDX , EAX
         FALSE -1 ALLOT M\ 33B DTST
         EXIT
      THEN
    THEN


   OP3 @ :-SET U< IF TRUE EXIT THEN

\ $  < IF
     DUP 'DROP XOR
     OP2 @ @ FFFFFCFF AND  83C09C0F XOR OR
     OP1 @ @  4801E083 XOR OR 0=
\ 0F9DC0		SETGE   AL
\ 83E001		AND     EAX , # 1
\ 48		DEC     EAX
     IF  M\ 3C DTST
             OP2 @ 1+ C@ 10 - J_COD 1 AND XOR TO J_COD
             OP3 ToOP0
             FALSE  -7 ALLOT M\ 3D DTST
             EXIT
     THEN

\ 5 OVER = IF
     OP0 @ W@ 4533 =  \ XOR   EAX , F8 [EBP]
     IF
         DUP 'DROP XOR
         OP3 @ W@ 4589 XOR OR  \ MOV    FC [EBP] , EAX
         OP2 @ W@ 45C7 XOR OR  \ MOV    F8 [EBP] , # 5
         OP1 @ W@ 458B XOR OR  \ MOV    EAX , FC [EBP]
         OP3 @ 2+ C@ OP1 @ 2+ C@ XOR OR
         OP2 @ 2+ C@ OP0 @ 2+ C@ XOR OR 0=
         IF M\ 234 DTST
            3D OP2 @ C!
            OP2 @ 3 + @ OP2 @ 1+ !
            OP2 ToOP0
            TRUE -8 ALLOT  M\ 235 DTST
            EXIT
         THEN
     THEN

     TRUE  ;

: BR-EVEN-EAX OFF-EAX
   IF   M\ 1002 DTST
       SetOP
      5 C, OFF-EAX ,
      0 TO OFF-EAX
         M\ 1003 DTST
   THEN
;


: ?BR-OPT
     BEGIN BEGIN  M\ -9 DTST
                 ?BR-OPT-RULES
           UNTIL  M\ -7 DTST
                  ['] NOOP OPT-RULES NIP
     UNTIL        BR-EVEN-EAX
   M\ -5 DTST
\ ZZZZ IF ." B=" DUP U. THEN
        OP0 @ :-SET U<
        IF    SetOP 0xC00B W,    \ OR EAX, EAX
              EXIT
        THEN
        OP0 @ C@
        \ 00XX.X101
        DUP   C7 AND 05 <>  \  ADD|OR|ADC|SBB|AND|SUB|XOR|CMP EAX, # X
\       OVER  3D <> AND   \ ~ CMP EAX, # X
        OVER  40 <> AND   \ DEC     EAX
        OVER  48 <> AND   \ INC     EAX
        OVER  3B <> AND   \ CMP
        OVER  0B <> AND   \ OR

        NIP
        OP0 @  W@
        DUP  4000  OR  ADD|XOR|OR|AND= INVERT
        OVER C01B  <> AND  \ SBB EAX, EAX
        OVER D02B  <> AND  \ SUB EDX, EAX
        OVER 4539  <> AND  \ CMP X [EBP], EAX
\        OVER 103B  <> AND  \ CMP EDX , [EAX]
\        OVER 453B  <> AND  \ CMP EAX , X [EBP]
\        OVER 053B  <> AND  \ CMP EAX , X
        OVER C083  <> AND  \ ADD EAX , # X
        OVER C203  <> AND  \ ADD EAX , EDX
        OVER C133  <> AND  \ XOR EAX , ECX
        OVER F8C1  <> AND  \ SAR EAX ,   X
        OVER 7D83  <> AND  \ CMP X [EBP], # Z
        OVER 3D81  <> AND  \ CMP 44444, # 55555
        OVER 3C81  <> AND  \ CMP   X [EAX*4] ,  # 55555
        OVER 7C81  <> AND  \ CMP X [ESP] , # X
        OVER 3D83  <> AND  \ CMP 44444, # 0
        OVER 3C83  <> AND  \ CMP [ESP], # 0
\        OVER C20B  <> AND  \  OR EAX , EDX
        NIP
        OP0 @  @ FFFFFF AND 240433 XOR \ XOR     EAX , [ESP]
        AND
        AND
        IF    SetOP 0xC00B W,    \ OR EAX, EAX
        THEN
\       DP @ TO :-SET
;

: ??BR-OPT
  OPT?  IF OPT_INIT
 ?BR-OPT
   M\ -B DTST
 OPT_CLOSE
       THEN ;
: ???BR-OPT
  OPT? 0= IF  C00B W,    \ OR EAX, EAX
  THEN
    FALSE TO ~BR-OPT  'DROP ??BR-OPT INLINE,
     TRUE TO ~BR-OPT
   OPT?
  IF   OPT_INIT
   OP0 @ W@ C00B = DUP >R IF OP0 OPexcise THEN
      ['] NOOP ?BR-OPT DROP
\         SetOP 0xC00B W,    \ OR EAX, EAX
    OP0 @ W@ C00B = R> 0= AND IF OP0 OPexcise THEN
      OPT_CLOSE
  THEN
;

: OPT   ( -- )  ['] NOOP DO_OPT DROP  ;

: FORLIT, ( N -- )
  'DUP _INLINE, SetOP 0B8 C, , OPT ;

: CON>LIT ( CFA -- CFA TRUE | FALSE )
                  OPT? 0= IF TRUE EXIT THEN ?SET
               MM_SIZE 0= IF TRUE EXIT THEN
                 DUP C@ 0E8 <> IF TRUE EXIT THEN

                 DUP 1+  REL@ CELL+
                 DUP   CREATE-CODE  =
                 IF  DROP OPT_INIT 5 + [>T] FORLIT, FALSE OPT_CLOSE EXIT
                 THEN

                 DUP     USER-CODE  =
                 IF  DROP  OPT_INIT 'DUP _INLINE,
                   SetOP  878D W, 5 + @ , OPT   FALSE  OPT_CLOSE  EXIT
                 THEN

                 DUP     USER-VALUE-CODE  =
                 IF  DROP  OPT_INIT 'DUP _INLINE,
                   SetOP  878B W, 5 + @ , OPT   FALSE  OPT_CLOSE  EXIT
                 THEN

                 DUP  CONSTANT-CODE  =
                 IF  DROP  OPT_INIT 5 + DUP 5 +  REL@
                     TOVALUE-CODE CELL- =
                    IF   'DUP _INLINE, SetOP 0A1 C, [>T] , OPT
                    ELSE    @  FORLIT,
                    THEN   FALSE  OPT_CLOSE  EXIT
                 THEN
                 DUP 1+ REL@ CELL+ DOES-CODE =
                 IF  5 +         \ CFA
                     SWAP 5 + OPT_INIT FORLIT,
                     TRUE     OPT_CLOSE EXIT
                 THEN
                 DUP  TOUSER-VALUE-CODE =
                 IF  DROP  OPT_INIT
                     SetOP  8789 W, CELL- @ , OPT
                     'DROP _INLINE,
                     FALSE  OPT_CLOSE  EXIT
                 THEN
                  TOVALUE-CODE =
                 IF  OPT_INIT
                     SetOP  A3 C,  CELL- [>T] ,   OPT
                     'DROP _INLINE,
                     FALSE  OPT_CLOSE  EXIT
                 THEN
  TRUE
;

: J?_STEP  ( ADR OPX -- ADR OPX+4 FALSE | OPX TRUE TRUE | ADR FALSE TRUE )


     OVER J-SET U> 0= IF   DROP FALSE TRUE EXIT THEN
     OVER DP @ =   IF   NIP  TRUE TRUE EXIT THEN
     DUP @ @ FFFFF0FF AND 800F =  IF  DROP  FALSE TRUE EXIT THEN
     DUP @ @ 000000E9 =  IF  DROP  FALSE TRUE EXIT THEN
     DUP CELL+ OP0 OpBuffSize + U>
                   IF   DROP FALSE TRUE EXIT THEN
     2DUP @ =      IF  NIP CELL+ TRUE TRUE EXIT THEN
     CELL+ FALSE
;

: J_+!
        DUP C@ F0
           AND 70 = IF 1+ ELSE
        DUP C@ EB = IF 1+ ELSE
        DUP C@ E9 = IF 1+ ELSE
        DUP W@ F0FF
         AND 800F = IF 2+  ELSE
          ." J_+! ERR" ABORT
        THEN  THEN THEN THEN  +!
;

: J_MOVE ( OPX n -- )
  OVER OP0 <>
  IF
      OVER CELL- @
      2DUP - NEGATE
      OVER DP @  - NEGATE ( U. U. U.  ABORT )  CMOVE
      OVER OP0
      ?DO  DUP NEGATE I +!
           I @ C@ E8 = IF DUP I @ 1+ +! THEN
           CELL
      +LOOP
  THEN
      OVER @
      JP0 JpBuffSize + JP0
      ?DO I @
           IF   DUP  I @ U<
                IF
                   OVER NEGATE I +!
                    DUP I @ J_@ U>
                    IF OVER I @ J_+!
                    THEN
                ELSE
                   DUP  I @ <>
                   IF
                     DUP I @ J_@ U<
                     IF OVER NEGATE  I @ J_+!
                     THEN
                   THEN
                THEN
           THEN  CELL
      +LOOP DROP
 \ THEN

  NIP NEGATE DUP ALLOT  :-SET + TO :-SET EXIT
   ;

 TRUE VALUE J_OPT?


\ FALSE VALUE J_OPT?

: RESOLVE_OPT ( ADR -- )

    OPT?  0= IF DROP EXIT THEN
  J_OPT?  0= IF DROP EXIT THEN

  DUP CELL- JP0 JpBuffSize + CELL- @ U<
    IF DUP CELL- REL@ CELL+ J-SET UMAX TO J-SET THEN

 \ ." J_S"  \ BASE @ HEX  J-SET U. DP @ U. BASE !
  DP @ OVER - 7E >  IF ( ." S" )    DROP EXIT THEN
  DP @ LAST-HERE <> IF ( ." L" ) ?SET DROP EXIT THEN
  OPT? 0= IF DROP EXIT THEN
  CELL+  OP0
    BEGIN J?_STEP
    UNTIL
    IF DUP  @
       DUP C@ E9 =
       IF  EB SWAP C! 3
       ELSE
             DUP 1+ W@ 10 - \ 400 +
            SWAP W!  4
       THEN
\       OVER CELL- @ REST
      J_MOVE DP @ TO LAST-HERE EXIT
    THEN  \ OPX
  DROP

;
\  0 TO J_OPT?

BASE !


