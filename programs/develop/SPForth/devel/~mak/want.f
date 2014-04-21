REQUIRE [IF] ~mak/CompIF.f
REQUIRE $! ~mak\place.f
REQUIRE [IFNDEF] ~nn\lib\ifdef.f

[IFNDEF] PARSE-WORD
: PARSE-WORD NextWord ;
[THEN]

CREATE WANT_BUFF 0x101 ALLOT
CREATE WANT_FILE 0x101 ALLOT
  S" ~mak\do_want.f"  WANT_FILE $!

: [WANT] ( addr len -- addr len | )
   2DUP PARSE-WORD COMPARE
   IF POSTPONE \ EXIT THEN
   2DROP INTERPRET \EOF ;

: WANT  ( -- )
   PARSE-WORD WANT_BUFF $!
   WANT_FILE COUNT INCLUDED ;
  
   