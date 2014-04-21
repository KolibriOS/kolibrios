DECIMAL

\ ' DUP  VALUE 'DUP_V
\ ' DROP VALUE 'DROP_V

USER     HLD  \ переменная - позиция последней литеры, перенесенной в PAD

0 VALUE  H-STDIN    \ хэндл файла - стандартного ввода
1 VALUE  H-STDOUT   \ хэндл файла - стандартного вывода
1 VALUE  H-STDERR   \ хэндл файла - стандартного вывода ошибок

USER ALIGN-BYTES

: ALIGNED ( addr -- a-addr ) \ 94
\ a-addr - первый выровненный адрес, больший или равный addr.
  ALIGN-BYTES @ DUP 0= IF 1+ DUP ALIGN-BYTES ! THEN
  2DUP
  MOD DUP IF - + ELSE 2DROP THEN
;

: ALIGN ( -- ) \ 94
\ Если указатель пространства данных не выровнен -
\ выровнять его.
  DP @ ALIGNED DP @ - ALLOT
;

: ALIGN-NOP ( n -- )
\ выровнять HERE на n и заполнить NOP
  HERE DUP ROT 2DUP
  MOD DUP IF - + ELSE 2DROP THEN
  OVER - DUP ALLOT 0x90 FILL
;

: IMMEDIATE ( -- ) \ 94
\ Сделать последнее определение словом немедленного исполнения.
\ Исключительная ситуация возникает, если последнее определение
\ не имеет имени.
  LAST @ NAME>F DUP C@ &IMMEDIATE OR SWAP C!
;


: :NONAME ( C: -- colon-sys ) ( S: -- xt ) \ 94 CORE EXT
\ Создать выполнимый токен xt, установить состояние компиляции и 
\ начать текущее определение, произведя colon-sys. Добавить семантику
\ инициализации к текущему определению.
\ Семантика выполнения xt будет задана словами, скомпилированными 
\ в тело определения. Это определение может быть позже выполнено по
\ xt EXECUTE.
\ Если управляющий стек реализован с импользованием стека данных,
\ colon-sys будет верхним элементом на стеке данных.
\ Инициализация: ( i*x -- i*x ) ( R: -- nest-sys )
\ Сохранить зависящую от реализации информацию nest-sys о вызове 
\ определения. Элементы стека i*x представляют аргументы xt.
\ xt Выполнение: ( i*x -- j*x )
\ Выполнить определение, заданное xt. Элементы стека i*x и j*x 
\ представляют аргументы и результаты xt соответственно.
  HERE ]
  HERE TO :-SET ;

: INCLUDED INCLUDED_ ;
  ' NOOP       TO <PRE>
  ' FIND1      TO FIND
  ' ?LITERAL2  TO ?LITERAL
  ' ?SLITERAL2 TO ?SLITERAL
  ' OK1        TO OK.
  ' (ABORT1")  TO (ABORT")

VECT TYPE ' _TYPE TO TYPE
VECT EMIT ' _EMIT TO EMIT


: H. BASE @ SWAP HEX U. BASE ! ;

: TST S" /rd/1/autoload.f" INCLUDED_ ;
: TST1 S" WORDS" EVALUATE ;

