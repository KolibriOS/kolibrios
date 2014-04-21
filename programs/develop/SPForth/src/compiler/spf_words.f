( Печать списка слов словаря - WORDS.
  ОС-независимые определения.
  Copyright [C] 1992-1999 A.Cherezov ac@forth.org
  Преобразование из 16-разрядного в 32-разрядный код - 1995-96гг
  Ревизия - сентябрь 1999
)

VARIABLE NNN

: ?CR-BREAK ( NFA -- NFA TRUE | FALSE )
  DUP
  IF DUP ZCOUNT NIP AT-XY? DROP + SCR_WIDTH-S >
     IF CR
        NNN @
        IF    -1 NNN +!  TRUE
        ELSE  ." more?" CR 16 NNN !
              KEY [CHAR] Q <> AND ?DUP 0<>
        THEN
     ELSE TRUE
     THEN
  THEN
;

: NLIST ( A -> )
  @
  CR W-CNT 0!  16 NNN !
  BEGIN  ?CR-BREAK
  WHILE
    W-CNT 1+!
    DUP ID. \ 9 EMIT
	SPACE AT-XY? >R 8 / 1+ 8 * R> AT-XY
    CDR
  REPEAT KEY? IF KEY DROP THEN
  CR CR ." Words: " W-CNT @ U. CR
;

: WORDS ( -- ) \ 94 TOOLS
\ Список имен определений в первом списке слов порядка поиска. Формат зависит 
\ от реализации.
\ WORDS может быть реализован с использованием слов форматного преобразования 
\ чисел. Соответственно, он может испортить перемещаемую область, 
\ идентифицируемую #>.
  CONTEXT @ NLIST
;

