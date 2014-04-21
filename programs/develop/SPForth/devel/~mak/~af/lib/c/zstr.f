\ $Id: zstring.f,v 1.1 2003/01/18 09:02:11 anfilat Exp $
\ Нуль-строки. Технология взята из ~yz\common.f

\ Копирует строку addr u по адресу z. В конец строки записывает 0
: CZMOVE ( a # z --) 2DUP + >R SWAP CMOVE R> 0 SWAP C! ;

: ALITERAL  R> COUNT OVER + 1+ >R ;

\ VOCABULARY ZStrSupport
\ GET-CURRENT ALSO ZStrSupport DEFINITIONS

USER toadr  USER fromadr  USER counter
: zchar ( --c/0) counter @ 1 <
  IF 0 ELSE -1 counter +! fromadr @ C@ fromadr 1+! THEN ;
: unchar  counter 1+! -1 fromadr +! ;
: c> ( c--) toadr @ C!  toadr 1+! ;
: escape ( c--c )
  DUP [CHAR] n = IF DROP 10 ELSE
    DUP [CHAR] r = IF DROP 13 ELSE
      DUP [CHAR] t = IF DROP 9 ELSE
       DUP [CHAR] b = IF DROP 8 ELSE
        DUP [CHAR] q = IF DROP [CHAR] " ELSE
          DUP [ CHAR 0 1- ] LITERAL OVER < SWAP [ CHAR 9 1+ ] LITERAL < AND IF
            [CHAR] 0 -
            BEGIN ( n) zchar DUP
              [ CHAR 0 1- ] LITERAL OVER < SWAP [ CHAR 9 1+ ] LITERAL < AND
            WHILE
              ( n c) [CHAR] 0 - SWAP 10 * +
            REPEAT
            0<> IF unchar THEN
          THEN
        THEN
       THEN
      THEN
    THEN
  THEN
;
: ESC-CZMOVE ( a # to --)
  toadr ! counter ! fromadr !
  BEGIN
    zchar
    DUP [CHAR] \ = IF DROP zchar escape THEN
  DUP c> 0= UNTIL ;


\ SET-CURRENT

: Z\LITERAL ( addr u -- \ a) \ в режиме интерпретации возвращает адрес
\ буфера в динамической памяти. Буфер желательно освободить
  STATE @ IF
    POSTPONE ALITERAL
    HERE 1+ DUP >R ESC-CZMOVE
    R@ ASCIIZ> NIP 2+ DUP ALLOT 2- R> 1- C!
  ELSE
    DUP 1+ ALLOCATE THROW DUP >R ESC-CZMOVE R>
  THEN
; IMMEDIATE

: ZLITERAL ( addr u -- \ a)
  STATE @ IF
    POSTPONE ALITERAL
    DUP C,
    HERE SWAP DUP ALLOT MOVE 0 C,
  ELSE
    DUP 1+ ALLOCATE THROW DUP >R CZMOVE R>
  THEN
; IMMEDIATE

\ Создает строку, оканчивающуюся нулем
: Z" ( -->") [CHAR] " PARSE [COMPILE] ZLITERAL ; IMMEDIATE

\ Создает 0-строку, при этом преобразует ее по C-правилам.
: Z\" ( -->") [CHAR] " PARSE [COMPILE] Z\LITERAL ; IMMEDIATE

\ PREVIOUS
