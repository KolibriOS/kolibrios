( Парсер строки с исходным текстом программы на Форте.
  Copyright [C] 1992-1999 A.Cherezov ac@forth.org
  Сентябрь 1999: PARSE и SKIP преобразованы из CODE
  в высокоуровневые определения. Переменные преобразованы в USER.
)

512  VALUE  C/L \ максимальный размер строки, которую можно ввести в TIB

: SOURCE ( -- c-addr u ) \ 94
\ c-addr - адрес входного буфера. u - количество символов в нем.
  TIB #TIB @
;

: SOURCE! ( c-addr u -- ) 
\ установить  c-addr u входным буфером (точнее, областью разбора - PARSE-AREA)
  #TIB ! TO TIB >IN 0! ;

: EndOfChunk ( -- flag )
  >IN @ SOURCE NIP < 0=        \ >IN не меньше, чем длина чанка
;

: CharAddr ( -- c-addr )
  SOURCE DROP >IN @
\ CR ." CA=" DEPTH .SN
 +
;

: PeekChar ( -- char )
  CharAddr C@       \ символ из текущего значения >IN
;

: IsDelimiter ( char -- flag )
  BL 1+ <
;

: GetChar ( -- char flag )
  EndOfChunk
  IF 0 FALSE
  ELSE PeekChar TRUE THEN
;

: OnDelimiter ( -- flag )
  GetChar SWAP IsDelimiter AND
;

: SkipDelimiters ( -- ) \ пропустить пробельные символы
  BEGIN
    OnDelimiter
  WHILE
    >IN 1+!
  REPEAT >IN @  >IN_WORD ! ;

: OnNotDelimiter ( -- flag )
  GetChar SWAP IsDelimiter 0= AND
;

: SkipWord ( -- ) \ пропустить непробельные символы
  BEGIN
    OnNotDelimiter
  WHILE
    >IN 1+!
  REPEAT
;
: SkipUpTo ( char -- ) \ пропустить до символа char
  BEGIN
    DUP GetChar \ ." SC="  DUP M.
  >R <> R> AND
  WHILE
    >IN 1+!
  REPEAT DROP
;

: ParseWord ( -- c-addr u )
  CharAddr \ CR ." P=" DUP 9 TYPE
 >IN @
\ CR ." XZ=" DEPTH .SN
  SkipWord  >IN @
\ CR ." X1=" DEPTH .SN
 - NEGATE
\ CR ." X2=" DEPTH .SN
\ CR ." PZ=" 2DUP  TYPE
;
CREATE UPPER_SCR  31 ALLOT

: UPC  ( c -- c' )
   DUP [CHAR] Z U>
   IF  0xDF AND
   THEN   ;

: UPPER ( ADDR LEN -- )
  0 ?DO COUNT UPC OVER 1- C! LOOP DROP ;

: UPPER_NW  ( ADDR LEN -- ADDR' LEN )
   UPPER_SCR PLACE 
   UPPER_SCR COUNT 2DUP UPPER ;

: PARSE-WORD  ( "name" -- c-addr u )
 \ http://www.complang.tuwien.ac.at/forth/ansforth/parse-word.html 
 \ это слово теперь будем использовать в INTERPRET
  \ - удобнее: не использует WORD и, соответственно, не мусорит в HERE;
  \ и разделителями считает все что <=BL, в том числе TAB и CRLF
  SkipDelimiters ParseWord
  >IN 1+! \ пропустили разделитель за словом
\  UPPER_V @ EXECUTE
;

: NextWord PARSE-WORD ;
: PARSE-NAME PARSE-WORD ;

: PARSE ( char "ccc<char>" -- c-addr u ) \ 94 CORE EXT
\ Выделить ccc, ограниченное символом char.
\ c-addr - адрес (внутри входного буфера), и u - длина выделенной строки.
\ Если разбираемая область была пуста, результирующая строка имеет нулевую
\ длину.
  CharAddr >IN @
  ROT SkipUpTo
  >IN @ - NEGATE
  >IN 1+!
;

: PSKIP ( char "ccc<char>" -- )
\ Пропустить разделители char.
  BEGIN
    DUP GetChar >R = R> AND
  WHILE
    >IN 1+!
  REPEAT DROP
;
