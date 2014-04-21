
: is_path_delimiter ( c -- flag )
  DUP [CHAR] \ = SWAP [CHAR] / = OR
;

: CUT-PATH ( a u -- a u1 )
\ из строки "path\name" выделить строку "path\"
  OVER +
  BEGIN 2DUP <> WHILE DUP C@ is_path_delimiter 0= WHILE 1- REPEAT 1+ THEN
  \ DUP 0!  \ ~ruv (to anfilat): не дќлжно тут затирать поданный буфер!
  OVER -
;

: ModuleName ( -- addr u )
 ABORT
;

: ModuleDirName ( -- addr u )
  ModuleName CUT-PATH
;

: +ModuleDirName ( addr u -- addr2 u2 )
  2>R
  ModuleDirName 2DUP +
  2R> DUP >R ROT SWAP 1+ MOVE 
  R> +
;

: +LibraryDirName ( addr u -- addr2 u2 )
\ ƒобавить addr u к полный_путь_приложени€+devel\
  2>R
  ModuleDirName 2DUP +
  S" devel\" ROT SWAP MOVE
  6 + 2DUP +
  2R> DUP >R ROT SWAP 1+ MOVE 
  R> +
;
: SOURCE-NAME ( -- a u )
  CURFILE @ DUP IF ASCIIZ> ELSE 0 THEN
;
