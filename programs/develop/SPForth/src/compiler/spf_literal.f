( Преобразование числовых литералов при интерпретации.
  ОС-независимые определения.
  Copyright [C] 1992-1999 A.Cherezov ac@forth.org
  Преобразование из 16-разрядного в 32-разрядный код - 1995-96гг
  Ревизия - сентябрь 1999
)

: ?SLITERAL1 ( c-addr u -> ... )
  \ преобразовать строку в число
  0 0 2SWAP
  OVER C@ [CHAR] - = IF 1- SWAP 1+ SWAP TRUE ELSE FALSE THEN >R
  >NUMBER
  DUP 1 > IF ." -?" -2001 THROW THEN \ ABORT" -?"
  IF C@ [CHAR] . <> IF -2002 THROW THEN \ ABORT" -??"
       R> IF DNEGATE THEN
       [COMPILE] 2LITERAL
  ELSE DROP D>S
       R> IF NEGATE THEN
       [COMPILE] LITERAL
  THEN
;
: ?LITERAL1 ( T -> ... )
  \ преобразовать строку в число
  COUNT ?SLITERAL1
;
: HEX-SLITERAL ( addr u -> flag )
  BASE @ >R HEX
  0 0 2SWAP 2- SWAP 2+ SWAP >NUMBER
  ?DUP IF
    1 = SWAP C@ [CHAR] L = AND 0= IF 2DROP FALSE R> BASE ! EXIT THEN
  ELSE DROP THEN
  D>S POSTPONE LITERAL TRUE
  R> BASE !
;

: INCLUDED_S  -2003 THROW
 INCLUDED ;

: ?SLITERAL2 ( c-addr u -- ... )
  ( расширенный вариант ?SLITERAL1:
    если строка - не число, то пытаемся трактовать её
    как имя файла для авто-INCLUDED)
  DUP 1 > IF OVER W@ 0x7830 ( 0x) = 
    IF 2DUP 2>R HEX-SLITERAL IF RDROP RDROP EXIT ELSE 2R> THEN THEN
  THEN
  2DUP 2>R ['] ?SLITERAL1 CATCH
  IF   2DROP 2R>
       OVER C@ [CHAR] " = OVER 2 > AND
       IF 2 - SWAP 1+ SWAP THEN ( убрал кавычки, если есть)
       2DUP + 0 SWAP C!
       ['] INCLUDED_S CATCH
       DUP 2 = OVER 3 = OR OVER 161 = OR ( файл не найден или путь не найден,
       или неразрешенное имя файла)
       IF  -2003 THROW \ ABORT"  -???"
       ELSE  THROW THEN
  ELSE RDROP RDROP
  THEN
;
: ?LITERAL2 ( c-addr -- ... )
  ( расширенный вариант ?LITERAL1:
    если строка - не число, то пытаемся трактовать её
    как имя файла для авто-INCLUDED)
  COUNT ?SLITERAL2
;
