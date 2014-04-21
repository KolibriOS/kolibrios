( Трансляция исходных текстов программ.
  ОС-независимые определения.
  Copyright [C] 1992-1999 A.Cherezov ac@forth.org
  Преобразование из 16-разрядного в 32-разрядный код - 1995-96гг
  Ревизия - сентябрь 1999
)

VECT OK.
VECT <MAIN>
VECT ?LITERAL
VECT ?SLITERAL
USER-VALUE SOURCE-ID-XT \ если не равен нулю, то содержит заполняющее

: DEPTH ( -- +n ) \ 94
\ +n - число одинарных ячеек, находящихся на стеке данных перед
\ тем как туда было помещено +n.
  SP@ S0 @ - NEGATE 4 U/
;
: ?STACK ( -> ) \ выдать ошибку "исчерпание стека", если он более чем пуст
  SP@ S0 @ SWAP U< IF S0 @ SP! -4 THROW THEN
;
: ?COMP ( -> )
  STATE @ 0= IF -312 THROW THEN ( Только для режима компиляции )
;

: WORD ( char "<chars>ccc<char>" -- c-addr ) \ 94
\ Пропустить ведущие разделители. Выбрать символы, ограниченные
\ разделителем char.
\ Исключительная ситуация возникает, если длина извлеченной строки
\ больше максимальной длины строки со счетчиком.
\ c-addr - адрес переменной области, содержащей извлеченное слово
\ в виде строки со счетчиком.
\ Если разбираемая область пуста или содержит только разделители,
\ результирующая строка имеет нулевую длину.
\ В конец строки помещается пробел, не включаемый в длину строки.
\ Программа может изменять символы в строке.
  DUP PSKIP PARSE
  DUP HERE C! HERE 1+ SWAP CMOVE
  BL HERE COUNT + !
  HERE
;
1 [IF]
: ' ( "<spaces>name" -- xt ) \ 94
\ Пропустить ведущие пробелы. Выделить name, ограниченное пробелом. Найти name 
\ и вернуть xt, выполнимый токен для name. Неопределенная ситуация возникает, 
\ если name не найдено.
\ Во время интерпретации  ' name EXECUTE  равносильно  name.
  PARSE-WORD SFIND 0=
  IF -321 THROW THEN (  -? )
;

[THEN]

: CHAR ( "<spaces>name" -- char ) \ 94
\ Пропустить ведущие разделители. Выделить имя, органиченное пробелами.
\ Положить код его первого символа на стек.
  PARSE-WORD DROP C@ ;

CREATE ILAST-WORD 0 , 0 ,

: INTERPRET_ ( -> ) \ интерпретировать входной поток
  SAVEERR? ON
  BEGIN
    PARSE-WORD DUP
  WHILE 2DUP  ILAST-WORD 2!
\	." <" TYPE ." >"
    SFIND ?DUP
    IF
         STATE @ =
         IF COMPILE, ELSE EXECUTE THEN
    ELSE
         S" NOTFOUND" SFIND 
         IF EXECUTE
         ELSE 2DROP ?SLITERAL THEN
\          ?SLITERAL
    THEN
    ?STACK
  REPEAT 2DROP
;

VARIABLE &INTERPRET
' INTERPRET_ &INTERPRET !

: INTERPRET &INTERPRET @ EXECUTE ;

\ : HALT ( ERRNUM -> ) \ выход с кодом ошибки
\  >R exit ;

: .SN ( n --)
\ Распечатать n верхних элементов стека
   >R BEGIN
         R@
      WHILE
        SP@ R@ 1- CELLS + @ DUP 0< 
        IF DUP U>D <# #S #> TYPE
           ." (" ABS 0 <# #S [CHAR] - HOLD #> TYPE ." ) " ELSE . THEN
        R> 1- >R
      REPEAT RDROP
;

: OK1
  STATE @ 0=
  IF ."  Ok" DEPTH 70 UMIN 
     0 ?DO [CHAR] . EMIT LOOP CR 
  THEN
;

: EVAL-WORD ( a u -- )
\ интерпретировать ( транслировать) слово с именем  a u
    SFIND ?DUP    IF
    STATE @ =  IF 
    COMPILE,   ELSE 
    EXECUTE    THEN
                  ELSE
    -2003 THROW THEN
;

: [   \ 94 CORE
\ Интерпретация: семантика неопределена.
\ Компиляция: Выполнить семантику выполнения, данную ниже.
\ Выполнение: ( -- )
\ Установить состояние интерпретации. [ слово немедленного выполнения.
  STATE 0!
; IMMEDIATE

: ] ( -- ) \ 94 CORE
\ Установить состояние компиляции.
  TRUE STATE !
;

: QUIT ( -- ) ( R: i*x ) \ CORE 94
\ Сбросить стек возвратов, записать ноль в SOURCE-ID.
\ Установить стандартный входной поток и состояние интерпретации.
\ Не выводить сообщений. Повторять следующее:
\ - Принять строку из входного потока во входной буфер, обнулить >IN
\   и интепретировать.
\ - Вывести зависящее от реализации системное приглашение, если
\   система находится в состоянии интерпретации, все процессы завершены,
\   и нет неоднозначных ситуаций.

\ R0 @ RP! ( не делаем этого, чтобы позволить "['] QUIT CATCH" )
  CONSOLE-HANDLES
  0 TO SOURCE-ID
  [COMPILE] [
  <MAIN>
;

: MAIN1 ( -- )
  BEGIN REFILL
  WHILE INTERPRET OK.
  REPEAT _BYE
;
' MAIN1 TO <MAIN>

: SAVE-SOURCE ( -- i*x i )
  SOURCE-ID-XT  SOURCE-ID   >IN @   SOURCE   CURSTR @   6
;

: RESTORE-SOURCE ( i*x i  -- )
  6 <> IF ABORT THEN
  CURSTR !    SOURCE!  >IN !  TO SOURCE-ID   TO SOURCE-ID-XT
;

: EVALUATE-WITH ( ( i*x c-addr u xt -- j*x )
\ Считая c-addr u входным потоком, вычислить её интерпретатором xt.
  SAVE-SOURCE N>R 
  >R  SOURCE!  -1 TO SOURCE-ID
  R> ( ['] INTERPRET) CATCH
  NR> RESTORE-SOURCE
  THROW
;

: EVALUATE ( i*x c-addr u -- j*x ) \ 94
\ Сохраняет текущие спецификации входного потока.
\ Записывает -1 в SOURCE-ID. Делает строку, заданную c-addr u,
\ входным потоком и входным буфером, устанавливает >IN в 0
\ и интерпретирует. Когда строка разобрана до конца - восстанавливает
\ спецификации предыдущего входного потока.
\ Другие изменения стека определяются выполняемыми по EVALUATE словами.
  ['] INTERPRET EVALUATE-WITH
;

: FQUIT
	BEGIN REFILL
	WHILE INTERPRET
 REPEAT ;

: INCLUDE-FILE ( i*x fileid -- j*x ) \ 94 FILE
	>IN  @ >R
	SOURCE-ID >R  TO SOURCE-ID
	RP@ #TIB @ ALIGNED - RP!
	TIB	RP@ #TIB @ CMOVE
	SOURCE 2>R 
\	TCR ." IF"
	['] FQUIT CATCH	SAVEERR
\	['] NOOP CATCH	SAVEERR	

	2R> SOURCE!
	RP@ TIB  #TIB @ CMOVE
	RP@ #TIB @ ALIGNED + RP!
	R> TO SOURCE-ID
	R> >IN ! THROW      ;

: INCLUDED_  ( c-addr u ---- )
\ Open the file with name c-addr u and interpret all lines contained in it.
	R/O  OPEN-FILE THROW \ ABORT" Can't open include file"
	DUP >R
	['] INCLUDE-FILE CATCH
	R> CLOSE-FILE DROP THROW
;

: REQUIRED ( waddr wu laddr lu -- )
  2SWAP SFIND
  IF DROP 2DROP
  ELSE 2DROP INCLUDED_ THEN
;
: REQUIRE ( "word" "libpath" -- )
  PARSE-NAME PARSE-NAME 2DUP + 0 SWAP C!
  REQUIRED
;

: AUTOEXEC S" /sys/INIT.F" INCLUDED_ ;
