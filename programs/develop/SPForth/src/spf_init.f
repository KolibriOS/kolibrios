( Инициализация USER-переменных.
  Copyright [C] 1992-1999 A.Cherezov ac@forth.org
  Сентябрь 1999
)

VARIABLE MAINX

VECT <FLOAT-INIT>

: TITLE
  ." SP-FOPTH ANS FORTH 94 for fasm" CR
  ." A.Cherezov  http://www.forth.org.ru/" CR
  ." M.Maksimov  http://forth.spb.su:8888/  http://www.chat.ru/~mak"  CR
  ." PAGE - Clearing of a screen" CR 
  ." WORDS - list of forth-words" CR
  ." BYE - KolibriOS  continuance" CR 
;

: ERR-EXIT ( xt -- )
  CATCH
  ?DUP IF _BYE THEN
;

: HH.
  0x10000000	U/MOD DIGIT> EMIT
  0x1000000	U/MOD DIGIT> EMIT
  0x100000	U/MOD DIGIT> EMIT
  0x10000	U/MOD DIGIT> EMIT
  0x1000	U/MOD DIGIT> EMIT
  0x100	U/MOD DIGIT> EMIT
  0x10	U/MOD DIGIT> EMIT DIGIT> EMIT ;

: INIT

	CURFILE 0!
[ TDIS-OPT ]
	1 ALIGN-BYTES !
	OP0 0! JP0  JpBuffSize  ERASE
	0 LIT, 0x20 TO MM_SIZE SET-OPT

  TITLE
  ['] AUTOEXEC CATCH ?DUP IF 
     ERROR_DO   THEN
  BEGIN
    ['] QUIT CATCH ( жЁЄ« ЇаЁҐ¬  вҐЄбв )
     ERROR_DO
  AGAIN
;
