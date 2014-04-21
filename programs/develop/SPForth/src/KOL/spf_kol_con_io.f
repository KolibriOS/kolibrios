( Консольный ввод-вывод.
)
: NMNM 0 IF THEN ;
: ACCEPT0 ( c-addr +n1 -- +n2 ) \ 94
\ Ввести строку максимальной длины до +n1 символов.
\ Исключительная ситуация возникает, если +n1 0 или больше 32767.
\ Отображать символы по мере ввода.
\ Ввод прерывается, когда получен символ "конец строки".
\ Ничего не добавляется в строку.
\ +n2 - длина строки, записанной по адресу c-addr.
   OVER + 1- OVER      \ SA EA A
 NMNM
   BEGIN KEY          \ SA EA A C
\ ." {"   DUP H. ." }"
     DUP 10 = OVER 13 = OR 0= 
   WHILE
       DUP 0x1B = IF  DROP DUP C@ EMIT   ELSE
       DUP   8  = IF  EMIT BL EMIT 8 EMIT
                     2- >R OVER 1- R> UMAX ELSE
       DUP   9  = IF  DROP DUP 8 BLANK
                     >R OVER R>    \ SA EA SA A
                     SWAP OVER -   \ SA EA SA A-SA
                     8 / 1+ 8 * +  ELSE    DUP EMIT  OVER C!
                THEN THEN
                THEN 1+ OVER UMIN \ SA EA A
   REPEAT \ HEX CR DEPTH .SN
                     \ SA EA A C
   DROP NIP - NEGATE  ;

VECT ACCEPT

' ACCEPT0 TO ACCEPT

: TYPE_M ( c-addr1 u --- )
\ Output the string starting at c-addr and length u to the terminal.
   OVER + SWAP BEGIN 2DUP - WHILE DUP C@ EMIT_N CHAR+ REPEAT
 CC_LINES
 2DROP

 ;

: _TYPE ( c-addr1 u --- )
\ Output the string starting at c-addr and length u to the terminal.
 2DUP SCR_BUF AT-XY? 80 * + +  SWAP CMOVE
 SCR_TYPE
 ;

\ : ZTYPE ( ADDR -- )
\   DUP >R LZTYPE DROP RDROP ;

: _CR ( -- ) \ 94
\ Перевод строки.
 13 EMIT
;

VECT CR ' _CR TO CR

: _EMIT ( x -- ) \ 94
\ Если x - изображаемый символ, вывести его на дисплей.
\  DUP SCR_BUF AT-XY? 80 * + + C!
 DUP 0xD = IF DROP SCR_CR EXIT THEN
 DUP 0x8 = IF DROP 0x00800000
  &AT-XY	W@ 0xD * 0x15 + 16 LSHIFT 0xB OR
  &AT-XY 2+	W@	0x6 * 16 LSHIFT 6	OR
   13 SYS4 DROP
  AT-XY? >R 1- R> AT-XY
 EXIT THEN
  >R RP@ 1 _TYPE RDROP
;


: SWITCH_CHAR ( c1 -- c2 )
  DUP	[CHAR] a    [CHAR] z 1+  WITHIN
  OVER	[CHAR] A    [CHAR] Z 1+  WITHIN OR
  IF   32 XOR  THEN
;


: KEY_M DR_CUR BEGIN 0 ?KEY UNTIL CL_CUR SWITCH_CHAR ;

: _KEY
  CORSOR_DROW
  BEGIN KEY? UNTIL &KEY C@ &KEY 0! SWITCH_CHAR
 ;

: PAGE
 draw_window
 SCR_BUF SCR_WIDTH SCR_HEIGHT * BLANK
 0 0 AT-XY
 DRAW_LINS
;
