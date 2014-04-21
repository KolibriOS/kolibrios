[IFNDEF]   CSP
VARIABLE   CSP    \ Указатель стека контроля
[THEN]
6 CONSTANT L-CAS# \ Допустимый уровень вложенности
CREATE     S-CSP   L-CAS# CELLS ALLOT \ Стек контроля
S-CSP CSP !

: +CSP ( -> P)    \ Добавить уровень
  CSP @ DUP CELL+ CSP !
;
: -CSP ( -> )     \ Убрать уровень
  CSP @ 1 CELLS - CSP !
;

: !CSP ( -> )     \ Инициализировать уровень
  SP@ +CSP !
;

: CSP@ ( -> A)
  CSP @ 1 CELLS - @
;
: ?CSP ( -> )     \ Проверить выдержанность стека
  SP@ CSP@ <> 37 ?ERROR ( ABORT" Сбой стека по CSP !")
  -CSP
;
