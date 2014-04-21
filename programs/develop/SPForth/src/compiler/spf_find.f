( Поиск слов в словарях и управление порядком поиска.
  ОС-независимые определения.
  Copyright [C] 1992-1999 A.Cherezov ac@forth.org
  Преобразование из 16-разрядного в 32-разрядный код - 1995-96гг
  Ревизия - сентябрь 1999
  Модифицированно Максимовым М.О.
  email:mak@mail.rtc.neva.ru
  http://informer.rtc.neva.ru/
  т д {812}105-92-03
  т р {812}552-47-64
)

VECT FIND

0x10 CELLS CONSTANT CONTEXT_SIZE

CREATE SEARCH-BUFF 0x81 ALLOT

Code ZSEARCH-WORDLIST ;( z-addr wid -- 0 | xt 1 | xt -1 ) \ 94 SEARCH
; Найти определение, заданное строкой c-addr u в списке слов, идентифицируемом 
; wid. Если определение не найдено, вернуть ноль.
; Если определение найдено, вернуть выполнимый токен xt и единицу (1), если 
; определение немедленного исполнения, иначе минус единицу (-1).
;	PUSH	WORD PTR [EBP]
	MOV	EDX, [EBP]
	PUSH	EDX
	MOV	EAX, [EAX]
	PUSH	EAX
	LEA	EBP,  [EBP+4]
	CALL	{' GETPR}
	test	eax, eax
	JZ	END
	LEA	EBP, [EBP-4]
	mov	[ebp],eax
	MOVZX	EAX, BYTE PTR [EDX-9]
	DEC	EAX
	OR	EAX,1
	
END:       RET
EndCode

: SEARCH-WORDLIST ( c-addr u wid -- 0 | xt 1 | xt -1 )
  >R 0x7F AND SEARCH-BUFF  ASCII-Z
  R>  ZSEARCH-WORDLIST

;

: SFIND ( addr len --- addr len 0| xt 1|xt -1 )
\ Search all word lists in the search order for the name in the
\ counted string at c-addr. If not found return the name address and 0.
\ If found return the execution token xt and -1 if the word is non-immediate
\ and 1 if the word is immediate.
  CONTEXT
  BEGIN	DUP @
  WHILE	>R
	2DUP  R@ @ SEARCH-WORDLIST ?DUP
	IF    RDROP 2NIP  EXIT \ Exit if found.
	THEN
	R> CELL+
  REPEAT @
;

: FIND1 ( c-addr -- c-addr 0 | xt 1 | xt -1 ) \ 94 SEARCH
\ Расширить семантику CORE FIND следующим:
\ Искать определение с именем, заданным строкой со счетчиком c-addr.
\ Если определение не найдено после просмотра всех списков в порядке поиска,
\ возвратить c-addr и ноль. Если определение найдено, возвратить xt.
\ Если определение немедленного исполнения, вернуть также единицу (1);
\ иначе также вернуть минус единицу (-1). Для данной строки, значения,
\ возвращаемые FIND во время компиляции, могут отличаться от значений,
\ возвращаемых не в режиме компиляции.
  COUNT SFIND
  DUP 0= IF 2DROP 1- 0 THEN ;

: DEFINITIONS ( -- ) \ 94 SEARCH
\ Сделать списком компиляции тот же список слов, что и первый список в порядке 
\ поиска. Имена последующих определений будут помещаться в список компиляции.
\ Последующие изменения порядка поиска не влияют на список компиляции.
  CONTEXT @ SET-CURRENT
;

: GET-ORDER_DROP ( CONTEXT -- widn .. wid1 )
  DUP @ DUP IF >R CELL+ RECURSE R> EXIT THEN 2DROP ;

: GET-ORDER     ( -- widn .. wid1 n )
	DEPTH >R
	CONTEXT GET-ORDER_DROP
	DEPTH R> - ;

: SET-ORDER     ( widn .. wid1 n -- )
                DUP 0<
                IF      DROP ONLY
                ELSE    CONTEXT CONTEXT_SIZE ERASE
                        0
                        ?DO     CONTEXT I CELLS+ !
                        LOOP
                THEN    ;


: FORTH ( -- ) \ 94 SEARCH EXT
\ Преобразовать порядок поиска, состоящий из widn, ...wid2, wid1 (где wid1 
\ просматривается первым) в widn,... wid2, widFORTH-WORDLIST.
  FORTH-WORDLIST CONTEXT !
;

: ONLY ( -- ) \ 94 SEARCH EXT
\ Установить список поиска на зависящий от реализации минимальный список поиска.
\ Минимальный список поиска должен включать слова FORTH-WORDLIST и SET-ORDER.
  CONTEXT CELL+ 0!
  FORTH
;

: ALSO ( -- ) \ 94 SEARCH EXT
\ Преобразовать порядок поиска, состоящий из widn, ...wid2, wid1 (где wid1 
\ просматривается первым) в widn,... wid2, wid1, wid1. Неопределенная ситуация 
\ возникает, если в порядке поиска слишком много списков.
 CONTEXT CONTEXT CELL+ CONTEXT_SIZE CMOVE> ;


: PREVIOUS ( -- ) \ 94 SEARCH EXT
\ Преобразовать порядок поиска, состоящий из widn, ...wid2, wid1 (где wid1 
\ просматривается первым) в widn,... wid2. Неопределенная ситуация возникает,
\ если порядок поиска был пуст перед выполнением PREVIOUS.
  _PREVIOUS ;

: _PREVIOUS ( -- ) \ 94 SEARCH EXT
 CONTEXT CELL+ CONTEXT CONTEXT_SIZE CMOVE  ;

: VOC-NAME. ( wid -- ) \ напечатать имя списка слов, если он именован
  DUP FORTH-WORDLIST = IF DROP ." FORTH"  EXIT THEN
\  DUP KERNEL-WORDLIST = IF DROP ." KERNEL"  EXIT THEN
  DUP CELL+ @ DUP IF ID. DROP ELSE DROP ." <NONAME>:" U. THEN
;

: ORDER ( -- ) \ 94 SEARCH EXT
\ Показать списки в порядке поиска, от первого просматриваемого списка до 
\ последнего. Также показать список слов, куда помещаются новые определения.
\ Формат изображения зависит от реализации.
\ ORDER может быть реализован с использованием слов форматного преобразования
\ чисел. Следовательно он может разрушить перемещаемую область, 
\ идентифицируемую #>.
  GET-ORDER ." Context: "
  0 ?DO ( DUP .) VOC-NAME. SPACE LOOP CR
  ." Current: " GET-CURRENT VOC-NAME. CR
;

: LATEST ( -> NFA )
  CURRENT @ @
;
