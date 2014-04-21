( Чтение строки исходного текста из входного потока: консоли или файла.
  Copyright [C] 1992-1999 A.Cherezov ac@forth.org
  Ревизия: Сентябрь 1999
)

VECT <PRE>

: CONSOLE-HANDLES \ $$$$
;

: QUERY ( --- )
\ Read a line from the terminal into the terminal input buffer.
  TIB 80 ACCEPT #TIB ! 0 >IN ! ;

: FREFILL ( h -- flag )
  TIB C/L ROT READ-LINE THROW \ TAKEN-TIB
  SWAP  #TIB !  0 >IN ! CURSTR 1+!
  0 SOURCE + C!
;


: REFILL ( --- f)
\ Refill the current input source when it is exhausted. f is
\ true if it was successfully refilled.

  SOURCE-ID -1 = IF
   0 \ Not refillable for EVALUATE
  ELSE   SOURCE-ID
   IF    SOURCE-ID FREFILL
   ELSE  QUERY -1 \ Always successful from terminal.
   THEN
  THEN
;
