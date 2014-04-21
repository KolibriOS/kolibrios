( Определяющие слова, создающие словарные статьи в словаре.
  ОС-независимые определения.
  Copyright [C] 1992-1999 A.Cherezov ac@forth.org
  Преобразование из 16-разрядного в 32-разрядный код - 1995-96гг
  Ревизия - сентябрь 1999
)

USER LAST-CFA
USER-VALUE LAST-NON

: REVEAL ( --- )
\ Add the last created definition to the CURRENT wordlist.
  LAST @ CURRENT @ ! ;

: SHEADER ( addr u -- )
  _SHEADER  REVEAL
;

: _SHEADER ( addr u -- )
  0 C,     ( flags )
  HERE 0 , ( cfa )
  DUP LAST-CFA !
  -ROT  WARNING @
  IF 2DUP GET-CURRENT SEARCH-WORDLIST
     IF DROP 2DUP TYPE ."  isn't unique" CR THEN
  THEN
  CURRENT @ SWORD,
  ALIGN
  HERE SWAP ! ( заполнили cfa )
;

: HEADER ( "name" -- )  PARSE-WORD SHEADER ;

: CREATED ( addr u -- )
\ Создать определение для c-addr u с семантикой выполнения, описанной ниже.
\ Если указатель пространства данных не выровнен, зарезервировать место
\ для выравнивания. Новый указатель пространства данных определяет
\ поле данных name. CREATE не резервирует место в поле данных name.
\ name Выполнение: ( -- a-addr )
\ a-addr - адрес поля данных name. Семантика выполнения name может
\ быть расширена с помощью DOES>.
  SHEADER
  HERE DOES>A ! ( для DOES )
  CREATE-CODE COMPILE,
;

: CREATE ( "<spaces>name" -- ) \ 94
   PARSE-WORD CREATED
;

: (DOES1) \ та часть, которая работает одновременно с CREATE (обычно)
  R> DOES>A @ CFL + -
  DOES>A @ 1+ ! ;

Code (DOES2)
   SUB  EBP, 4
   MOV [EBP], EAX
   POP  EBX
   POP  EAX
   PUSH EBX
   RET
EndCODE

: DOES>  \ 94
\ Интерпретация: семантика неопределена.
\ Компиляция: ( C: clon-sys1 -- colon-sys2 )
\ Добавить семантику времени выполнения, данную ниже, к текущему
\ определению. Будет или нет текущее определение сделано видимо
\ для поиска в словаре при компиляции DOES>, зависит от реализации.
\ Поглощает colon-sys1 и производит colon-sys2. Добавляет семантику
\ инициализации, данную ниже, к текущему определению.
\ Время выполнения: ( -- ) ( R: nest-sys1 -- )
\ Заменить семантику выполнения последнего определения name, на семантику
\ выполнения name, данную ниже. Возвратить управление в вызывающее опреде-
\ ление, заданное nest-sys1. Неопределенная ситуация возникает, если name
\ не было определено через CREATE или определенное пользователем слово,
\ вызывающее CREATE.
\ Инициализация: ( i*x -- i*x a-addr ) ( R: -- nest-sys2 )
\ Сохранить зависящую от реализации информацию nest-sys2 о вызывающем
\ определении. Положить адрес поля данных name на стек. Элементы стека
\ i*x представляют аргументы name.
\ name Выполнение: ( i*x -- j*x )
\ Выполнить часть определения, которая начинается с семантики инициализации,
\ добавленной DOES>, которое модифицировало name. Элементы стека i*x и j*x
\ представляют аргументы и результаты слова name, соответственно.
  ['] (DOES1) COMPILE,
  ['] (DOES2) COMPILE,  \   ['] C-R>    MACRO, 
; IMMEDIATE

: VOCABULARY ( "<spaces>name" -- )
\ Создать список слов с именем name. Выполнение name заменит первый список
\ в порядке поиска на список с именем name.
  WORDLIST DUP
  CREATE
  ,
  LATEST OVER CELL+ ! ( ссылка на имя словаря )
  GET-CURRENT SWAP PAR! ( словарь-предок )
\  FORTH-WORDLIST SWAP CLASS! ( класс )
  VOC
  ( DOES> не работает в этом ЦК)
  (DOES1) (DOES2) \ так сделал бы DOES>, определенный выше
  @ CONTEXT !
;

: VARIABLE ( "<spaces>name" -- ) \ 94
\ Пропустить ведущие пробелы. Выделить name, ограниченное пробелом.
\ Создать определение для name с семантикой выполнения, данной ниже.
\ Зарезервировать одну ячейку пространства данных с выровненным адресом.
\ name используется как "переменная".
\ name Выполнение: ( -- a-addr )
\ a-addr - адрес зарезервированной ячейки. За инициализацию ячейки отвечает 
\ программа
  CREATE
  0 ,
;
: CONSTANT ( x "<spaces>name" -- ) \ 94
\ Пропустить ведущие пробелы. Выделить name, ограниченное пробелом.
\ Создать определение для name с семантикой выполнения, данной ниже.
\ name используется как "константа".
\ name Выполнение: ( -- x )
\ Положить x на стек.
  HEADER
  CONSTANT-CODE COMPILE, ,
;
: VALUE ( x "<spaces>name" -- ) \ 94 CORE EXT
\ Пропустить ведущие пробелы. Выделить name, ограниченное пробелом. Создать 
\ определение для name с семантикой выполнения, определенной ниже, с начальным 
\ значением равным x.
\ name используется как "значение".
\ Выполнение: ( -- x )
\ Положить x на стек. Значение x - то, которое было дано, когда имя создавалось,
\ пока не исполнится фраза x TO name, задав новое значение x, 
\ ассоциированное с name.
  HEADER
  CONSTANT-CODE COMPILE, ,
  TOVALUE-CODE COMPILE,
;
: VECT ( -> )
  ( создать слово, семантику выполнения которого можно менять,
    записывая в него новый xt по TO)
  HEADER
  VECT-CODE COMPILE, ['] NOOP ,
  TOVALUE-CODE COMPILE,
;

: ->VARIABLE ( x "<spaces>name" -- ) \ 94
  HEADER
  CREATE-CODE COMPILE,
  ,
;

: USER-ALIGNED ( -- a-addr n )
   USER-HERE 3 + 2 RSHIFT ( 4 / ) 4 * DUP
   USER-HERE -
;

: USER-CREATE ( "<spaces>name" -- )
  HEADER
  HERE DOES>A ! ( для DOES )
  USER-CODE COMPILE,
  USER-ALIGNED
  USER-ALLOT  ,
;
: USER ( "<spaces>name" -- ) \ локальные переменные потока
  USER-CREATE
  4 USER-ALLOT
;

' _TOUSER-VALUE-CODE TO TOUSER-VALUE-CODE

: USER-VALUE ( "<spaces>name" -- ) \ 94 CORE EXT
  HEADER
  USER-VALUE-CODE COMPILE,
  USER-ALIGNED SWAP ,
  CELL+ USER-ALLOT
  TOUSER-VALUE-CODE COMPILE,
;

: ->VECT ( x -> )
  HEADER
  VECT-CODE COMPILE, ,
  TOVALUE-CODE COMPILE,
;

: : _: ;

: _: ( C: "<spaces>name" -- colon-sys ) \ 94
\ Пропустить ведущие разделители. Выделить имя, ограниченное пробелом.
\ Создать определение для имени, называемое "определение через двоеточие".
\ Установить состояние компиляции и начать текущее определение, получив
\ colon-sys. Добавить семантику инициализации, описанную ниже, в текущее
\ определение. Семантика выполнения будет определена словами, скомпилиро-
\ ванными в тело определения. Текущее определение должно быть невидимо
\ при поиске в словаре до тех пор, пока не будет завершено.
\ Инициализация: ( i*x -- i*x ) ( R: -- nest-sys )
\ Сохранить информацию nest-sys о вызове определения. Состояние стека
\ i*x представляет аргументы имени.
\ Имя Выполнение: ( i*x -- j*x )
\ Выполнить определение имени. Состояния стека i*x и j*x представляют
\ аргументы и результаты имени соответственно.
  PARSE-WORD _SHEADER ]
  HERE TO :-SET
;

\ S" ~mak\CompIF.f" INCLUDED

