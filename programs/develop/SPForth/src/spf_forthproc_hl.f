( Оставшиеся слова "форт-процессора" в виде высокоуровневых определений.
  Copyright [C] 1992-1999 A.Cherezov ac@forth.org
  Преобразование из 16-разрядного в 32-разрядный код - 1995-96гг
  Ревизия - сентябрь 1999
)

0 CONSTANT FALSE ( -- false ) \ 94 CORE EXT
\ Вернуть флаг "ложь".

-1 CONSTANT TRUE ( -- true ) \ 94 CORE EXT
\ Вернуть флаг "истина", ячейку со всеми установленными битами.

4 CONSTANT CELL

: */ ( n1 n2 n3 -- n4 ) \ 94
\ Умножить n1 на n2, получить промежуточный двойной результат d.
\ Разделить d на n3, получить частное n4.
  */MOD NIP
;

: CHAR+ ( c-addr1 -- c-addr2 ) \ 94
\ Прибавить размер символа к c-addr1 и получить c-addr2.
  1+
;
: CHARS ( n1 -- n2 ) \ 94
\ n2 - размер n1 символов.
; IMMEDIATE

: MOVE ( addr1 addr2 u -- ) \ 94
\ Если u больше нуля, копировать содержимое u байт из addr1 в addr2.
\ После MOVE в u байтах по адресу addr2 содержится в точности то же,
\ что было в u байтах по адресу addr1 до копирования.
  >R 2DUP SWAP R@ + U< \ назначение попадает в диапазон источника или левее
  IF 2DUP U<           \ И НЕ левее
     IF R> CMOVE> ELSE R> CMOVE THEN
  ELSE R> CMOVE THEN ;

: ERASE ( addr u -- ) \ 94 CORE EXT
\ Если u больше нуля, очистить все биты каждого из u байт памяти,
\ начиная с адреса addr.
  0 FILL ;

: BLANK ( addr len -- )     \ fill addr for len with spaces (blanks)
  BL FILL ;

: DABS ( d -- ud ) \ 94 DOUBLE
\ ud абсолютная величина d.
  DUP 0< IF DNEGATE THEN
;

255 CONSTANT MAXCOUNTED   \ maximum length of contents of a counted string

\ : 0X BASE @ HEX >R BL WORD ?LITERAL
\      R> BASE ! ; IMMEDIATE
: "CLIP"        ( a1 n1 -- a1 n1' )   \ clip a string to between 0 and MAXCOUNTED
                MAXCOUNTED AND ;

: PLACE         ( addr len dest -- )
                SWAP "CLIP" SWAP
 2DUP C! CHAR+ SWAP CHARS MOVE ;

: +PLACE        ( addr len dest -- ) \ append string addr,len to counted
\ string dest
    >R "CLIP" MAXCOUNTED  R@ C@ -  MIN R>
\ clip total to MAXCOUNTED string
    2DUP 2>R

    COUNT CHARS + SWAP MOVE
    2R> +! ;

: C+PLACE       ( c1 a1 -- )    \ append char c1 to the counted string at a1
                DUP 1+! COUNT + 1- C! ;

: STR>R ( addr u -- addr1 u)
\ Положить строку addr u на стек возвратов
\ Возвратить addr1 адрес новой строки
;

0  VALUE  DOES-CODE

: $!         ( addr len dest -- )
   PLACE ;

: ASCII-Z     ( addr len buff -- buff-z )        \ make an ascii string
   DUP >R $! R> COUNT OVER + 0 SWAP C! ;

: 0MAX 0 MAX ;

: ASCIIZ>  ZCOUNT ;

: R>     ['] C-R>    INLINE, ;   IMMEDIATE
: >R     ['] C->R    INLINE, ;   IMMEDIATE

: 2CONSTANT  ( d --- )
\ Create a new definition that has the following runtime behavior.
\ Runtime: ( --- d) push the constant double number on the stack. 
  CREATE HERE 2! 8 ALLOT DOES> 2@ ;

: U/MOD 0 SWAP UM/MOD ;

: 2NIP 2SWAP 2DROP ;

: ON TRUE SWAP ! ;
: OFF ( a--) 0! ;
