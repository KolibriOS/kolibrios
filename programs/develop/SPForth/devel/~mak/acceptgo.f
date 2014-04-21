REQUIRE ACCEPTHistory ~micro/lib/key/accept.f

: REFILL ( -- flag ) \ 94 FILE EXT
  SOURCE-ID IF
    REFILL
  ELSE
    H-STDIN [ H-STDIN ] LITERAL <> IF
      REFILL
    ELSE
      TIB 79 ACCEPT #TIB ! >IN 0! <PRE> -1 EXIT
    THEN
  THEN
;

: MAIN2 ( -- )
  BEGIN
    REFILL
  WHILE
    INTERPRET OK
  REPEAT BYE
;

' MAIN2 TO <MAIN>

