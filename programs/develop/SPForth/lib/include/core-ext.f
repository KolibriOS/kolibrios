\ 94 CORE EXT

: .R ( n1 n2 -- ) \ 94 CORE EXT
\ Вывести на экран n1 выравненным вправо в поле шириной n2 символов.
\ Если число символов, необходимое для изображения n1, больше чем n2,
\ изображаются все цифры числа без ведущих пробелов в поле необходимой
\ ширины.
  >R DUP >R ABS
  S>D <# #S R> SIGN #>
  R> OVER - 0 MAX SPACES TYPE
;
: 0> ( n -- flag ) \ 94 CORE EXT
\ flag "истина" тогда и только тогда, когда n больше нуля
  0 >
;

: MARKER ( "<spaces>name" -- ) \ 94 CORE EXT
\ Пропустить ведущие пробелы. Выделить name, ограниченное пробелами.
\ Создать определение с семантикой выполнения, описанной ниже.
\ name Выполнение: ( -- )
\ Восстановить распределение памяти словаря и указатели порядка поиска
\ к состоянию, которое они имели перед определением name. Убрать 
\ определение name и все последующие определения. Не требуется 
\ обязательно восстанавливать любые оставшиеся структуры, которые 
\ могут быть связаны с удаленными определениями или освобожденным 
\ пространством данных. Никакая другая контекстуальная информация, 
\ как основание системы счисления, не изменяется.
  HERE
\  [C]HERE , [E]HERE ,
  GET-CURRENT ,
  GET-ORDER DUP , 0 ?DO DUP , @ , LOOP
  CREATE ,
  DOES> @ DUP \ ONLY
\  DUP @ [C]DP ! CELL+
\  DUP @ [E]DP ! CELL+
  DUP @ SET-CURRENT CELL+
  DUP @ >R R@ CELLS 2* + 1 CELLS - R@ 0
  ?DO DUP DUP @ SWAP CELL+ @ OVER ! SWAP 2 CELLS - LOOP
  DROP R> SET-ORDER
  DP !
;

: SAVE-INPUT ( -- xn ... x1 n )  \ 94 CORE EXT
\ x1 - xn описывают текущее состояние спецификаций входного потока для
\ последующего использования словом RESTORE-INPUT.
  SOURCE-ID 0>
  IF TIB #TIB @ 2DUP C/L 2 + ALLOCATE THROW DUP >R SWAP CMOVE
     R> TO TIB  >IN @
     SOURCE-ID FILE-POSITION THROW
     5
  ELSE BLK @ >IN @ 2 THEN
;
: RESTORE-INPUT ( xn ... x1 n -- flag ) \ 94 CORE EXT
\ Попытка восстановить спецификации входного потока к состоянию, 
\ описанному x1 - xn. flag "истина", если спецификации входного 
\ потока не могут быть восстановлены.
\ Неопределенная ситуация возникает, если входной поток, 
\ представленный аргументами не тот же, что и текущий входной поток.
  SOURCE-ID 0>
  IF DUP 5 <> IF 0 ?DO DROP LOOP -1 EXIT THEN
     DROP SOURCE-ID REPOSITION-FILE ?DUP IF >R 2DROP DROP R> EXIT THEN
     >IN ! #TIB ! TO TIB FALSE
  ELSE DUP 2 <> IF 0 ?DO DROP LOOP -1 EXIT THEN
     DROP >IN ! BLK ! FALSE
  THEN
;
: U.R ( u n -- ) \ 94 CORE EXT
\ Вывести на экран u выравненным вправо в поле шириной n символов.
\ Если число символов, необходимое для изображения u, больше чем n,
\ изображаются все цифры числа без ведущих пробелов в поле необходимой
\ ширины.
  >R  U>D <# #S #>
  R> OVER - 0 MAX SPACES TYPE
;
\EOF
: UNUSED ( -- u ) \ 94 CORE EXT
\ u - объем памяти, оставшейся в области, адресуемой HERE,
\ в байтах.
  IMAGE-SIZE
  HERE IMAGE-BASE - -
;