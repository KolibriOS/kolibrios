
\ Temporary variables
( 24.09.1997 Черезов А. )
\ April 12th, 2000 - 14:44 Mihail Maksimov
\ добавил конструкции !! ... !! и >| ... | , ликвидировал |DOES

\ оптимизированный вариант. переменные можно использовать и внутри DO LOOP
( 10.06.1999 Ruvim Pinka, idea - Mihail Maksimov )

( Простое расширение СП-Форта локальными переменными.
  Реализовано без использования LOCALS стандарта 94.

  Объявление временных переменных, видимых только внутри
  текущего слова и ограниченных временем вызова данного
  слова выполняется с помощью слова "|" аналогично
  Смолтолку: внутри определения слова используется
  конструкция
  | список локальных переменных через пробел |

  Это заставляет СП-Форт автоматически выделять место в
  стеке возвратов для этих переменных в момент вызова слова
  и автоматически освобождать место при выходе из него.

  Обращение к таким локальным переменным - как к обычным
  переменным по имени и следующими @ и !
  Имена локальных переменных существуют в динамическом
  словаре TEMP-NAMES только в момент компиляции слова, а
  после этого вычищаются и более недоступны.
)
\ Инициализация временных переменных значениями, лежащими на
\ стеке (например, входными параметрами), возможна "списком"
\ с помощью конструкции
\ (( имена инициализируемых локальных переменных ))
\ Имена должны быть ранее объявлены в слове с помощью | ... |

( Использование локальных переменных внутри циклов DO LOOP
  невозможно по причине, описанной в стандарте 94.

  При желании использовать локальные переменные в стиле VALUE-переменных
  можно использовать конструкцию
  || список локальных переменных через пробел ||
  Имена этих переменных будут давать не адрес, а свое значение.
  Соответственно присвоение значений будет осуществляться конструкцией
  -> имя
  по аналогии с присвоением значений VALUE-переменным словом TO.
)

VARIABLE TEMP-CNT
WORDLIST CONSTANT TEMP-NAMES

: INIT-TEMP-NAMES
  ALSO TEMP-NAMES CONTEXT !
  TEMP-CNT 0!
;
: DEL-NAMES ( A -- )
  DUP>R
  @
  BEGIN
    DUP 0<>
  WHILE
    DUP CDR SWAP 5 - FREE THROW
  REPEAT DROP
  R> 0!
;
: DEL-TEMP-NAMES
  TEMP-NAMES DEL-NAMES
;
HEX
: COMPIL, ( A -- )
  0E8 DOES>A @ C! DOES>A 1+!              \ машинная команда CALL
  DOES>A @ CELL+ - DOES>A @ !
  DOES>A @ 1- DOES>A !
;
DECIMAL
C" LAST-HERE" FIND NIP
[IF] 
  : TEMP-DOES ( N -- ) ( -- ADDR )
    ['] DUP MACRO,
    0x8D C, 0x44 C, 0x24 C, C,  \  LEA     EAX , X [ESP]
    HERE TO LAST-HERE  \  разрешено оптимизировать
    ;
[ELSE]
  : TEMP-DOES ( N -- ) ( -- ADDR )
     POSTPONE RP@ LIT, POSTPONE +  ;
[THEN]

: |TEMP-DOES ( N -- ) ( -- VALUE )
  TEMP-DOES ['] @ COMPILE,
;
: |TEMP-DOES! ( N --  ) ( X -- )
  TEMP-DOES ['] ! COMPILE,
;

VARIABLE  add_depth   add_depth 0!   

\ глубина в стеке возвратов до начала переменных

: !TEMP-CREATE ( addr u -- )
  DUP 20 + ALLOCATE THROW >R
  R@ CELL+ CHAR+ 2DUP C!
  CHAR+ SWAP MOVE ( name )
  TEMP-NAMES @
  R@ CELL+ CHAR+ TEMP-NAMES ! ( latest )
  R@ CELL+ CHAR+ COUNT + DUP>R ! ( link )
  R> CELL+ DUP DOES>A ! R@ ! ( cfa )
  &IMMEDIATE R> CELL+ C! ( flags )
  ['] _CREATE-CODE COMPIL,
  TEMP-CNT @ DOES>A @ 5 + !
  TEMP-CNT 1+!
  POSTPONE >R   DOES> @  2 +  CELLS  add_depth @ +  |TEMP-DOES ;

: TEMP-CREATE ( addr u -- )
 !TEMP-CREATE   DOES> @  2 +  CELLS  add_depth @ +   TEMP-DOES ;

: ->            ' 5 + @  2 +  CELLS  add_depth @ +  |TEMP-DOES!
; IMMEDIATE

: |DROP    R> RP@ + RP! ;

' |DROP VALUE '|DROP

: !!!!;  ( N N1 -- )
  DROP  TEMP-CNT @ CELLS LIT,    POSTPONE >R
  DROP            '|DROP LIT,    POSTPONE >R ;

: !!
  BEGIN  NextWord 2DUP S" !!" COMPARE 0<>
  WHILE          !TEMP-CREATE
  REPEAT  !!!!;  ; IMMEDIATE

: ||
  BEGIN  NextWord 2DUP S" ||" COMPARE 0<>
  WHILE  0 LIT,  !TEMP-CREATE
  REPEAT !!!!;   ; IMMEDIATE

: |
  BEGIN  NextWord 2DUP S" |"  COMPARE 0<>
  WHILE  0 LIT,   TEMP-CREATE
  REPEAT !!!!;   ; IMMEDIATE

: >|
  BEGIN  NextWord 2DUP S" |"  COMPARE 0<>
  WHILE           TEMP-CREATE
  REPEAT !!!!;   ; IMMEDIATE

: ((
  0
  BEGIN
    BL WORD DUP COUNT S" ))" COMPARE 0<>
  WHILE
    FIND IF >R 1+ ELSE 5012 THROW THEN
  REPEAT DROP
  BEGIN
    DUP 0<>
  WHILE
\    R> EXECUTE POSTPONE !     ( исправлено для поддержки || )
    R> 5 + @    2 + CELLS add_depth @ + 
    |TEMP-DOES!
    1-
  REPEAT DROP
; IMMEDIATE


\ ===
\ переопределение соответствующих слов для возможности использовать
\ временные переменные внутри  цикла DO LOOP  и независимо от изменения
\ содержимого стека возвратов  словами   >R   R>


: DO     POSTPONE DO     [  3 CELLS ] LITERAL  add_depth +!
; IMMEDIATE

: LOOP   POSTPONE LOOP   [ -3 CELLS ] LITERAL  add_depth +!
; IMMEDIATE

: +LOOP  POSTPONE +LOOP  [ -3 CELLS ] LITERAL  add_depth +!
; IMMEDIATE

: >R     POSTPONE >R     [  1 CELLS ] LITERAL  add_depth +!
; IMMEDIATE

: R>     POSTPONE R>     [ -1 CELLS ] LITERAL  add_depth +!
; IMMEDIATE

\ ===


: :: : ;

: : ( -- )
  : INIT-TEMP-NAMES
;
:: ; ( -- )
  DEL-TEMP-NAMES PREVIOUS
  POSTPONE ;
  add_depth 0!      \ на всякий случай ;)
; IMMEDIATE

