( Создание словарых статей и словарей WORDLIST.
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
HEX
1 CONSTANT &IMMEDIATE \ константа для высечения флажка IMMEDIATE
2 CONSTANT &VOC

\ Возвратить wid - идентификатор списка слов, включающего все стандартные 
\ слова, обеспечиваемые реализацией. Этот список слов изначально список 
\ компиляции и часть начального порядка поиска.
: >BODY ( xt -- a-addr ) \ 94
\ a-addr - адрес поля данных, соответствующий xt.
\ Исключительная ситуация возникает, если xt не от слова,
\ определенного через CREATE.
(  1+ @ было в версии 2.5 )
  5 +
;

: SWORD, ( addr u wid -> ) \ добавление заголовка статьи с именем,
         \ заданным строкой addr u, к списку, заданному wid.
         \ Формирует только поля имени и связи с
         \ отведением памяти по ALLOT.
  HERE CELL+
  DUP LAST !
  SWAP DUP @ , !
  S, 0 C,
;

: WORDLIST ( -- wid ) \ 94 SEARCH
\ Создает новый пустой список слов, возвращая его идентификатор wid.
\ Новый список слов может быть возвращен из предварительно распределенных 
\ списков слов или может динамически распределяться в пространстве данных.
\ Система должна допускать создание как минимум 8 новых списков слов в 
\ дополнение к имеющимся в системе.
  HERE VOC-LIST  @ ,  VOC-LIST !
  HERE 0 , \ здесь будет указатель на имя последнего слова списка
       0 , \ здесь будет указатель на имя списка для именованых
       0 , \ wid словаря-предка
       0 , \ класс словаря = wid словаря, определяющего свойства данного
;


: CLASS! ( cls wid -- ) CELL+ CELL+ CELL+ ! ;
: CLASS@ ( wid -- cls ) CELL+ CELL+ CELL+ @ ;
: PAR!   ( Pwid wid -- ) CELL+ CELL+ ! ;
: PAR@   ( wid -- Pwid ) CELL+ CELL+ @ ;


: ID. ( NFA[E] -> )
  ZCOUNT TYPE
;

\ -9 -- flags
\ -8 -- cfa
\ -4 -- LFA
\  0 -- NFA

Code NAME>L ;( NFA -> LFA )
	LEA EAX, [EAX-4]
     RET
EndCode

Code NAME>C ;( NFA -> 'CFA )
	LEA EAX, [EAX-8]
     RET
EndCode

Code NAME> ;( NFA -> CFA )
	MOV EAX, [EAX-8]
     RET
EndCode

Code NAME>F ;( NFA -> FFA )
	LEA EAX, [EAX-9]
     RET
EndCode

Code CDR ;( NFA1 -> NFA2 )
     OR EAX, EAX
     SIF 0<>
		MOV EAX, [EAX-4]
     STHEN
     RET
EndCode

: ?IMMEDIATE ( NFA -> F )
  NAME>F C@ &IMMEDIATE AND
;

: ?VOC ( NFA -> F )
  NAME>F C@ &VOC AND
;
0 [IF]
: IMM ( -- ) \ 94
\ Сделать последнее определение словом немедленного исполнения.
\ Исключительная ситуация возникает, если последнее определение
\ не имеет имени.
  LAST @ NAME>F DUP C@ &IMMEDIATE OR SWAP ." I=" 2DUP H. H.
;
: IMMEDIATE ( -- ) \ 94
\ Сделать последнее определение словом немедленного исполнения.
\ Исключительная ситуация возникает, если последнее определение
\ не имеет имени.
  LAST @ NAME>F DUP C@ &IMMEDIATE OR SWAP C!
;
[THEN]
: VOC ( -- )
\ Пометить последнее определенное слово признаком "словарь".
  LAST @ NAME>F DUP C@ &VOC OR SWAP C!
;

\ ==============================================
\ отладка - поиск слова по адресу в его теле


\ ==============================================
\ отладка - поиск слова по адресу в его теле

: N_UMAX ( nfa nfa1 -- nfa|nfa1 )
 OVER DUP IF NAME> THEN
 OVER DUP IF NAME> THEN U< IF NIP EXIT THEN DROP ;

: WL_NEAR_NFA ( addr wid - addr nfa | addr 0 )
   @
   BEGIN 2DUP DUP IF NAME> THEN U<
   WHILE CDR
   REPEAT
;

0
[IF]

: NEAR_NFA ( addr - nfa addr | 0 addr )
   0 SWAP 
   VOC-LIST
    BEGIN  @ DUP
    WHILE    DUP >R CELL+ WL_NEAR_NFA SWAP >R N_UMAX R>  R>
    REPEAT   DROP
;

[ELSE]

: WL_NEAR_NFA_N ( addr nfa - addr nfa | addr 0 )
   BEGIN 2DUP DUP IF NAME> THEN U<
   WHILE CDR
   REPEAT
;

: WL_NEAR_NFA_M (  addr wid - nfa2 addr | 0 addr )
   0 -ROT
   CELL+ @
   BEGIN  DUP
   WHILE  WL_NEAR_NFA_N  \  nfa addr nfa1
       SWAP >R 
       DUP  >R  N_UMAX 
       R>  DUP  IF CDR THEN
       R>  SWAP
   REPEAT DROP
;

: NEAR_NFA ( addr - nfa addr | 0 addr )
   0 SWAP 
   VOC-LIST
   BEGIN  @ DUP
   WHILE  DUP  >R   WL_NEAR_NFA_M
   >R  N_UMAX  R>  R>
   REPEAT DROP
;

[THEN]

: WordByAddr  ( addr -- c-addr u )
\ найти слово, телу которого принадлежит данный адрес
   DUP         DP @ U> IF DROP S" <not in the image>" EXIT THEN
   NEAR_NFA DROP  DUP 0= IF DROP S" <not found>"        EXIT THEN
   COUNT
;


DECIMAL