\ Конструкция выбора CASE
\ с учетом возможной вложенности операторов CASE


DECIMAL
VARIABLE   CSP    \ Указатель стека контроля
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
\  SP@ CSP@ <> 37 ?ERROR ( ABORT" Сбой стека по CSP !")
  -CSP
;
: CASE ( -> )
  !CSP
; IMMEDIATE
: OF
  POSTPONE OVER POSTPONE =
  [COMPILE] IF POSTPONE DROP
; IMMEDIATE
: ENDOF
  [COMPILE] ELSE
; IMMEDIATE
: ENDCASE
  POSTPONE DROP BEGIN SP@ CSP@ =
  0=  WHILE  [COMPILE] THEN  REPEAT -CSP
; IMMEDIATE
