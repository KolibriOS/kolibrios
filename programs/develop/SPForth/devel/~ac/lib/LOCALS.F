lib\ext\locals.f \EOF

( 28.Mar.2000 Andrey Cherezov  Copyright [C] RU FIG
  Использованы идеи следующих авторов:
  Ruvim Pinka; Dmitry Yakimov; Oleg Shalyopa; Yuriy Zhilovets;
  Konstantin Tarasov

  !! Работает, только начиная с 30 билда SPF/3.75: VERSION . 375030  Ok
)

( Простое расширение СП-Форта локальными переменными.
  Реализовано без использования LOCALS стандарта 94.

  Объявление временных переменных, видимых только внутри
  текущего слова и ограниченных временем вызова данного
  слова выполняется с помощью слова "{". Внутри определения 
  слова используется конструкция, подобная стековой нотации Форта
  { список_инициализированных_локалов \ сп.неиниц.локалов -- что угодно }
  Например:

  { a b c d \ e f -- i j }

  Часть "\ сп.неиниц.локалов" может отсутствовать, например:

  { item1 item2 -- }

  Это заставляет СП-Форт автоматически выделять место в
  стеке возвратов для этих переменных в момент вызова слова
  и автоматически освобождать место при выходе из него.

  Обращение к таким локальным переменным - как к VALUE-переменным
  по имени. Если нужен адрес переменной, то используется "^ имя".

  Примеры:

  : TEST { a b c d \ e f -- } a . b . c .  b c + -> e  e .  f .  ^ a @ . ;
   Ok
  1 2 3 4 TEST
  1 2 3 5 0 1  Ok

  : TEST { a b -- } a . b . CR 5 0 DO I . a . b . CR LOOP ;
   Ok
  12 34 TEST
  12 34
  0 12 34
  1 12 34
  2 12 34
  3 12 34
  4 12 34
   Ok

  : TEST { a b } a . b . ;
   Ok
  1 2 TEST
  1 2  Ok

  : TEST { a b \ c } a . b . c . ;
   Ok
  1 2 TEST
  1 2 0  Ok

  : TEST { a b -- } a . b . ;
   Ok
  1 2 TEST
  1 2  Ok

  : TEST { a b \ c -- d } a . b . c . ;
   Ok
  1 2 TEST
  1 2 0  Ok

  : TEST { \ a b } a . b .  1 -> a  2 -> b  a . b . ;
   Ok
  TEST
  0 0 1 2  Ok

  Имена локальных переменных существуют в динамическом
  временном словаре только в момент компиляции слова, а
  после этого вычищаются и более недоступны.

  Использовать конструкцию "{ ... }" внутри одного определения можно
  только один раз.

  Компиляция этой библиотеки добавляет в текущий словарь компиляции
  Только два слова:
  словарь "vocLocalsSupport" и "{"
  Все остальные детали "спрятаны" в словаре, использовать их
  не рекомендуется.
)


VOCABULARY vocLocalsSupport

GET-CURRENT ALSO vocLocalsSupport DEFINITIONS

USER widLocals
USER uLocalsCnt
USER uLocalsUCnt
USER uPrevCurrent
USER uAddDepth

: (Local^) ( N -- ADDR )
  RP@ +
;
: LocalOffs ( n -- offs )
  uLocalsCnt @ SWAP - CELLS CELL+ uAddDepth @ +
;
: CompileLocalsInit
  uPrevCurrent @ SET-CURRENT
  uLocalsCnt  @ uLocalsUCnt @ - ?DUP IF CELLS LIT, POSTPONE DRMOVE THEN
  uLocalsUCnt @ ?DUP IF LIT, POSTPONE (RALLOT) THEN
  uLocalsCnt  @ ?DUP 
  IF CELLS LIT, POSTPONE >R  ['] (LocalsExit) LIT, POSTPONE >R THEN
;
: CompileLocal@ ( n -- )
  LocalOffs LIT, POSTPONE RP+@
;
: LocalsStartup
  TEMP-WORDLIST widLocals !
  GET-CURRENT uPrevCurrent !
  ALSO vocLocalsSupport
  ALSO widLocals @ CONTEXT ! DEFINITIONS
  uLocalsCnt 0!
  uLocalsUCnt 0!
  uAddDepth 0!
;
: LocalsCleanup
  PREVIOUS PREVIOUS
  widLocals @ FREE-WORDLIST
;
: LocalsDoes@
  uLocalsCnt @ ,
  uLocalsCnt 1+!
  DOES> @ CompileLocal@
;
: ;; POSTPONE ; ; IMMEDIATE

: ^ ' >BODY @ LocalOffs LIT, POSTPONE RP+ ; IMMEDIATE

: -> ' >BODY @ LocalOffs LIT, POSTPONE RP+! ; IMMEDIATE

: в POSTPONE -> ; IMMEDIATE

WARNING @ WARNING 0!
\ ===
\ переопределение соответствующих слов для возможности использовать
\ временные переменные внутри  цикла DO LOOP  и независимо от изменения
\ содержимого стека возвратов  словами   >R   R>

: DO    POSTPONE DO     [  3 CELLS ] LITERAL  uAddDepth +! ; IMMEDIATE
: ?DO   POSTPONE ?DO    [  3 CELLS ] LITERAL  uAddDepth +! ; IMMEDIATE
: LOOP  POSTPONE LOOP   [ -3 CELLS ] LITERAL  uAddDepth +! ; IMMEDIATE
: +LOOP POSTPONE +LOOP  [ -3 CELLS ] LITERAL  uAddDepth +! ; IMMEDIATE
: >R    POSTPONE >R     [  1 CELLS ] LITERAL  uAddDepth +! ; IMMEDIATE
: R>    POSTPONE R>     [ -1 CELLS ] LITERAL  uAddDepth +! ; IMMEDIATE
: RDROP POSTPONE RDROP  [ -1 CELLS ] LITERAL  uAddDepth +! ; IMMEDIATE

\ ===

: ;  LocalsCleanup POSTPONE ; ; IMMEDIATE

WARNING !

\ =====================================================================
SET-CURRENT

: {
  LocalsStartup
  BEGIN
    BL SKIP PeekChar DUP [CHAR] \ <> 
                    OVER [CHAR] - <> AND
                    SWAP [CHAR] } <> AND
  WHILE
    CREATE LocalsDoes@ IMMEDIATE
  REPEAT

  PeekChar >IN 1+! DUP [CHAR] } <>
  IF
    [CHAR] \ =
    IF
      BEGIN
        BL SKIP PeekChar DUP [CHAR] - <> SWAP [CHAR] } <> AND
      WHILE
        CREATE LocalsDoes@ IMMEDIATE
        uLocalsUCnt 1+!
      REPEAT
    THEN
    [CHAR] } PARSE 2DROP
  ELSE DROP THEN
  CompileLocalsInit
;; IMMEDIATE

PREVIOUS
