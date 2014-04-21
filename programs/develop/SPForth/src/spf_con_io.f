( Консольный ввод-вывод.
  ОС-независимые слова [относительно...].
  Copyright [C] 1992-1999 A.Cherezov ac@forth.org
  Преобразование из 16-разрядного в 32-разрядный код - 1995-96гг
  Ревизия - сентябрь 1999
)
32 VALUE BL ( -- char ) \ 94
\ char - значение символа "пробел".

: SPACE ( -- ) \ 94
\ Вывести на экран один пробел.
  BL EMIT
;

: SPACES ( n -- ) \ 94
\ Если n>0 - вывести на дисплей n пробелов.
  BEGIN
    DUP
  WHILE
    BL EMIT 1-
  REPEAT DROP
;

VARIABLE PENDING-CHAR \ клавиатура одна -> переменная глобальная, не USER

VECT DO-KEY?

' NOOP TO DO-KEY?

: KEY?
  EVENT-CASE DO-KEY?
  &KEY @ 0<> ;

\ : KEY?
\ 0 ;

VECT KEY


' _KEY TO KEY

